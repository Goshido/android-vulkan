#include <logger.h>
#include <sound_mixer.h>

GX_DISABLE_COMMON_WARNINGS

#include <unordered_map>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

[[maybe_unused]] bool SoundMixer::Init () noexcept
{
    if ( !CheckAAudioResult ( AAudio_createStreamBuilder ( &_builder ), "SoundMixer::Init", "Can't create builder" ) )
    {
        Destroy ();
        return false;
    }

    // TODO
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

bool SoundMixer::CheckAAudioResult ( aaudio_result_t result, char const* from, char const* message ) noexcept
{
    if ( result == AAUDIO_OK )
        return true;

    LogError ( "%s - %s. Error: %s.", from, message, ResolveAAudioResult ( result ) );
    return false;
}

[[maybe_unused]] void SoundMixer::ResolveBurstSize () noexcept
{
    // TODO
}

char const* SoundMixer::ResolveAAudioResult ( aaudio_result_t result ) noexcept
{
    static std::unordered_map<aaudio_result_t, char const*> const map =
    {
        { AAUDIO_ERROR_BASE, "AAUDIO_ERROR_BASE" },
        { AAUDIO_ERROR_DISCONNECTED, "AAUDIO_ERROR_DISCONNECTED" },
        { AAUDIO_ERROR_ILLEGAL_ARGUMENT, "AAUDIO_ERROR_ILLEGAL_ARGUMENT" },
        { AAUDIO_ERROR_INTERNAL, "AAUDIO_ERROR_INTERNAL" },
        { AAUDIO_ERROR_INVALID_STATE, "AAUDIO_ERROR_INVALID_STATE" },
        { AAUDIO_ERROR_INVALID_HANDLE, "AAUDIO_ERROR_INVALID_HANDLE" },
        { AAUDIO_ERROR_UNIMPLEMENTED, "AAUDIO_ERROR_UNIMPLEMENTED" },
        { AAUDIO_ERROR_UNAVAILABLE, "AAUDIO_ERROR_UNAVAILABLE" },
        { AAUDIO_ERROR_NO_FREE_HANDLES, "AAUDIO_ERROR_NO_FREE_HANDLES" },
        { AAUDIO_ERROR_NO_MEMORY, "AAUDIO_ERROR_NO_MEMORY" },
        { AAUDIO_ERROR_NULL, "AAUDIO_ERROR_NULL" },
        { AAUDIO_ERROR_TIMEOUT, "AAUDIO_ERROR_TIMEOUT" },
        { AAUDIO_ERROR_WOULD_BLOCK, "AAUDIO_ERROR_WOULD_BLOCK" },
        { AAUDIO_ERROR_INVALID_FORMAT, "AAUDIO_ERROR_INVALID_FORMAT" },
        { AAUDIO_ERROR_OUT_OF_RANGE, "AAUDIO_ERROR_OUT_OF_RANGE" },
        { AAUDIO_ERROR_NO_SERVICE, "AAUDIO_ERROR_NO_SERVICE" },
        { AAUDIO_ERROR_INVALID_RATE, "AAUDIO_ERROR_INVALID_RATE" }
    };

    if ( auto const findResult = map.find ( result ); findResult != map.cend () )
        return findResult->second;

    constexpr static char const* unknownResult = "UNKNOWN";
    return unknownResult;
}

} // namespace android_vulkan
