#include <precompiled_headers.hpp>
#include <logger.hpp>


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
