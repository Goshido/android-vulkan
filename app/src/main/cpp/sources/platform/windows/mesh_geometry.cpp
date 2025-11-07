#include <precompiled_headers.hpp>
#include <android_vulkan_sdk/mesh2.hpp>
#include <av_assert.hpp>
#include <file.hpp>
#include <logger.hpp>
#include <platform/windows/mesh_geometry.hpp>
#include <vulkan_api.hpp>
#include <vulkan_utils.hpp>


namespace android_vulkan {

namespace {

constexpr VkAccessFlags ACCESS_MASK = AV_VK_FLAG ( VK_ACCESS_SHADER_READ_BIT ) |
    AV_VK_FLAG ( VK_ACCESS_SHADER_WRITE_BIT );

constexpr VkBufferUsageFlags USAGE = AV_VK_FLAG ( VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT ) |
    AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT ) |
    AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT );

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void MeshGeometry::FreeResources ( Renderer &renderer ) noexcept
{
    FreeTransferResources ( renderer );
    FreeResourceInternal ( renderer );
    MakeUnique ();
}

void MeshGeometry::FreeTransferResources ( Renderer &renderer ) noexcept
{
    if ( _transferBuffer != VK_NULL_HANDLE )
        vkDestroyBuffer ( renderer.GetDevice (), std::exchange ( _transferBuffer, VK_NULL_HANDLE ), nullptr );

    Allocation &allocation = _transferAllocation;

    if ( allocation._memory == VK_NULL_HANDLE )
        return;

    renderer.FreeMemory ( std::exchange ( allocation._memory, VK_NULL_HANDLE ),
        std::exchange ( allocation._offset, 0U )
    );
}

MeshBufferInfo const &MeshGeometry::GetMeshBufferInfo () const noexcept
{
    return _meshBufferInfo;
}

[[maybe_unused]] bool MeshGeometry::LoadMesh ( Renderer &renderer,
    VkCommandBuffer commandBuffer,
    bool externalCommandBuffer,
    VkFence fence,
    std::string &&fileName
) noexcept
{
    if ( fileName.empty () ) [[unlikely]]
    {
        LogError ( "MeshGeometry::LoadMesh - Can't upload data. Filename is empty." );
        return false;
    }

    static std::regex const isMesh2 ( R"__(^.+?\.mesh2$)__" );
    std::smatch match;

    if ( !std::regex_match ( fileName, match, isMesh2 ) ) [[unlikely]]
    {
        LogError ( "MeshGeometry::LoadMesh - Mesh format is not supported: %s", fileName.c_str () );
        return false;
    }

    return LoadFromMesh2 ( renderer, commandBuffer, externalCommandBuffer, fence, std::move ( fileName ) );
}

[[maybe_unused]] bool MeshGeometry::LoadMesh ( Renderer &renderer,
    VkCommandBuffer commandBuffer,
    bool externalCommandBuffer,
    VkFence fence,
    AbstractData data,
    uint32_t vertexCount
) noexcept
{
    FreeResources ( renderer );

    VkBufferCreateInfo const bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size =  static_cast<VkDeviceSize> ( data.size () ),
        .usage = USAGE,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    bool result = CreateBuffer ( renderer,
        _meshBufferInfo._buffer,
        _gpuAllocation,
        bufferInfo,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Mesh"
    );

    if ( !result ) [[unlikely]]
        return false;

    UploadJob const job
    {
        ._data = data.data (),
        ._dstOffset = 0U,
        ._size = _gpuAllocation._range
    };

    result = GPUTransfer ( renderer,
        commandBuffer,
        externalCommandBuffer,
        fence,
        { &job, 1U }
    );

    if ( !result ) [[unlikely]]
        return false;

    CommitMeshInfo ( renderer.GetDevice (),
        VK_INDEX_TYPE_NONE_KHR,
        {
            ._offset = 0U,
            ._range = static_cast<size_t> ( _gpuAllocation._range )
        },

        std::nullopt
    );

    _vertexCount = vertexCount;
    _vertexBufferVertexCount = vertexCount;
    return true;
}

[[maybe_unused]] bool MeshGeometry::LoadMesh ( Renderer &renderer,
    VkCommandBuffer commandBuffer,
    bool externalCommandBuffer,
    VkFence fence,
    Indices16 indices,
    Positions positions,
    GXAABB const &bounds
) noexcept
{
    FreeResources ( renderer );

    size_t const indexCount = indices.size ();
    size_t const positionCount = positions.size ();

    bool const result = Upload ( renderer,
        commandBuffer,
        externalCommandBuffer,
        fence,
        { reinterpret_cast<uint8_t const*> ( indices.data () ), indexCount * sizeof ( uint16_t ) },
        VK_INDEX_TYPE_UINT16,
        { reinterpret_cast<uint8_t const*> ( positions.data () ), positionCount * sizeof ( GXVec3 ) },
        {},
        static_cast<uint32_t> ( positionCount )
    );

    if ( !result ) [[unlikely]]
        return false;

    _vertexCount = static_cast<uint32_t> ( indexCount );
    _vertexBufferVertexCount = static_cast<uint32_t> ( positionCount );
    _bounds = bounds;
    return true;
}

[[maybe_unused]] bool MeshGeometry::LoadMesh ( Renderer &renderer,
    VkCommandBuffer commandBuffer,
    bool externalCommandBuffer,
    VkFence fence,
    Indices32 indices,
    Positions positions,
    GXAABB const &bounds
) noexcept
{
    FreeResources ( renderer );

    size_t const indexCount = indices.size ();
    size_t const positionCount = positions.size ();

    bool const result = Upload ( renderer,
        commandBuffer,
        externalCommandBuffer,
        fence,
        { reinterpret_cast<uint8_t const*> ( indices.data () ), indexCount * sizeof ( uint32_t ) },
        VK_INDEX_TYPE_UINT32,
        { reinterpret_cast<uint8_t const*> ( positions.data () ), positionCount * sizeof ( GXVec3 ) },
        {},
        static_cast<uint32_t> ( positionCount )
    );

    if ( !result ) [[unlikely]]
        return false;

    _vertexCount = static_cast<uint32_t> ( indexCount );
    _vertexBufferVertexCount = static_cast<uint32_t> ( positionCount );
    _bounds = bounds;
    return true;
}

[[maybe_unused]] bool MeshGeometry::LoadMesh ( Renderer &renderer,
    VkCommandBuffer commandBuffer,
    bool externalCommandBuffer,
    VkFence fence,
    Indices16 indices,
    Positions positions,
    Vertices vertices,
    GXAABB const &bounds
) noexcept
{
    FreeResources ( renderer );

    size_t const indexCount = indices.size ();
    size_t const positionCount = positions.size ();
    size_t const vertexCount = vertices.size ();

    AV_ASSERT ( positionCount == vertexCount )

    bool const result = Upload ( renderer,
        commandBuffer,
        externalCommandBuffer,
        fence,
        { reinterpret_cast<uint8_t const*> ( indices.data () ), indexCount * sizeof ( uint16_t ) },
        VK_INDEX_TYPE_UINT16,
        { reinterpret_cast<uint8_t const*> ( positions.data () ), positionCount * sizeof ( GXVec3 ) },
        { vertices.data (), vertexCount },
        static_cast<uint32_t> ( positionCount )
    );

    if ( !result ) [[unlikely]]
        return false;

    _vertexCount = static_cast<uint32_t> ( indexCount );
    _vertexBufferVertexCount = static_cast<uint32_t> ( positionCount );
    _bounds = bounds;
    return true;
}

[[maybe_unused]] bool MeshGeometry::LoadMesh ( Renderer &renderer,
    VkCommandBuffer commandBuffer,
    bool externalCommandBuffer,
    VkFence fence,
    Indices32 indices,
    Positions positions,
    Vertices vertices,
    GXAABB const &bounds
) noexcept
{
    FreeResources ( renderer );

    size_t const indexCount = indices.size ();
    size_t const positionCount = positions.size ();
    size_t const vertexCount = vertices.size ();

    AV_ASSERT ( positionCount == vertexCount )

    bool const result = Upload ( renderer,
        commandBuffer,
        externalCommandBuffer,
        fence,
        { reinterpret_cast<uint8_t const*> ( indices.data () ), indexCount * sizeof ( uint32_t ) },
        VK_INDEX_TYPE_UINT32,
        { reinterpret_cast<uint8_t const*> ( positions.data () ), positionCount * sizeof ( GXVec3 ) },
        { vertices.data (), vertexCount },
        static_cast<uint32_t> ( positionCount )
    );

    if ( !result ) [[unlikely]]
        return false;

    _vertexCount = static_cast<uint32_t> ( indexCount );
    _vertexBufferVertexCount = static_cast<uint32_t> ( positionCount );
    _bounds = bounds;
    return true;
}

void MeshGeometry::FreeResourceInternal ( Renderer &renderer ) noexcept
{
    _vertexCount = 0U;

    if ( VkBuffer &buffer = _meshBufferInfo._buffer; buffer != VK_NULL_HANDLE ) [[likely]]
        vkDestroyBuffer ( renderer.GetDevice (), std::exchange ( buffer, VK_NULL_HANDLE ), nullptr );

    if ( _gpuAllocation._memory == VK_NULL_HANDLE ) [[unlikely]]
        return;

    renderer.FreeMemory ( std::exchange ( _gpuAllocation._memory, VK_NULL_HANDLE ),
        std::exchange ( _gpuAllocation._offset, 0U )
    );
}

void MeshGeometry::CommitMeshInfo ( VkDevice device,
    VkIndexType indexType,
    StreamInfo &&stream0,
    std::optional<StreamInfo> &&stream1
) noexcept
{
    _meshBufferInfo._indexType = indexType;

    VkBufferDeviceAddressInfo const info
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

bool MeshGeometry::GPUTransfer ( Renderer &renderer,
    VkCommandBuffer commandBuffer,
    bool externalCommandBuffer,
    VkFence fence,
    UploadJobs jobs
) noexcept
{
    size_t const dataSize = [ &jobs ] () -> size_t {
        size_t size = 0U;

        for ( UploadJob const &job : jobs )
            size += job._size;

        return size;
    } ();

    constexpr VkMemoryPropertyFlags flags = AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ) |
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );

    bool result = CreateBuffer ( renderer,
        _transferBuffer,
        _transferAllocation,

        {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0U,
            .size = static_cast<VkDeviceSize> ( dataSize ),
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0U,
            .pQueueFamilyIndices = nullptr
        },

        flags,
        "Mesh staging buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    void* transferData = nullptr;

    result = renderer.MapMemory ( transferData,
        _transferAllocation._memory,
        _transferAllocation._offset,
        "MeshGeometry::GPUTransfer",
        "Can't map data"
    );

    if ( !result ) [[unlikely]]
        return false;

    auto* writePtr = static_cast<uint8_t*> ( transferData );

    for ( UploadJob const &job : jobs )
    {
        std::memcpy ( writePtr, job._data , job._size );
        writePtr += static_cast<size_t> ( job._size );
    }

    renderer.UnmapMemory ( _transferAllocation._memory );

    if ( !externalCommandBuffer )
    {
        constexpr VkCommandBufferBeginInfo commandBufferBeginInfo
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            .pInheritanceInfo = nullptr
        };

        result = Renderer::CheckVkResult ( vkBeginCommandBuffer ( commandBuffer, &commandBufferBeginInfo ),
            "MeshGeometry::GPUTransfer",
            "Can't begin command buffer"
        );

        if ( !result ) [[unlikely]]
        {
            return false;
        }
    }

    // Note most extreme case is 3 upload jobs: index buffer, position buffer and 'rest data' buffer.
    VkBufferCopy bufferCopy[ 3U ];
    size_t const jobCount = jobs.size ();
    AV_ASSERT ( jobCount <= std::size ( bufferCopy ) )
    VkDeviceSize offset = 0U;

    for ( size_t i = 0U; i < jobCount; ++i )
    {
        UploadJob const &job = jobs[ i ];
        VkDeviceSize const size = job._size;

        bufferCopy[ i ] =
        {
            .srcOffset = offset,
            .dstOffset = job._dstOffset,
            .size = size
        };

        offset += size;
    }

    VkBuffer buffer = _meshBufferInfo._buffer;
    vkCmdCopyBuffer ( commandBuffer, _transferBuffer, buffer, static_cast<uint32_t> ( jobCount ), bufferCopy );
    UploadJob const &last = jobs.back ();

    VkBufferMemoryBarrier const barrierInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = ACCESS_MASK,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = buffer,
        .offset = 0U,
        .size = last._dstOffset + last._size
    };

    constexpr VkPipelineStageFlags dstStage = AV_VK_FLAG ( VK_PIPELINE_STAGE_VERTEX_SHADER_BIT ) |
        AV_VK_FLAG ( VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT );

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        dstStage,
        0U,
        0U,
        nullptr,
        1U,
        &barrierInfo,
        0U,
        nullptr
    );

    if ( externalCommandBuffer )
        return true;

    result = Renderer::CheckVkResult ( vkEndCommandBuffer ( commandBuffer ),
        "MeshGeometry::GPUTransfer",
        "Can't end command buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    VkSubmitInfo const submitInfo
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 0U,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = nullptr,
        .commandBufferCount = 1U,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = 0U,
        .pSignalSemaphores = nullptr
    };

    return Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &submitInfo, fence ),
        "MeshGeometry::GPUTransfer",
        "Can't submit command"
    );
}

bool MeshGeometry::LoadFromMesh2 ( Renderer &renderer,
    VkCommandBuffer commandBuffer,
    bool externalCommandBuffer,
    VkFence fence,
    std::string &&fileName
) noexcept
{
    FreeResourceInternal ( renderer );
    File file ( fileName );

    if ( !file.LoadContent () ) [[unlikely]]
        return false;

    std::vector<uint8_t> const &content = file.GetContent ();
    uint8_t const* rawData = content.data ();
    auto const &header = *reinterpret_cast<Mesh2Header const*> ( rawData );

    auto const selector = static_cast<size_t> ( header._indexCount >= INDEX16_LIMIT );
    constexpr VkIndexType const cases[] = { VK_INDEX_TYPE_UINT16, VK_INDEX_TYPE_UINT32 };

    bool const result = Upload ( renderer,
        commandBuffer,
        externalCommandBuffer,
        fence,

        {
            rawData + static_cast<size_t> ( header._indexDataOffset ),
            header._indexCount * INDEX_SIZES[ selector ]
        },

        cases[ selector ],

        {
            rawData + static_cast<size_t> ( header._positionDataOffset ),
            static_cast<size_t> ( header._vertexCount ) * sizeof ( GXVec3 )
        },

        {
            reinterpret_cast<VertexInfo const*> ( rawData + static_cast<size_t> ( header._vertexDataOffset ) ),
            static_cast<size_t> ( header._vertexCount )
        },

        static_cast<uint32_t> ( header._vertexCount )
    );

    if ( !result ) [[unlikely]]
        return false;

    Vec3 const &mins = header._bounds._min;
    Vec3 const &maxs = header._bounds._max;
    _bounds.Empty ();
    _bounds.AddVertex ( mins[ 0U ], mins[ 1U ], mins[ 2U ] );
    _bounds.AddVertex ( maxs[ 0U ], maxs[ 1U ], maxs[ 2U ] );

    _vertexCount = static_cast<uint32_t> ( header._indexCount );
    _vertexBufferVertexCount = header._vertexCount;
    _fileName = std::move ( fileName );
    return true;
}

bool MeshGeometry::Upload ( Renderer &renderer,
    VkCommandBuffer commandBuffer,
    bool externalCommandBuffer,
    VkFence fence,
    AbstractData indices,
    VkIndexType indexType,
    AbstractData vertexStream0,
    Vertices vertexStream1,
    uint32_t vertexCount
) noexcept
{
    AV_ASSERT ( indexType == VK_INDEX_TYPE_UINT16 || indexType == VK_INDEX_TYPE_UINT32 )

    size_t const indSize = indices.size ();

    size_t const posSize = vertexStream0.size ();
    auto const restSize = static_cast<size_t> ( vertexCount * sizeof ( VertexInfo ) );

    size_t const vertexAlignment = std::max ( static_cast<size_t> ( 4U ),
        renderer.GetMinStorageBufferOffsetAlignment ()
    );

    size_t sizeLeft = vertexAlignment + posSize + vertexAlignment + restSize;
    auto* ptr = reinterpret_cast<void*> ( indSize );

    auto const posOffset = reinterpret_cast<size_t> ( std::align ( vertexAlignment, posSize, ptr, sizeLeft ) );
    ptr = static_cast<uint8_t*> ( ptr ) + posSize;
    sizeLeft -= posSize;

    auto const restOffset = reinterpret_cast<size_t> ( std::align ( vertexAlignment, restSize, ptr, sizeLeft ) );

    VkBufferCreateInfo bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = static_cast<VkDeviceSize> ( restOffset + restSize ),
        .usage = USAGE,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    bool result = CreateBuffer ( renderer,
        _meshBufferInfo._buffer,
        _gpuAllocation,
        bufferInfo,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Mesh"
    );

    if ( !result ) [[unlikely]]
        return false;

    if ( vertexStream1.empty () )
    {
        UploadJob const jobs[]
        {
            {
                ._data = indices.data (),
                ._dstOffset = 0U,
                ._size = static_cast<VkDeviceSize> ( indSize )
            },
            {
                ._data = vertexStream0.data (),
                ._dstOffset = static_cast<VkDeviceSize> ( posOffset ),
                ._size = static_cast<VkDeviceSize> ( posSize )
            }
        };

        result = GPUTransfer ( renderer,
            commandBuffer,
            externalCommandBuffer,
            fence,
            { jobs, std::size ( jobs ) }
        );

        if ( !result ) [[unlikely]]
            return false;

        CommitMeshInfo ( renderer.GetDevice (),
            indexType,
            {
                ._offset = posOffset,
                ._range = posSize
            },

            std::nullopt
        );

        return true;
    }

    UploadJob const jobs[]
    {
        {
            ._data = indices.data (),
            ._dstOffset = 0U,
            ._size = static_cast<VkDeviceSize> ( indSize )
        },
        {
            ._data = vertexStream0.data (),
            ._dstOffset = static_cast<VkDeviceSize> ( posOffset ),
            ._size = static_cast<VkDeviceSize> ( posSize )
        },
        {
            ._data = vertexStream1.data (),
            ._dstOffset = static_cast<VkDeviceSize> ( restOffset ),
            ._size = static_cast<VkDeviceSize> ( restSize )
        }
    };

    result = GPUTransfer ( renderer,
        commandBuffer,
        externalCommandBuffer,
        fence,
        { jobs, std::size ( jobs ) }
    );

    if ( !result ) [[unlikely]]
        return false;

    CommitMeshInfo ( renderer.GetDevice (),
        indexType,
        {
            ._offset = posOffset,
            ._range = posSize
        },

        std::optional<StreamInfo> {
            {
                ._offset = restOffset,
                ._range = restSize
            }
        }
    );

    return true;
}

} // namespace android_vulkan
