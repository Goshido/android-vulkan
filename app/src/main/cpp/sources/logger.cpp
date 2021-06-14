#include <logger.h>
#include <android/log.h>


namespace android_vulkan {

constexpr static char const* TAG = "android_vulkan::C++";

void LogDebug ( const char* format, ... )
{
    va_list args;
    va_start ( args, format );
    __android_log_vprint ( ANDROID_LOG_DEBUG, TAG, format, args );
    va_end ( args );
}

void LogError ( const char* format, ... )
{
    va_list args;
    va_start ( args, format );
    __android_log_vprint ( ANDROID_LOG_ERROR, TAG, format, args );
    va_end ( args );
}

void LogInfo ( const char* format, ... )
{
    va_list args;
    va_start ( args, format );
    __android_log_vprint ( ANDROID_LOG_INFO, TAG, format, args );
    va_end ( args );
}

void LogWarning ( const char* format, ... )
{
    va_list args;
    va_start ( args, format );
    __android_log_vprint ( ANDROID_LOG_WARN, TAG, format, args );
    va_end ( args );
}

} // namespace android_vulkan
