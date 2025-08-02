#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <platform/windows/mesh_geometry.hpp>
#include <vulkan_api.hpp>
#include <vulkan_utils.hpp>


namespace android_vulkan::windows {

MeshBufferInfo const &MeshGeometry::GetMeshBufferInfo () const noexcept
{
    return _meshBufferInfo;
}

void MeshGeometry::CommitMeshInfo ( VkDevice device,
    VkIndexType indexType,
    StreamInfo &&stream0,
    std::optional<StreamInfo> &&stream1
) noexcept
{
    _meshBufferInfo._indexType = indexType;

    VkBufferDeviceAddressInfo info
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .pNext = nullptr,
        .buffer = _meshBufferInfo._buffer
    };

    VkDeviceAddress const address = vkGetBufferDeviceAddress ( device, &info );
    bool const hasStream1 = stream1.has_value ();

    constexpr auto assign = [] ( VkDeviceSize &offset, VkDeviceSize &range, StreamInfo const &stream ) noexcept {
        offset = static_cast<VkDeviceSize> ( stream._offset );
        range = static_cast<VkDeviceSize> ( stream._range );
    };

    assign ( _meshBufferInfo._vertexDataOffsets[ 0U ], _meshBufferInfo._vertexDataRanges[ 0U ], stream0 );

    if ( hasStream1 )
        assign ( _meshBufferInfo._vertexDataOffsets[ 1U ], _meshBufferInfo._vertexDataRanges[ 1U ], *stream1 );

    GX_DISABLE_WARNING ( 4061 )

    switch ( indexType )
    {
        case VK_INDEX_TYPE_NONE_KHR:
            _meshBufferInfo._bdaStream0 = address;

            if ( hasStream1 )
            {
                _meshBufferInfo._bdaStream1 = address + static_cast<VkDeviceAddress> ( stream1->_offset );
            }
        break;

        case VK_INDEX_TYPE_UINT16:
            [[fallthrough]];
        case VK_INDEX_TYPE_UINT32:
            _meshBufferInfo._bdaIndex = address;
            _meshBufferInfo._bdaStream0 = address + static_cast<VkDeviceAddress> ( stream0._offset );

            if ( hasStream1 )
            {
                _meshBufferInfo._bdaStream1 = address + static_cast<VkDeviceAddress> ( stream1->_offset );
            }
        break;

        default:
            // IMPOSSIBLE
            AV_ASSERT ( false )
        break;
    }

    GX_ENABLE_WARNING ( 4061 )
}

android_vulkan::MeshGeometry::BufferSyncItem const &MeshGeometry::GetBufferSync (
    BufferSyncItem::eType /*type*/
) const noexcept
{
    constexpr static BufferSyncItem sync
    {
        ._dstAccessMask = AV_VK_FLAG ( VK_ACCESS_SHADER_READ_BIT ) |
            AV_VK_FLAG ( VK_ACCESS_SHADER_WRITE_BIT ),

        ._usage = AV_VK_FLAG ( VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT ) |
            AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT ) |
            AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT )
    };

    return sync;
}

VkBuffer &MeshGeometry::GetDeviceBuffer () noexcept
{
    return _meshBufferInfo._buffer;
}

} // namespace android_vulkan::windows
