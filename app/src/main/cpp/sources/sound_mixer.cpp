#include <logger.h>
#include <sound_mixer.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>
#include <cinttypes>
#include <cstdio>
#include <unordered_map>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

namespace {

constexpr float CHANNEL_VOLUME = 1.0F;
constexpr static auto DECOMPRESSOR_TIMEOUT = std::chrono::milliseconds ( 1U );

//----------------------------------------------------------------------------------------------------------------------

[[nodiscard]] char const* GetFormatString ( aaudio_format_t fmt, std::span<char> unknownFormat ) noexcept
{
    static std::unordered_map<aaudio_format_t, char const*> const map
    {
        { AAUDIO_FORMAT_INVALID, "AAUDIO_FORMAT_INVALID" },
        { AAUDIO_FORMAT_UNSPECIFIED, "AAUDIO_FORMAT_UNSPECIFIED" },
        { AAUDIO_FORMAT_PCM_I16, "AAUDIO_FORMAT_PCM_I16" },
        { AAUDIO_FORMAT_PCM_FLOAT, "AAUDIO_FORMAT_PCM_FLOAT" },
        { AAUDIO_FORMAT_PCM_I24_PACKED, "AAUDIO_FORMAT_PCM_I24_PACKED" },
        { AAUDIO_FORMAT_PCM_I32, "AAUDIO_FORMAT_PCM_I32" }
    };

    if ( auto const findResult = map.find ( fmt ); findResult != map.cend () )
        return findResult->second;

    std::snprintf ( unknownFormat.data (), unknownFormat.size (), "UNKNOWN [%d]", static_cast<int> ( fmt ) );
    return unknownFormat.data ();
};

} // end of anonymous namespace

class StreamCloser final
{
    private:
        AAudioStream&       _stream;

    public:
        StreamCloser () = delete;

        StreamCloser ( StreamCloser const & ) = delete;
        StreamCloser& operator = ( StreamCloser const & ) = delete;

        StreamCloser ( StreamCloser && ) = delete;
        StreamCloser& operator = ( StreamCloser && ) = delete;

        explicit StreamCloser ( AAudioStream &stream ) noexcept;

        ~StreamCloser () noexcept;
};

StreamCloser::StreamCloser ( AAudioStream &stream ) noexcept:
    _stream ( stream )
{
    // NOTHING
}

StreamCloser::~StreamCloser () noexcept
{
    SoundMixer::CheckAAudioResult ( AAudioStream_close ( &_stream ),
        "StreamCloser::~StreamCloser",
        "Can't close stream"
    );
}

//----------------------------------------------------------------------------------------------------------------------

bool SoundMixer::Init () noexcept
{
    _listenerOrientation.Identity ();

    std::fill_n ( _channelVolume, std::size ( _channelVolume ), CHANNEL_VOLUME );
    SetMasterVolume ( _masterVolume );

    if ( !CheckAAudioResult ( AAudio_createStreamBuilder ( &_builder ), "SoundMixer::Init", "Can't create builder" ) )
    {
        Destroy ();
        return false;
    }

    AAudioStream* probe = nullptr;
    aaudio_result_t result = AAudioStreamBuilder_openStream ( _builder, &probe );

    if ( !CheckAAudioResult ( result, "SoundMixer::Init", "Can't open system probe stream" ) )
        return false;

    {
        // Operator block is needed because StreamCloser instance.
        StreamCloser const streamCloser ( *probe );
        PrintStreamInfo ( *probe, "System parameters" );
    }

    AAudioStreamBuilder_setChannelCount ( _builder, GetChannelCount () );
    AAudioStreamBuilder_setContentType ( _builder, AAUDIO_CONTENT_TYPE_SONIFICATION );
    AAudioStreamBuilder_setErrorCallback ( _builder, &SoundMixer::ErrorCallback, this );
    AAudioStreamBuilder_setDirection ( _builder, AAUDIO_DIRECTION_OUTPUT );
    AAudioStreamBuilder_setFormat ( _builder, GetFormat () );
    AAudioStreamBuilder_setPerformanceMode ( _builder, AAUDIO_PERFORMANCE_MODE_NONE );
    AAudioStreamBuilder_setSampleRate ( _builder, GetSampleRate () );
    AAudioStreamBuilder_setSharingMode ( _builder, AAUDIO_SHARING_MODE_SHARED );
    AAudioStreamBuilder_setUsage ( _builder, AAUDIO_USAGE_GAME );

    if ( !ResolveBufferSize () )
    {
        Destroy ();
        return false;
    }

    result = AAudioStreamBuilder_openStream ( _builder, &probe );

    if ( !CheckAAudioResult ( result, "SoundMixer::ResolveBufferSize", "Can't open engine probe stream" ) )
    {
        Destroy ();
        return false;
    }

    StreamCloser const streamCloser ( *probe );
    result = AAudioStream_setBufferSizeInFrames ( probe, _bufferFrameCount );

    if ( result < 0 )
    {
        // Error happens.
        CheckAAudioResult ( result, "SoundMixer::Init", "Can't change probe buffer size" );
        Destroy ();
        return false;
    }

    if ( result != _bufferFrameCount )
    {
        LogError ( "SoundMixer::Init - Can't change stream buffer size to %" PRIi32 ". "
            "Actual returned size is %d.",
            _bufferFrameCount,
            static_cast<int> ( result )
        );

        Destroy ();
        return false;
    }

    PrintStreamInfo ( *probe, "Engine parameters" );
    _decompressorFlag = true;

    _decompressorThread = std::thread (
        [ this ] () noexcept {
            while ( _decompressorFlag )
            {
                std::unique_lock<std::mutex> const lock ( _mutex );

                for ( auto& record: _decompressors )
                    record.second->OnDecompress ();

                // To not consume too much CPU time.
                std::this_thread::sleep_for ( DECOMPRESSOR_TIMEOUT );
            }
        }
    );

    return true;
}

void SoundMixer::Destroy () noexcept
{
    if ( _decompressorThread.joinable () )
    {
        _decompressorFlag = false;
        _decompressorThread.join ();
    }

#ifdef ANDROID_VULKAN_DEBUG

    if ( !_emitters.empty () )
    {
        constexpr char const format[] =
R"__(SoundMixer::Destroy - Memory leak detected: %zu
>>>)__";

        LogError ( format, _emitters.size () );

        for ( auto const& record : _emitters )
            LogError ( "    %s", record.second->GetFile ().c_str () );

        LogError ( "<<<" );

#ifdef ANDROID_VULKAN_STRICT_MODE

        assert ( false );

#endif // ANDROID_VULKAN_STRICT_MODE

    }

#endif // ANDROID_VULKAN_DEBUG

    if ( !_builder )
        return;

    CheckAAudioResult ( AAudioStreamBuilder_delete ( _builder ), "SoundMixer::Destroy", "Can't destroy builder" );
    _builder = nullptr;
}

std::optional<AAudioStream*> SoundMixer::CreateStream ( SoundEmitter::Context &context ) noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );
    auto result = CreateStreamInternal ( context );

    if ( result != std::nullopt )
        _emitters.emplace ( *result, context._soundEmitter );

    return result;
}

bool SoundMixer::DestroyStream ( AAudioStream &stream ) noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );

    bool const result = SoundMixer::CheckAAudioResult ( AAudioStream_close ( &stream ),
        "SoundMixer::DestroyStream",
        "Can't close stream"
    );

    if ( !result )
        return false;

    if ( auto const findResult = _emitters.find ( &stream ); findResult != _emitters.cend () )
    {
        _decompressors.erase ( &stream );
        _emitters.erase ( findResult );
        return true;
    }

    LogError ( "SoundMixer::DestroyStream - Can't find stream!" );
    assert ( false );
    return false;
}

[[maybe_unused]] size_t SoundMixer::GetBufferSampleCount () const noexcept
{
    return _bufferSampleCount;
}

[[maybe_unused]] float SoundMixer::GetChannelVolume ( eSoundChannel channel ) const noexcept
{
    return _channelVolume[ static_cast<size_t> ( channel ) ];
}

[[maybe_unused]] void SoundMixer::SetChannelVolume ( eSoundChannel channel, float volume ) noexcept
{
    _channelVolume[ static_cast<size_t> ( channel ) ] = std::clamp ( volume, 0.0F, 1.0F );
}

[[maybe_unused]] float SoundMixer::GetMasterVolume () const noexcept
{
    return _masterVolume;
}

void SoundMixer::SetMasterVolume ( float volume ) noexcept
{
    _masterVolume = std::clamp ( volume, 0.0F, 1.0F );

    for ( size_t i = 0U; i < TOTAL_SOUND_CHANNELS; ++i )
    {
        _effectiveChannelVolume[ i ] = volume * _channelVolume[ i ];
    }
}

SoundListenerInfo const& SoundMixer::GetListenerInfo () noexcept
{
    if ( _listenerTransformChanged )
    {
        constexpr GXVec3 le ( -1.0F, 0.0F, 0.0F );

        _listenerOrientation.TransformFast ( _listenerInfo._leftDirection, le );
        _listenerInfo._rightDirection = _listenerInfo._leftDirection;
        _listenerInfo._rightDirection.Reverse ();

        _listenerTransformChanged = false;
    }

    return _listenerInfo;
}

[[maybe_unused]] void SoundMixer::SetListenerLocation ( GXVec3 const &location ) noexcept
{
    _listenerInfo._location = location;
    _listenerTransformChanged = true;
}

SoundStorage& SoundMixer::GetSoundStorage () noexcept
{
    return _soundStorage;
}

[[maybe_unused]] void SoundMixer::SetListenerOrientation ( GXQuat const &orientation ) noexcept
{
    _listenerOrientation = orientation;
    _listenerTransformChanged = true;
}

void SoundMixer::Pause () noexcept
{
    for ( auto& record : _emitters )
    {
        SoundEmitter* emitter = record.second;

        if ( emitter->IsPlaying () )
        {
            if ( emitter->Pause () )
            {
                _emittersToResume.push_back ( emitter );
            }
        }
    }
}

void SoundMixer::Resume () noexcept
{
    for ( auto* emitter : _emittersToResume )
    {
        if ( !emitter->Play () )
        {
            LogWarning ( "SoundMixer::OnResume - Can't play," );
        }
    }

    _emittersToResume.clear ();
}

void SoundMixer::RegisterDecompressor ( AAudioStream &stream, PCMStreamer &streamer ) noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );

    if ( _decompressors.contains ( &stream ) )
    {
        LogError ( "SoundMixer::RegisterDecompressor - Stream already registered. Please check business logic." );
        assert ( false );
        return;
    }

    _decompressors.emplace ( &stream, &streamer );
}

void SoundMixer::UnregisterDecompressor ( AAudioStream &stream ) noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );

    if ( _decompressors.erase ( &stream ) > 0U )
        return;

    LogError ( "SoundMixer::UnregisterDecompressor - Can't find stream. Please check business logic." );
    assert ( false );
}

bool SoundMixer::CheckAAudioResult ( aaudio_result_t result, char const* from, char const* message ) noexcept
{
    if ( result == AAUDIO_OK )
        return true;

    // Positive value. This case should be process differently by AAudio design.
    assert ( result < 0 );

    LogError ( "%s - %s. Error: %s.", from, message, AAudio_convertResultToText ( result ) );
    return false;
}

void SoundMixer::RecreateSoundEmitter ( AAudioStream &stream ) noexcept
{
    std::thread thread (
        [ this, &stream ] () noexcept {
            std::unique_lock<std::mutex> const lock ( _mutex );
            auto const emitterFind = _emitters.find ( &stream );

            if ( emitterFind == _emitters.cend () )
            {
                LogError ( "SoundMixer::RecreateSoundEmitter - Can't find sound emitter." );
                assert ( false );
                return;
            }

            SoundEmitter& emitter = *emitterFind->second;
            StreamCloser const closer ( *emitterFind->first );
            auto result = CreateStreamInternal ( emitter.GetContext () );

            if ( result == std::nullopt )
            {
                LogError ( "SoundMixer::RecreateSoundEmitter - Can't create new AAudioStream." );
                assert ( false );
                return;
            }

            AAudioStream* newStream = *result;

            if ( auto const streamerFind = _decompressors.find ( &stream ); streamerFind != _decompressors.cend () )
            {
                PCMStreamer* streamer = streamerFind->second;
                _decompressors.erase ( streamerFind );
                _decompressors.emplace ( newStream, streamer );
            }

            emitter.OnStreamRecreated ( *newStream );
            _emitters.erase ( emitterFind );
            _emitters.emplace ( newStream, &emitter );
        }
    );

    thread.detach ();
}

std::optional<AAudioStream*> SoundMixer::CreateStreamInternal ( SoundEmitter::Context &context ) noexcept
{
    AAudioStreamBuilder_setDataCallback ( _builder, &SoundMixer::PCMCallback, &context );

    AAudioStream* stream = nullptr;
    aaudio_result_t result = AAudioStreamBuilder_openStream ( _builder, &stream );

    if ( !CheckAAudioResult ( result, "SoundMixer::CreateStreamInternal", "Can't open stream" ) )
        return std::nullopt;

    if ( int32_t const sampleRate = AAudioStream_getSampleRate ( stream ); sampleRate != GetSampleRate () )
    {
        StreamCloser const streamCloser ( *stream );

        LogError ( "SoundMixer::CreateStreamInternal - Unexpected sample rate %" PRIi32 ". Should be %" PRIi32 ".",
            sampleRate,
            GetSampleRate ()
        );

        assert ( false );
        return std::nullopt;
    }

    if ( aaudio_format_t const format = AAudioStream_getFormat ( stream ); format != GetFormat () )
    {
        StreamCloser const streamCloser ( *stream );

        constexpr size_t size = 128U;
        char current[ size ];
        char expected[ size ];

        LogError ( "SoundMixer::CreateStreamInternal - Unexpected format %s. Should be %s.",
            GetFormatString ( format, current ),
            GetFormatString ( GetFormat (), expected )
        );

        assert ( false );
        return std::nullopt;
    }

    if ( int32_t const channels = AAudioStream_getChannelCount ( stream ); channels != GetChannelCount () )
    {
        StreamCloser const streamCloser ( *stream );

        LogError ( "SoundMixer::CreateStreamInternal - Unexpected channel count %" PRIi32 ". Should be %" PRIi32 ".",
            channels,
            GetChannelCount ()
        );

        assert ( false );
        return std::nullopt;
    }

    int32_t const frames = AAudioStream_setBufferSizeInFrames ( stream, _bufferFrameCount );

    if ( frames != _bufferFrameCount )
    {
        StreamCloser const streamCloser ( *stream );

        LogError ( "SoundMixer::CreateStreamInternal - Unexpected buffer frame count %" PRIi32 ". "
            "Should be %" PRIi32 ".",
            frames,
            _bufferFrameCount
        );

        assert ( false );
        return std::nullopt;
    }

    return stream;
}

bool SoundMixer::ResolveBufferSize () noexcept
{
    AAudioStream* probe = nullptr;
    aaudio_result_t result = AAudioStreamBuilder_openStream ( _builder, &probe );

    if ( !CheckAAudioResult ( result, "SoundMixer::ResolveBufferSize", "Can't open probe stream (branch 1)" ) )
        return false;

    // IIFE pattern.
    int32_t const burst = [] ( AAudioStream &p ) noexcept -> int32_t {
        StreamCloser const streamCloser ( p );
        return AAudioStream_getFramesPerBurst ( &p );
    } ( *probe );

    AAudioStreamBuilder_setBufferCapacityInFrames ( _builder, burst );
    AAudioStreamBuilder_setFramesPerDataCallback ( _builder, burst );
    result = AAudioStreamBuilder_openStream ( _builder, &probe );

    if ( !CheckAAudioResult ( result, "SoundMixer::ResolveBufferSize", "Can't open probe stream (branch 2)" ) )
        return false;

    result = AAudioStream_setBufferSizeInFrames ( probe, burst );

    if ( result < 0 )
    {
        // Error happens.
        char msg[ 128U ];
        std::snprintf ( msg, std::size ( msg ), "Can't change stream buffer size to %" PRIi32, burst );
        CheckAAudioResult ( result, "SoundMixer::ResolveBufferSize", msg );
        return false;
    }

    if ( result != burst )
    {
        LogError ( "SoundMixer::ResolveBufferSize - Can't change stream buffer size to burst size %" PRIi32 ". "
            "Actual returned size is %d.",
            burst,
            static_cast<int> ( result )
        );

        return false;
    }

    _bufferFrameCount = burst;
    _bufferSampleCount = static_cast<size_t> ( burst * GetChannelCount () );
    return true;
}

void SoundMixer::ErrorCallback ( AAudioStream* stream, void* userData, aaudio_result_t err )
{
    CheckAAudioResult ( err, "SoundMixer::ErrorCallback", "Got error from AAudio subsystem" );

    if ( err != AAUDIO_ERROR_DISCONNECTED )
        return;

    LogInfo ( "SoundMixer::ErrorCallback - Main audio endpoint has been disconnected, Trying to restore sound..." );
    auto& soundMixer = *static_cast<SoundMixer*> ( userData );
    soundMixer.RecreateSoundEmitter ( *stream );
}

aaudio_data_callback_result_t SoundMixer::PCMCallback ( AAudioStream* /*stream*/,
    void* userData,
    void* audioData,
    int32_t numFrames
)
{
    auto& context = *static_cast<SoundEmitter::Context*> ( userData );
    assert ( context._soundMixer->_bufferFrameCount == numFrames );

    context._soundEmitter->FillPCM (
        std::span ( static_cast<PCMStreamer::PCMType*> ( audioData ),
            static_cast<size_t> ( numFrames * GetChannelCount () )
        ),

        context._soundMixer->_effectiveChannelVolume[ static_cast<size_t> ( context._soundChannel ) ]
    );

    return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

void SoundMixer::PrintStreamInfo ( AAudioStream &stream, char const* header ) noexcept
{
    constexpr char const format[] =
R"__(SoundMixer::PrintStreamInfo - %s:
>>>
    Stream: %p
    Buffer size in frames: %d
    Frames per burst: %d
    Buffer capacity in frames: %d
    Frames per data callback: %d
    Sample rate: %d
    Channel count: %d
    Device ID: %d
    Format: %s
    Sharing mode: %s
    Performance mode: %s
    Direction: %s
    Session ID: %s
    Usage: %s
    Content type: %s
<<<)__";

    constexpr size_t size = 128U;
    char unknownSharingMode[ size ];

    auto const getSharingMode = [ &unknownSharingMode ] ( aaudio_sharing_mode_t mode ) noexcept -> char const* {
        static std::unordered_map<aaudio_sharing_mode_t, char const*> const map
        {
            { AAUDIO_SHARING_MODE_EXCLUSIVE, "AAUDIO_SHARING_MODE_EXCLUSIVE" },
            { AAUDIO_SHARING_MODE_SHARED, "AAUDIO_SHARING_MODE_SHARED" }
        };

        if ( auto const findResult = map.find ( mode ); findResult != map.cend () )
            return findResult->second;

        std::snprintf ( unknownSharingMode, size, "UNKNOWN [%d]", static_cast<int> ( mode ) );
        return unknownSharingMode;
    };

    char unknownPerf[ size ];

    auto const getPerformanceMode = [ &unknownPerf ] ( aaudio_performance_mode_t performance ) noexcept -> char const* {
        static std::unordered_map<aaudio_performance_mode_t, char const*> const map
        {
            { AAUDIO_PERFORMANCE_MODE_NONE, "AAUDIO_PERFORMANCE_MODE_NONE" },
            { AAUDIO_PERFORMANCE_MODE_POWER_SAVING, "AAUDIO_PERFORMANCE_MODE_POWER_SAVING" },
            { AAUDIO_PERFORMANCE_MODE_LOW_LATENCY, "AAUDIO_PERFORMANCE_MODE_LOW_LATENCY" }
        };

        if ( auto const findResult = map.find ( performance ); findResult != map.cend () )
            return findResult->second;

        std::snprintf ( unknownPerf, size, "UNKNOWN [%d]", static_cast<int> ( performance ) );
        return unknownPerf;
    };

    char unknownDirection[ size ];

    auto const getDirection = [ &unknownDirection ] ( aaudio_direction_t direction ) noexcept -> char const* {
        static std::unordered_map<aaudio_direction_t, char const*> const map
        {
            { AAUDIO_DIRECTION_OUTPUT, "AAUDIO_DIRECTION_OUTPUT" },
            { AAUDIO_DIRECTION_INPUT, "AAUDIO_DIRECTION_INPUT" }
        };

        if ( auto const findResult = map.find ( direction ); findResult != map.cend () )
            return findResult->second;

        std::snprintf ( unknownDirection, size, "UNKNOWN [%d]", static_cast<int> ( direction ) );
        return unknownDirection;
    };

    char unknownSessionID[ size ];

    auto const getSessionID = [ &unknownSessionID ] ( aaudio_session_id_t sessionID ) noexcept -> char const* {
        static std::unordered_map<aaudio_session_id_t, char const*> const map
        {
            { AAUDIO_SESSION_ID_NONE, "AAUDIO_SESSION_ID_NONE" },
            { AAUDIO_SESSION_ID_ALLOCATE, "AAUDIO_SESSION_ID_ALLOCATE" }
        };

        if ( auto const findResult = map.find ( sessionID ); findResult != map.cend () )
            return findResult->second;

        std::snprintf ( unknownSessionID, size, "UNKNOWN [%d]", static_cast<int> ( sessionID ) );
        return unknownSessionID;
    };

    char unknownUsage[ size ];

    auto const getUsage = [ &unknownUsage ] ( aaudio_usage_t usage ) noexcept -> char const* {
        static std::unordered_map<aaudio_usage_t, char const*> const map
        {
            { AAUDIO_USAGE_MEDIA, "AAUDIO_USAGE_MEDIA" },
            { AAUDIO_USAGE_VOICE_COMMUNICATION, "AAUDIO_USAGE_VOICE_COMMUNICATION" },
            { AAUDIO_USAGE_VOICE_COMMUNICATION_SIGNALLING, "AAUDIO_USAGE_VOICE_COMMUNICATION_SIGNALLING" },
            { AAUDIO_USAGE_ALARM, "AAUDIO_USAGE_ALARM" },
            { AAUDIO_USAGE_NOTIFICATION, "AAUDIO_USAGE_NOTIFICATION" },
            { AAUDIO_USAGE_NOTIFICATION_RINGTONE, "AAUDIO_USAGE_NOTIFICATION_RINGTONE" },
            { AAUDIO_USAGE_NOTIFICATION_EVENT, "AAUDIO_USAGE_NOTIFICATION_EVENT" },
            { AAUDIO_USAGE_ASSISTANCE_ACCESSIBILITY, "AAUDIO_USAGE_ASSISTANCE_ACCESSIBILITY" },
            { AAUDIO_USAGE_ASSISTANCE_NAVIGATION_GUIDANCE, "AAUDIO_USAGE_ASSISTANCE_NAVIGATION_GUIDANCE" },
            { AAUDIO_USAGE_ASSISTANCE_SONIFICATION, "AAUDIO_USAGE_ASSISTANCE_SONIFICATION" },
            { AAUDIO_USAGE_GAME, "AAUDIO_USAGE_GAME" },
            { AAUDIO_USAGE_ASSISTANT, "AAUDIO_USAGE_ASSISTANT" },
            { AAUDIO_SYSTEM_USAGE_EMERGENCY, "AAUDIO_SYSTEM_USAGE_EMERGENCY" },
            { AAUDIO_SYSTEM_USAGE_SAFETY, "AAUDIO_SYSTEM_USAGE_SAFETY" },
            { AAUDIO_SYSTEM_USAGE_VEHICLE_STATUS, "AAUDIO_SYSTEM_USAGE_VEHICLE_STATUS" },
            { AAUDIO_SYSTEM_USAGE_ANNOUNCEMENT, "AAUDIO_SYSTEM_USAGE_ANNOUNCEMENT" }
        };

        if ( auto const findResult = map.find ( usage ); findResult != map.cend () )
            return findResult->second;

        std::snprintf ( unknownUsage, size, "UNKNOWN [%d]", static_cast<int> ( usage ) );
        return unknownUsage;
    };

    char unknownContentType[ size ];

    auto const getContentType = [ &unknownContentType ] ( aaudio_content_type_t contentType ) noexcept -> char const* {
        static std::unordered_map<aaudio_content_type_t, char const*> const map
        {
            { AAUDIO_CONTENT_TYPE_SPEECH, "AAUDIO_CONTENT_TYPE_SPEECH" },
            { AAUDIO_CONTENT_TYPE_MUSIC, "AAUDIO_CONTENT_TYPE_MUSIC" },
            { AAUDIO_CONTENT_TYPE_MOVIE, "AAUDIO_CONTENT_TYPE_MOVIE" },
            { AAUDIO_CONTENT_TYPE_SONIFICATION, "AAUDIO_CONTENT_TYPE_SONIFICATION" }
        };

        if ( auto const findResult = map.find ( contentType ); findResult != map.cend () )
            return findResult->second;

        std::snprintf ( unknownContentType, size, "UNKNOWN [%d]", static_cast<int> ( contentType ) );
        return unknownContentType;
    };

    char unknownFormat[ size ];

    LogInfo ( format,
        header,
        &stream,
        static_cast<int> ( AAudioStream_getBufferSizeInFrames ( &stream ) ),
        static_cast<int> ( AAudioStream_getFramesPerBurst ( &stream ) ),
        static_cast<int> ( AAudioStream_getBufferCapacityInFrames ( &stream ) ),
        static_cast<int> ( AAudioStream_getFramesPerDataCallback ( &stream ) ),
        static_cast<int> ( AAudioStream_getSampleRate ( &stream ) ),
        static_cast<int> ( AAudioStream_getChannelCount ( &stream ) ),
        static_cast<int> ( AAudioStream_getDeviceId ( &stream ) ),
        GetFormatString ( AAudioStream_getFormat ( &stream ), unknownFormat ),
        getSharingMode ( AAudioStream_getSharingMode ( &stream ) ),
        getPerformanceMode ( AAudioStream_getPerformanceMode ( &stream ) ),
        getDirection ( AAudioStream_getDirection ( &stream ) ),
        getSessionID ( AAudioStream_getSessionId ( &stream ) ),
        getUsage ( AAudioStream_getUsage ( &stream ) ),
        getContentType ( AAudioStream_getContentType ( &stream ) )
    );
}

} // namespace android_vulkan
