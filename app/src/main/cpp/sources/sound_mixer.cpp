#include <logger.h>
#include <sound_mixer.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>
#include <cinttypes>
#include <cstdio>
#include <unordered_map>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

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
    aaudio_result_t const result = AAudioStream_close ( &_stream );

    if ( !SoundMixer::CheckAAudioResult ( result, "StreamCloser::~StreamCloser", "Can't close stream" ) )
    {
        LogWarning ( "StreamCloser::~StreamCloser - Strange. Very strange,,," );
        assert ( false );
    }
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] bool SoundMixer::Init () noexcept
{
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
        [[maybe_unused]] bool const dummy = CheckAAudioResult ( result,
            "SoundMixer::Init",
            "Can't change probe buffer size"
        );

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
    return true;
}

void SoundMixer::Destroy () noexcept
{
    if ( !_builder )
        return;

    [[maybe_unused]] bool const result = CheckAAudioResult ( AAudioStreamBuilder_delete ( _builder ),
        "SoundMixer::Destroy",
        "Can't destroy builder"
    );

    _builder = nullptr;
}

[[maybe_unused]] size_t SoundMixer::GetBufferSampleCount () const noexcept
{
    return _bufferSampleCount;
}

bool SoundMixer::CheckAAudioResult ( aaudio_result_t result, char const* from, char const* message ) noexcept
{
    if ( result == AAUDIO_OK )
        return true;

    // Positive value. This case should be process differently by AAudio design.
    assert ( result > 0 );

    LogError ( "%s - %s. Error: %s.", from, message, AAudio_convertResultToText ( result ) );
    return false;
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
        [[maybe_unused]] bool const dummy = CheckAAudioResult ( result, "SoundMixer::ResolveBufferSize", msg );
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
    char unknownFormat[ size ];

    auto const getFormat = [ &unknownFormat ] ( aaudio_format_t fmt ) noexcept -> char const* {
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

        std::snprintf ( unknownFormat, size, "UNKNOWN [%d]", static_cast<int> ( fmt ) );
        return unknownFormat;
    };

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
        getFormat ( AAudioStream_getFormat ( &stream ) ),
        getSharingMode ( AAudioStream_getSharingMode ( &stream ) ),
        getPerformanceMode ( AAudioStream_getPerformanceMode ( &stream ) ),
        getDirection ( AAudioStream_getDirection ( &stream ) ),
        getSessionID ( AAudioStream_getSessionId ( &stream ) ),
        getUsage ( AAudioStream_getUsage ( &stream ) ),
        getContentType ( AAudioStream_getContentType ( &stream ) )
    );
}

} // namespace android_vulkan
