#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <platform/android/mesh_geometry.hpp>
#include <vulkan_utils.hpp>


namespace android_vulkan {

MeshBufferInfo const &MeshGeometry::GetMeshBufferInfo () const noexcept
{
    return _meshBufferInfo;
}

void MeshGeometry::CommitMeshInfo ( VkDevice /*device*/,
    VkIndexType indexType,
    StreamInfo &&stream0,
    std::optional<StreamInfo> &&stream1
) noexcept
{
    _meshBufferInfo._indexType = indexType;

    constexpr auto assign = [] ( VkDeviceSize &offset, VkDeviceSize &range, StreamInfo const &stream ) noexcept {
        offset = static_cast<VkDeviceSize> ( stream._offset );
        range = static_cast<VkDeviceSize> ( stream._range );
    };

    assign ( _meshBufferInfo._vertexDataOffsets[ 0U ], _meshBufferInfo._vertexDataRanges[ 0U ], stream0 );

    if ( stream1 )
    {
        assign ( _meshBufferInfo._vertexDataOffsets[ 1U ], _meshBufferInfo._vertexDataRanges[ 1U ], *stream1 );
    }
}

MeshGeometryBase::BufferSyncItem const &MeshGeometry::GetBufferSync (
    BufferSyncItem::eType type
) const noexcept
{
    constexpr static BufferSyncItem noIndexInfo
    {
        ._dstAccessMask = AV_VK_FLAG ( VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT ) |
            AV_VK_FLAG ( VK_ACCESS_SHADER_READ_BIT ),

        ._usage = AV_VK_FLAG ( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ) |
            AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT ) |
            AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT )
    };

    constexpr static BufferSyncItem withIndexInfo {
        ._dstAccessMask = AV_VK_FLAG ( VK_ACCESS_INDEX_READ_BIT ) |
            AV_VK_FLAG ( VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT ) |
            AV_VK_FLAG ( VK_ACCESS_SHADER_READ_BIT ),

        ._usage = AV_VK_FLAG ( VK_BUFFER_USAGE_INDEX_BUFFER_BIT ) |
            AV_VK_FLAG ( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ) |
            AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT ) |
            AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT )
    };

    switch ( type )
    {
        case BufferSyncItem::eType::NoIndexInfo:
        return noIndexInfo;

        case BufferSyncItem::eType::WithIndexInfo:
        return withIndexInfo;

        default:
            // IMPOSSIBLE
            AV_ASSERT ( false )
        return noIndexInfo;
    }
}

VkBuffer &MeshGeometry::GetDeviceBuffer () noexcept
{
    return _meshBufferInfo._buffer;
}

} // namespace android_vulkan
