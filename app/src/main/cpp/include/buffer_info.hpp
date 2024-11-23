#ifndef ANDROID_VULKAN_BUFFER_INFO_HPP
#define ANDROID_VULKAN_BUFFER_INFO_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <vulkan/vulkan.h>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

struct BufferInfo final
{
    VkBuffer        _buffer = VK_NULL_HANDLE;
    VkDeviceSize    _range = 0U;
};

struct MeshBufferInfo final
{
    constexpr static size_t     POSITION_BUFFER_INDEX = 0U;
    constexpr static size_t     REST_DATA_BUFFER_INDEX = 1U;

    BufferInfo                  _postions {};
    BufferInfo                  _rest {};
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_BUFFER_INFO_HPP
