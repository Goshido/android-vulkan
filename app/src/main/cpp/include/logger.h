#ifndef ANDROID_VULKAN_LOGGER_H
#define ANDROID_VULKAN_LOGGER_H


namespace android_vulkan {

void LogDebug ( const char* format, ... );
void LogError ( const char* format, ... );
void LogInfo ( const char* format, ... );
void LogWarning ( const char* format, ... );

} // namespace android_vulkan


#endif // ANDROID_VULKAN_LOGGER_H
