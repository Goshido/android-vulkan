#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <GXCommon/GXWarning.hpp>


namespace android_vulkan {

namespace {

constexpr char const* TAG = "android_vulkan::C++";

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void LogDebug ( char const* format, ... )
{
    va_list args;
    va_start ( args, format );
    __android_log_vprint ( ANDROID_LOG_DEBUG, TAG, format, args );
    va_end ( args );
}

void LogError ( char const* format, ... )
{
    va_list args;
    va_start ( args, format );
    __android_log_vprint ( ANDROID_LOG_ERROR, TAG, format, args );
    va_end ( args );
}

void LogInfo ( char const* format, ... )
{
    va_list args;
    va_start ( args, format );
    __android_log_vprint ( ANDROID_LOG_INFO, TAG, format, args );
    va_end ( args );
}

void LogWarning ( char const* format, ... )
{
    va_list args;
    va_start ( args, format );
    __android_log_vprint ( ANDROID_LOG_WARN, TAG, format, args );
    va_end ( args );
}

} // namespace android_vulkan
