#ifndef ANDROID_VULKAN_BUFFER_INFO_HPP
#define ANDROID_VULKAN_BUFFER_INFO_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <vulkan/vulkan.h>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

struct BufferInfo final
{
    VkBuffer        _buffer;
    VkDeviceSize    _range;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_BUFFER_INFO_HPP
