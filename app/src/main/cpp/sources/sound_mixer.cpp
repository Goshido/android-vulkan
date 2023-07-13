#include <av_assert.hpp>
#include <logger.hpp>
#include <sound_mixer.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <cinttypes>
#include <cstdio>
#include <unordered_map>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

namespace {

constexpr float CHANNEL_VOLUME = 1.0F;
constexpr size_t MAX_HARDWARE_STREAM_CAP = 32U;
constexpr double TRIM_TIMEOUT_SECONDS = 5.0F;
constexpr auto WORKER_TIMEOUT = std::chrono::microseconds ( 1U );

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
}

class StreamCloser final
{
    private:
        AAudioStream        &_stream;

    public:
        StreamCloser () = delete;

        StreamCloser ( StreamCloser const & ) = delete;
        StreamCloser &operator = ( StreamCloser const & ) = delete;

        StreamCloser ( StreamCloser && ) = delete;
        StreamCloser &operator = ( StreamCloser && ) = delete;

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

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

SoundMixer::StreamInfo::StreamInfo ( SoundMixer &mixer, StreamList::iterator used ) noexcept:
    _mixer ( &mixer ),
    _used ( used )
{
    // NOTHING
}

bool SoundMixer::StreamInfo::LockAndReturnFillSilence () noexcept
{
    auto &fillLock = *_fillLock;
    bool expected = false;

    while ( !fillLock.compare_exchange_weak ( expected, true ) )
    {}

    AV_ASSERT ( !expected )
    return _fillSilence;
}

void SoundMixer::StreamInfo::Release () noexcept
{
    _fillLock->store ( false );
}

void SoundMixer::StreamInfo::Modify ( bool fillSilence ) noexcept
{
    auto &fillLock = *_fillLock;
    bool expected = false;

    while ( !fillLock.compare_exchange_weak ( expected, true ) )
    {}

    AV_ASSERT ( !expected )
    _fillSilence = fillSilence;
    fillLock.store ( false );
}

//----------------------------------------------------------------------------------------------------------------------

void SoundMixer::CheckMemoryLeaks () noexcept
{

#ifdef ANDROID_VULKAN_DEBUG

    if ( !_streamMap.empty () )
    {
        constexpr char const format[] = R"__(SoundMixer::Destroy - Memory leak detected: %zu
>>>)__";

        LogError ( format, _streamMap.size () );

        for ( auto const &record : _streamMap )
        {
            StreamInfo const &si = *record.second;
            LogError ( "    %s", si._emitter->GetFile ().c_str () );
        }

        LogError ( "<<<" );

#ifdef ANDROID_VULKAN_STRICT_MODE

        AV_ASSERT ( false )

#endif // ANDROID_VULKAN_STRICT_MODE

    }

#endif // ANDROID_VULKAN_DEBUG

    _streamMap.clear ();
}

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

    if ( !CreateHardwareStreams () )
    {
        Destroy ();
        return false;
    }

    PrintStreamInfo ( *_streamInfo.front ()._stream, "Engine parameters" );
    _workerFlag = true;

    _workerThread = std::thread (
        [ this ] () noexcept {
            auto last = std::chrono::system_clock::now ();

            while ( _workerFlag )
            {
                // To not consume too much CPU time.
                std::this_thread::sleep_for ( WORKER_TIMEOUT );

                std::unique_lock<std::mutex> const lock ( _mutex );

                for ( auto* streamer : _decompressors )
                    streamer->OnDecompress ();

                for ( auto const &action : _actionQueue )
                {
                    // [2022/12/31] For example play operation could take 6 ms. So moving execution in another thread.
                    std::thread actionThread (
                        [] ( ActionHandler handler, AAudioStream* stream ) noexcept {
                            handler ( *stream );
                        },

                        action._handler,
                        action._stream
                    );

                    actionThread.detach ();
                }

                _actionQueue.clear ();

                auto const now = std::chrono::system_clock::now ();

                if ( std::chrono::duration<double> const delta = now - last; delta.count () < TRIM_TIMEOUT_SECONDS )
                    continue;

                last = now;
                _soundStorage.Trim ();
            }
        }
    );

    return true;
}

void SoundMixer::Destroy () noexcept
{
    if ( _workerThread.joinable () )
    {
        _workerFlag = false;
        _workerThread.join ();
    }

    std::unique_lock<std::mutex> const lock ( _mutex );

    for ( auto &si : _streamInfo )
    {
        SoundMixer::CheckAAudioResult ( AAudioStream_close ( si._stream ),
            "SoundMixer::Destroy",
            "Can't close stream"
        );
    }

    _free.clear ();
    _used.clear ();

    if ( !_builder )
        return;

    CheckAAudioResult ( AAudioStreamBuilder_delete ( _builder ), "SoundMixer::Destroy", "Can't destroy builder" );
    _builder = nullptr;
}

size_t SoundMixer::GetBufferSampleCount () const noexcept
{
    return _bufferSampleCount;
}

[[maybe_unused]] float SoundMixer::GetChannelVolume ( eSoundChannel channel ) const noexcept
{
    return _channelVolume[ static_cast<size_t> ( channel ) ];
}

void SoundMixer::SetChannelVolume ( eSoundChannel channel, float volume ) noexcept
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

SoundListenerInfo const &SoundMixer::GetListenerInfo () noexcept
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

void SoundMixer::SetListenerLocation ( GXVec3 const &location ) noexcept
{
    _listenerInfo._location = location;
    _listenerTransformChanged = true;
}

SoundStorage &SoundMixer::GetSoundStorage () noexcept
{
    return _soundStorage;
}

void SoundMixer::SetListenerOrientation ( GXQuat const &orientation ) noexcept
{
    _listenerOrientation = orientation;
    _listenerTransformChanged = true;
}

bool SoundMixer::IsOffline () const noexcept
{
    return !_workerFlag;
}

void SoundMixer::Pause () noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );

    if ( IsOffline () )
        return;

    for ( auto &record : _streamMap )
    {
        StreamInfo const &si = *record.second;

        if ( si._emitter->IsPlaying () )
        {
            AAudioStream* stream = si._stream;

            _actionQueue.emplace_back (
                ActionInfo
                {
                    ._handler = &SoundMixer::ExecutePause,
                    ._stream = stream
                }
            );

            _streamToResume.push_back ( stream );
        }
    }
}

void SoundMixer::Resume () noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );

    if ( IsOffline () )
        return;

    for ( auto* stream : _streamToResume )
    {
        _actionQueue.emplace_back (
            ActionInfo
            {
                ._handler = &SoundMixer::ExecutePlay,
                ._stream = stream
            }
        );
    }

    _streamToResume.clear ();
}

bool SoundMixer::RequestPause ( SoundEmitter &emitter ) noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );

    if ( IsOffline () )
        return true;

    auto findResult = _streamMap.find ( &emitter );

    if ( findResult == _streamMap.end () )
    {
        LogWarning ( "SoundMixer::RequestPause - Can't find emitter. Abort..." );
        AV_ASSERT ( false )
        return false;
    }

    StreamInfo &si = *findResult->second;
    si.Modify ( true );

    _free.splice ( _free.begin (), _used, si._used );
    _streamMap.erase ( findResult );

    _actionQueue.emplace_back (
        ActionInfo
        {
            ._handler = &SoundMixer::ExecutePause,
            ._stream = si._stream
        }
    );

    return true;
}

bool SoundMixer::RequestPlay ( SoundEmitter &emitter ) noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );

    if ( IsOffline () )
        return true;

    if ( _free.empty () )
    {
        LogWarning ( "SoundMixer::RequestPlay - No more free streams. Abort..." );
        return false;
    }

    _used.splice ( _used.cbegin (), _free, _free.cbegin () );

    StreamInfo &si = *_used.front ();
    si.Modify ( false );

    si._emitter = &emitter;
    si._used = _used.begin ();
    _streamMap.emplace ( &emitter, &si );

    _actionQueue.emplace_back (
        ActionInfo
        {
            ._handler = &SoundMixer::ExecutePlay,
            ._stream = si._stream
        }
    );

    return true;
}

bool SoundMixer::RequestStop ( SoundEmitter &emitter ) noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );

    auto findResult = _streamMap.find ( &emitter );

    if ( findResult == _streamMap.end () )
    {
        LogWarning ( "SoundMixer::RequestStop - Can't find emitter. Abort..." );
        AV_ASSERT ( false )
        return false;
    }

    if ( IsOffline () )
    {
        _streamMap.erase ( findResult );
        return true;
    }

    StreamInfo &si = *findResult->second;
    si.Modify ( true );

    _free.splice ( _free.begin (), _used, si._used );
    _streamMap.erase ( findResult );

    _actionQueue.emplace_back (
        ActionInfo
        {
            ._handler = &SoundMixer::ExecuteStop,
            ._stream = si._stream
        }
    );

    return true;
}

void SoundMixer::RegisterDecompressor ( PCMStreamer &streamer ) noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );

    if ( _decompressors.contains ( &streamer ) )
    {
        LogError ( "SoundMixer::RegisterDecompressor - Decompressor already registered. Please check business logic." );
        AV_ASSERT ( false )
        return;
    }

    _decompressors.insert ( &streamer );
}

void SoundMixer::UnregisterDecompressor ( PCMStreamer &streamer ) noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );

    if ( _decompressors.erase ( &streamer ) > 0U )
        return;

    LogError ( "SoundMixer::UnregisterDecompressor - Can't find decompressor. Please check business logic." );
    AV_ASSERT ( false )
}

bool SoundMixer::CheckAAudioResult ( aaudio_result_t result, char const* from, char const* message ) noexcept
{
    if ( result == AAUDIO_OK )
        return true;

    // Positive value. This case should be process differently by AAudio design.
    AV_ASSERT ( result < 0 )

    LogError ( "%s - %s. Error: %s.", from, message, AAudio_convertResultToText ( result ) );
    return false;
}

bool SoundMixer::CreateHardwareStreams () noexcept
{
    _streamInfo.reserve ( MAX_HARDWARE_STREAM_CAP );
    size_t counter = 0U;

    for ( ; counter < MAX_HARDWARE_STREAM_CAP; ++counter )
    {
        StreamInfo &si = _streamInfo.emplace_back ( StreamInfo ( *this, _used.end () ) );
        AAudioStreamBuilder_setDataCallback ( _builder, &SoundMixer::PCMCallback, &si );

        if ( AAudioStreamBuilder_openStream ( _builder, &si._stream ) != AAUDIO_OK )
        {
            _streamInfo.pop_back ();
            break;
        }

        if ( !SetStreamBufferSize ( *si._stream, "SoundMixer::CreateHardwareStreams" ) )
        {
            AV_ASSERT ( false )
            return false;
        }

        if ( !ValidateStream ( *si._stream ) )
            return false;

        _free.push_back ( &si );
    }

    if ( counter < 1U )
    {
        LogError ( "SoundMixer::CreateHardwareStreams - No available hardware streams!" );
        AV_ASSERT ( false )
        return false;
    }

    LogInfo ( "SoundMixer::CreateHardwareStreams - Hardware streams: %zu.", counter );
    return true;
}

std::optional<AAudioStream*> SoundMixer::CreateStream ( StreamInfo &streamInfo) noexcept
{
    AAudioStreamBuilder_setDataCallback ( _builder, &SoundMixer::PCMCallback, &streamInfo );

    AAudioStream* stream = nullptr;
    aaudio_result_t result = AAudioStreamBuilder_openStream ( _builder, &stream );

    if ( !CheckAAudioResult ( result, "SoundMixer::CreateStream", "Can't open stream" ) )
        return std::nullopt;

    if ( !SetStreamBufferSize ( *stream, "SoundMixer::CreateStream" ) )
        return std::nullopt;

    if ( !ValidateStream ( *stream ) )
        return std::nullopt;

    return stream;
}

void SoundMixer::RecreateSoundStream ( AAudioStream &stream ) noexcept
{
    std::thread thread (
        [ this, &stream ] () noexcept {
            std::unique_lock<std::mutex> const lock ( _mutex );

            StreamCloser const closer ( stream );
            StreamInfo* targetStreamInfo = nullptr;

            for ( auto &si : _streamInfo )
            {
                if ( si._stream == &stream )
                {
                    targetStreamInfo = &si;
                    break;
                }
            }

            if ( !targetStreamInfo )
            {
                LogError ( "SoundMixer::RecreateSoundStream - Can't find stream." );
                AV_ASSERT ( false )
                return;
            }

            if ( auto result = CreateStream ( *targetStreamInfo ); result != std::nullopt )
            {
                targetStreamInfo->_stream = *result;
                return;
            }

            LogError ( "SoundMixer::RecreateSoundStream - Can't create new AAudioStream." );
            AV_ASSERT ( false )
        }
    );

    thread.detach ();
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

    StreamCloser const streamCloser ( *probe );
    result = AAudioStream_setBufferSizeInFrames ( probe, burst );

    if ( result < 0 )
    {
        // Error happens.
        char msg[ 128U ];
        std::snprintf ( msg, std::size ( msg ), "Can't change stream buffer size to %" PRIi32, burst );
        CheckAAudioResult ( result, "SoundMixer::ResolveBufferSize", msg );
        return false;
    }

    _bufferFrameCount = static_cast<int32_t> ( result );
    _bufferSampleCount = static_cast<size_t> ( result ) * static_cast<size_t> ( GetChannelCount () );
    return true;
}

bool SoundMixer::SetStreamBufferSize ( AAudioStream &stream, char const* where ) const noexcept
{
    aaudio_result_t const result = AAudioStream_setBufferSizeInFrames ( &stream, _bufferFrameCount );

    if ( result > 0 )
        return true;

    // Error happened.
    char msg[ 128U ];
    std::snprintf ( msg, std::size ( msg ), "Can't change stream buffer size to %" PRIi32, _bufferFrameCount );
    CheckAAudioResult ( result, where, msg );
    AV_ASSERT ( false )
    return false;
}

bool SoundMixer::ValidateStream ( AAudioStream &stream ) const noexcept
{
    if ( int32_t const sampleRate = AAudioStream_getSampleRate ( &stream ); sampleRate != GetSampleRate () )
    {
        StreamCloser const streamCloser ( stream );

        LogError ( "SoundMixer::ValidateStream - Unexpected sample rate %" PRIi32 ". Should be %" PRIi32 ".",
            sampleRate,
            GetSampleRate ()
        );

        AV_ASSERT ( false )
        return false;
    }

    if ( aaudio_format_t const format = AAudioStream_getFormat ( &stream ); format != GetFormat () )
    {
        StreamCloser const streamCloser ( stream );

        constexpr size_t size = 128U;
        char current[ size ];
        char expected[ size ];

        LogError ( "SoundMixer::ValidateStream - Unexpected format %s. Should be %s.",
            GetFormatString ( format, current ),
            GetFormatString ( GetFormat (), expected )
        );

        AV_ASSERT ( false )
        return false;
    }

    if ( int32_t const channels = AAudioStream_getChannelCount ( &stream ); channels != GetChannelCount () )
    {
        StreamCloser const streamCloser ( stream );

        LogError ( "SoundMixer::ValidateStream - Unexpected channel count %" PRIi32 ". Should be %" PRIi32 ".",
            channels,
            GetChannelCount ()
        );

        AV_ASSERT ( false )
        return false;
    }

    int32_t const frames = AAudioStream_setBufferSizeInFrames ( &stream, _bufferFrameCount );

    if ( frames == _bufferFrameCount )
        return true;

    StreamCloser const streamCloser ( stream );

    LogError ( "SoundMixer::ValidateStream - Unexpected buffer frame count %" PRIi32 ". Should be %" PRIi32 ".",
        frames,
        _bufferFrameCount
    );

    AV_ASSERT ( false )
    return false;
}

void SoundMixer::ErrorCallback ( AAudioStream* stream, void* userData, aaudio_result_t err )
{
    CheckAAudioResult ( err, "SoundMixer::ErrorCallback", "Got error from AAudio subsystem" );

    if ( err != AAUDIO_ERROR_DISCONNECTED )
        return;

    LogInfo ( "SoundMixer::ErrorCallback - Main audio endpoint has been disconnected, Trying to restore sound..." );
    auto &soundMixer = *static_cast<SoundMixer*> ( userData );
    soundMixer.RecreateSoundStream ( *stream );
}

void SoundMixer::ExecutePause ( AAudioStream &stream ) noexcept
{
    SoundMixer::CheckAAudioResult ( AAudioStream_requestPause ( &stream ),
        "SoundMixer::ExecutePause",
        "Can't pause"
    );
}

void SoundMixer::ExecutePlay ( AAudioStream &stream ) noexcept
{
    SoundMixer::CheckAAudioResult ( AAudioStream_requestStart ( &stream ),
        "SoundMixer::ExecutePlay",
        "Can't play"
    );
}

void SoundMixer::ExecuteStop ( AAudioStream &stream ) noexcept
{
    SoundMixer::CheckAAudioResult ( AAudioStream_requestStop ( &stream ),
        "SoundMixer::ExecuteStop",
        "Can't stop"
    );
}

aaudio_data_callback_result_t SoundMixer::PCMCallback ( AAudioStream* /*stream*/,
    void* userData,
    void* audioData,
    int32_t numFrames
)
{
    auto &si = *static_cast<StreamInfo*> ( userData );
    constexpr auto sizeFactor = static_cast<size_t> ( GetChannelCount () ) * sizeof ( PCMStreamer::PCMType );

    if ( si._mixer->IsOffline () )
    {
        std::memset ( audioData, 0, static_cast<size_t> ( numFrames * sizeFactor ) );
        return AAUDIO_CALLBACK_RESULT_STOP;
    }

    if ( si.LockAndReturnFillSilence () )
    {
        std::memset ( audioData, 0,  static_cast<size_t> ( numFrames * sizeFactor ) );
        si.Release ();
        return AAUDIO_CALLBACK_RESULT_CONTINUE;
    }

    SoundMixer* mixer = si._mixer;

    si._emitter->FillPCM (
        std::span ( static_cast<PCMStreamer::PCMType*> ( audioData ),
            static_cast<size_t> ( numFrames * GetChannelCount () )
        ),

        mixer->_effectiveChannelVolume[ static_cast<size_t> ( si._emitter->GetSoundChannel () ) ]
    );

    si.Release ();
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
