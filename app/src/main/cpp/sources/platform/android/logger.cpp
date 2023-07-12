#include <logger.hpp>
#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <android/log.h>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

constexpr static char const* TAG = "android_vulkan::C++";

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
