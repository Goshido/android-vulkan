#ifndef ANDROID_VULKAN_WINDOWS_MESH_BUFFER_INFO_HPP
#define ANDROID_VULKAN_WINDOWS_MESH_BUFFER_INFO_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <vulkan/vulkan.h>

GX_RESTORE_WARNING_STATE


namespace android_vulkan::windows {

// Note if index buffer is present '_indexType' must not be VK_INDEX_TYPE_NONE_KHR.
// Index data offset is implicitly 0. Vertex data could contain:
// - single stream of arbitrary data
// - two streams: vertex positions, rest data
struct MeshBufferInfo final
{
    VkBuffer            _buffer = VK_NULL_HANDLE;

    VkDeviceAddress     _indexBDA = 0U;
    VkDeviceAddress     _positionBDA = 0U;
    VkDeviceAddress     _restBDA = 0U;

    VkDeviceSize        _vertexDataOffsets[ 2U ]{};
    VkDeviceSize        _vertexDataRanges[ 2U ]{};
    VkIndexType         _indexType = VK_INDEX_TYPE_NONE_KHR;
};

} // namespace android_vulkan::windows


#endif // ANDROID_VULKAN_WINDOWS_MESH_BUFFER_INFO_HPP
