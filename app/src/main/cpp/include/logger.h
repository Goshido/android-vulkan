#ifndef ANDROID_VULKAN_LOGGER_H
#define ANDROID_VULKAN_LOGGER_H


namespace android_vulkan {

void LogDebug ( char const* format, ... );
void LogError ( char const* format, ... );
void LogInfo ( char const* format, ... );
void LogWarning ( char const* format, ... );

} // namespace android_vulkan


#endif // ANDROID_VULKAN_LOGGER_H
