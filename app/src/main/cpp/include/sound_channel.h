#ifndef ANDROID_VULKAN_SOUND_CHANNEL_H
#define ANDROID_VULKAN_SOUND_CHANNEL_H


namespace android_vulkan {

enum class eSoundChannel : size_t
{
    Music [[maybe_unused]] = 0U,
    Ambient [[maybe_unused]] = 1U,
    VFX [[maybe_unused]] = 2U,
    Speech = 3U
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_SOUND_CHANNEL_H
