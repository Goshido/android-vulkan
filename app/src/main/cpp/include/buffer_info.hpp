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

// Note if index buffer is present '_indexType' should be not VK_INDEX_TYPE_MAX_ENUM.
// Index data offset is implicitly 0. Vertex data could contain:
// - single stream of arbitrary data
// - two streams: vertex positions, rest data
struct MeshBufferInfo final
{
    VkBuffer        _buffer = VK_NULL_HANDLE;
    VkIndexType     _indexType = VK_INDEX_TYPE_MAX_ENUM;
    VkDeviceSize    _vertexDataOffsets[ 2U ]{};
    VkDeviceSize    _vertexDataRanges[ 2U ]{};
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_BUFFER_INFO_HPP
