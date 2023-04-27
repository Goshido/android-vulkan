#include <logger.h>

#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <cstdarg>
#include <cstdio>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

void LogDebug ( char const* format, ... )
{
    va_list args;
    va_start ( args, format );
    std::vprintf ( format, args );
    va_end ( args );
    std::printf ( "\n" );
}

void LogError ( char const* format, ... )
{
    va_list args;
    va_start ( args, format );
    std::vprintf ( format, args );
    va_end ( args );
    std::printf ( "\n" );
}

void LogInfo ( char const* format, ... )
{
    va_list args;
    va_start ( args, format );
    std::vprintf ( format, args );
    va_end ( args );
    std::printf ( "\n" );
}

void LogWarning ( char const* format, ... )
{
    va_list args;
    va_start ( args, format );
    std::vprintf ( format, args );
    va_end ( args );
    std::printf ( "\n" );
}

} // namespace android_vulkan
