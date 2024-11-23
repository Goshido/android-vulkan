#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <mesh_geometry.hpp>
#include <file.hpp>
#include <logger.hpp>
#include <vulkan_utils.hpp>
#include <android_vulkan_sdk/mesh2.hpp>


namespace android_vulkan {

namespace {

constexpr VkFlags VERTEX_BUFFER_USAGE = AV_VK_FLAG ( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ) |
    AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT );

constexpr size_t INDEX16_LIMIT = 1U << 16U;
constexpr size_t const INDEX_SIZES[] = { sizeof ( uint16_t ), sizeof ( uint32_t ) };

//----------------------------------------------------------------------------------------------------------------------

struct BufferSyncItem final
{
    VkAccessFlags           _dstAccessMask;
    VkPipelineStageFlags    _dstStage;
    VkAccessFlags           _srcAccessMask;
    VkPipelineStageFlags    _srcStage;

    constexpr explicit BufferSyncItem ( VkAccessFlags srcAccessMask,
        VkPipelineStageFlags srcStage,
        VkAccessFlags dstAccessMask,
        VkPipelineStageFlags dstStage
    ) noexcept:
        _dstAccessMask ( dstAccessMask ),
        _dstStage ( dstStage ),
        _srcAccessMask ( srcAccessMask ),
        _srcStage ( srcStage )
    {
        // NOTHING
    }
};

std::map<VkBufferUsageFlags, BufferSyncItem> const g_accessMapper =
{
    {
        VERTEX_BUFFER_USAGE,

        BufferSyncItem ( VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            AV_VK_FLAG ( VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT ) | AV_VK_FLAG ( VK_ACCESS_SHADER_READ_BIT ),
            AV_VK_FLAG ( VK_PIPELINE_STAGE_VERTEX_INPUT_BIT ) | AV_VK_FLAG ( VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT )
        )
    },
    {
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,

        BufferSyncItem ( VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_ACCESS_INDEX_READ_BIT,
            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT
        )
    }
};

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void MeshGeometry::FreeResources ( Renderer &renderer ) noexcept
{
    FreeTransferResources ( renderer );
    FreeResourceInternal ( renderer );
    _fileName.clear ();
}

void MeshGeometry::FreeTransferResources ( Renderer &renderer ) noexcept
{
    if ( _transferBuffer != VK_NULL_HANDLE )
    {
        vkDestroyBuffer ( renderer.GetDevice (), _transferBuffer, nullptr );
        _transferBuffer = VK_NULL_HANDLE;
    }

    Allocation &allocation = _transferAllocation;

    if ( allocation._memory == VK_NULL_HANDLE )
        return;

    renderer.FreeMemory ( allocation._memory, allocation._offset );
    allocation._memory = VK_NULL_HANDLE;
    allocation._offset = std::numeric_limits<VkDeviceSize>::max ();
}

GXAABB const &MeshGeometry::GetBounds () const noexcept
{
    return _bounds;
}

VkBuffer const *MeshGeometry::GetVertexBuffers () const noexcept
{
    return _vertexBuffers;
}

BufferInfo MeshGeometry::GetBufferInfo () const noexcept
{
    return
    {
        ._buffer = _vertexBuffers[ MeshBufferInfo::POSITION_BUFFER_INDEX ],
        ._range = _vertexAllocations[ MeshBufferInfo::POSITION_BUFFER_INDEX ]._range,
    };
}

MeshBufferInfo MeshGeometry::GetMeshBufferInfo () const noexcept
{
    return
    {
        ._postions
        {
            ._buffer = _vertexBuffers[ MeshBufferInfo::POSITION_BUFFER_INDEX ],
            ._range = _vertexAllocations[ MeshBufferInfo::POSITION_BUFFER_INDEX ]._range
        },

        ._rest
        {
            ._buffer = _vertexBuffers[ MeshBufferInfo::REST_DATA_BUFFER_INDEX ],
            ._range = _vertexAllocations[ MeshBufferInfo::REST_DATA_BUFFER_INDEX ]._range
        }
    };
}

uint32_t MeshGeometry::GetVertexBufferVertexCount () const noexcept
{
    return _vertexBufferVertexCount;
}

MeshGeometry::IndexBuffer const &MeshGeometry::GetIndexBuffer () const noexcept
{
    return _indexBuffer;
}

std::string const &MeshGeometry::GetName () const noexcept
{
    return _fileName;
}

uint32_t MeshGeometry::GetVertexCount () const noexcept
{
    return _vertexCount;
}

bool MeshGeometry::IsUnique () const noexcept
{
    return _fileName.empty ();
}

void MeshGeometry::MakeUnique () noexcept
{
    _fileName.clear ();
}

bool MeshGeometry::LoadMesh ( Renderer &renderer,
    VkCommandBuffer commandBuffer,
    bool externalCommandBuffer,
    VkFence fence,
    std::string &&fileName
) noexcept
{
    if ( fileName.empty () ) [[unlikely]]
    {
        LogError ( "LoadMesh::LoadMesh - Can't upload data. Filename is empty." );
        return false;
    }

    static std::regex const isMesh2 ( R"__(^.+?\.mesh2$)__" );
    std::smatch match;

    if ( !std::regex_match ( fileName, match, isMesh2 ) )
        return false;

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
        .usage = VERTEX_BUFFER_USAGE | AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    VkBuffer &buffer = _vertexBuffers[ MeshBufferInfo::POSITION_BUFFER_INDEX ];
    Allocation &allocation = _vertexAllocations[ MeshBufferInfo::POSITION_BUFFER_INDEX ];

    if ( !CreateBuffer ( renderer, buffer, allocation, bufferInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Mesh" ) )
    {
        [[unlikely]]
        return false;
    }

    UploadJob const job
    {
        ._buffer = buffer,
        ._data = data.data (),
        ._size = allocation._range,
        ._usage = VERTEX_BUFFER_USAGE
    };

    bool const result = GPUTransfer ( renderer,
        commandBuffer,
        externalCommandBuffer,
        fence,
        { &job, 1U }
    );

    if ( !result ) [[unlikely]]
        return false;

    _vertexCount = vertexCount;
    _vertexBufferVertexCount = vertexCount;
    return true;
}

bool MeshGeometry::LoadMesh ( Renderer &renderer,
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
        static_cast<uint32_t> ( indexCount ),
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

bool MeshGeometry::LoadMesh ( Renderer &renderer,
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
        static_cast<uint32_t> ( indexCount ),
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

bool MeshGeometry::LoadMesh ( Renderer &renderer,
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
        static_cast<uint32_t> ( indexCount ),
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

bool MeshGeometry::LoadMesh ( Renderer &renderer,
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
        static_cast<uint32_t> ( indexCount ),
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

bool MeshGeometry::CreateBuffer ( Renderer &renderer,
    VkBuffer &buffer,
    Allocation &allocation,
    VkBufferCreateInfo const &createInfo,
    VkMemoryPropertyFlags memoryProperty,
    [[maybe_unused]] char const *name
) noexcept
{
    VkDevice device = renderer.GetDevice ();

    bool const result = Renderer::CheckVkResult ( vkCreateBuffer ( device, &createInfo, nullptr, &buffer ),
        "MeshGeometry::CreateBuffer",
        "Can't create buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    allocation._range = createInfo.size;
    AV_SET_VULKAN_OBJECT_NAME ( device, buffer, VK_OBJECT_TYPE_BUFFER, "%s", name )

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements ( device, buffer, &memoryRequirements );

    return
        renderer.TryAllocateMemory ( allocation._memory,
            allocation._offset,
            memoryRequirements,
            memoryProperty,
            "Can't allocate memory (MeshGeometry::CreateBuffer)"
        ) &&

        Renderer::CheckVkResult (
            vkBindBufferMemory ( device, buffer, allocation._memory, allocation._offset ),
            "MeshGeometry::CreateBuffer",
            "Can't bind memory"
        );
}

void MeshGeometry::FreeResourceInternal ( Renderer &renderer ) noexcept
{
    _vertexCount = 0U;
    VkDevice device = renderer.GetDevice ();

    if ( VkBuffer &positions = _vertexBuffers[ MeshBufferInfo::POSITION_BUFFER_INDEX ]; positions != VK_NULL_HANDLE )
    {
        [[likely]]
        vkDestroyBuffer ( device, positions, nullptr );
        positions = VK_NULL_HANDLE;
    }

    if ( VkBuffer &restData = _vertexBuffers[ MeshBufferInfo::REST_DATA_BUFFER_INDEX ]; restData != VK_NULL_HANDLE )
    {
        vkDestroyBuffer ( device, restData, nullptr );
        restData = VK_NULL_HANDLE;
    }

    if ( _indexBuffer._buffer != VK_NULL_HANDLE )
    {
        vkDestroyBuffer ( device, _indexBuffer._buffer, nullptr );
        _indexBuffer._buffer = VK_NULL_HANDLE;
    }

    if ( _indexAllocation._memory != VK_NULL_HANDLE )
    {
        renderer.FreeMemory ( _indexAllocation._memory, _indexAllocation._offset );
        _indexAllocation._memory = VK_NULL_HANDLE;
        _indexAllocation._offset = std::numeric_limits<VkDeviceSize>::max ();
    }

    if ( Allocation &mem = _vertexAllocations[ MeshBufferInfo::POSITION_BUFFER_INDEX ]; mem._memory != VK_NULL_HANDLE )
    {
        [[likely]]
        renderer.FreeMemory ( mem._memory, mem._offset );
        mem._memory = VK_NULL_HANDLE;
        mem._offset = std::numeric_limits<VkDeviceSize>::max ();
    }

    Allocation &rest = _vertexAllocations[ MeshBufferInfo::REST_DATA_BUFFER_INDEX ];

    if ( rest._memory == VK_NULL_HANDLE )
        return;

    renderer.FreeMemory ( rest._memory, rest._offset );
    rest._memory = VK_NULL_HANDLE;
    rest._offset = std::numeric_limits<VkDeviceSize>::max ();
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
    VkBufferMemoryBarrier barrierInfo[ 3U ];
    size_t const jobCount = jobs.size ();
    AV_ASSERT ( jobCount <= std::size ( barrierInfo ) )

    VkPipelineStageFlags srcStages = 0U;
    VkPipelineStageFlags dstStages = 0U;

    VkBufferCopy bufferCopy
    {
        .srcOffset = 0U,
        .dstOffset = 0U,
        .size = 0U
    };

    for ( size_t i = 0U; i < jobCount; ++i )
    {
        UploadJob const &job = jobs[ i ];
        auto const findResult = g_accessMapper.find ( job._usage );

        if ( findResult == g_accessMapper.cend () )
        {
            LogError ( "MeshGeometry::GPUTransfer - Unexpected usage 0x%08X", job._usage );
            return false;
        }

        BufferSyncItem const &syncItem = findResult->second;
        srcStages |= syncItem._srcStage;
        dstStages |= syncItem._dstStage;
        VkDeviceSize size = job._size;

        barrierInfo[ i ] =
        {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = syncItem._srcAccessMask,
            .dstAccessMask = syncItem._dstAccessMask,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = job._buffer,
            .offset = 0U,
            .size = size
        };

        bufferCopy.size = size;
        vkCmdCopyBuffer ( commandBuffer, _transferBuffer, job._buffer, 1U, &bufferCopy );
        bufferCopy.srcOffset += size;
    }

    vkCmdPipelineBarrier ( commandBuffer,
        srcStages,
        dstStages,
        0U,
        0U,
        nullptr,
        static_cast<uint32_t> ( jobCount ),
        barrierInfo,
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

        static_cast<uint32_t> ( header._indexCount ),
        cases[ selector ],

        {
            rawData + static_cast<size_t> ( header._positionDataOffset ),
            static_cast<size_t> ( header._vertexCount ) * sizeof ( VertexInfo )
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
    uint32_t indexCount,
    VkIndexType indexType,
    AbstractData vertexStream0,
    Vertices vertexStream1,
    uint32_t vertexCount
) noexcept
{
    constexpr VkBufferUsageFlags indexBufferUsageFlags = AV_VK_FLAG ( VK_BUFFER_USAGE_INDEX_BUFFER_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT );

    AV_ASSERT ( indexType == VK_INDEX_TYPE_UINT16 || indexType == VK_INDEX_TYPE_UINT32 )

    auto const indexBufferSize = static_cast<VkDeviceSize> (
        indexCount * INDEX_SIZES[ static_cast<size_t> ( indexType == VK_INDEX_TYPE_UINT32 ) ]
    );

    VkBufferCreateInfo bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = indexBufferSize,
        .usage = indexBufferUsageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    bool result = CreateBuffer ( renderer,
        _indexBuffer._buffer,
        _indexAllocation,
        bufferInfo,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Mesh indices"
    );

    if ( !result ) [[unlikely]]
        return false;

    _indexBuffer._type = indexType;

    constexpr VkBufferUsageFlags vertexBufferUsageFlags = AV_VK_FLAG ( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT );

    bufferInfo.size = static_cast<VkDeviceSize> ( vertexStream0.size () );
    bufferInfo.usage = vertexBufferUsageFlags;
    VkBuffer &positionBuffer = _vertexBuffers[ MeshBufferInfo::POSITION_BUFFER_INDEX ];
    Allocation &positionAllocation = _vertexAllocations[ MeshBufferInfo::POSITION_BUFFER_INDEX ];

    result = CreateBuffer ( renderer,
        positionBuffer,
        positionAllocation,
        bufferInfo,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Mesh positions"
    );

    if ( !result ) [[unlikely]]
        return false;

    constexpr VkBufferUsageFlags vertexDataUsage = AV_VK_FLAG ( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT );

    if ( vertexStream1.empty () )
    {
        UploadJob const jobs[]
        {
            {
                ._buffer = _indexBuffer._buffer,
                ._data = indices.data (),
                ._size = _indexAllocation._range,
                ._usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT
            },
            {
                ._buffer = positionBuffer,
                ._data = vertexStream0.data (),
                ._size = positionAllocation._range,
                ._usage = vertexDataUsage
            }
        };

        return GPUTransfer ( renderer, commandBuffer, externalCommandBuffer, fence, { jobs, std::size ( jobs ) } );
    }

    bufferInfo.size = static_cast<VkDeviceSize> ( vertexCount * sizeof ( VertexInfo ) );
    bufferInfo.usage = vertexBufferUsageFlags;
    VkBuffer &restDataBuffer = _vertexBuffers[ MeshBufferInfo::REST_DATA_BUFFER_INDEX ];
    Allocation &restDataAllocation = _vertexAllocations[ MeshBufferInfo::REST_DATA_BUFFER_INDEX ];

    result = CreateBuffer ( renderer,
        restDataBuffer,
        restDataAllocation,
        bufferInfo,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Mesh rest data"
    );

    if ( !result ) [[unlikely]]
        return false;

    UploadJob const jobs[]
    {
        {
            ._buffer = _indexBuffer._buffer,
            ._data = indices.data (),
            ._size = _indexAllocation._range,
            ._usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT
        },
        {
            ._buffer = positionBuffer,
            ._data = vertexStream0.data (),
            ._size = positionAllocation._range,
            ._usage = vertexDataUsage
        },
        {
            ._buffer = restDataBuffer,
            ._data = vertexStream1.data (),
            ._size = restDataAllocation._range,
            ._usage = vertexDataUsage
        }
    };

    return GPUTransfer ( renderer, commandBuffer, externalCommandBuffer, fence, { jobs, std::size ( jobs ) } );
}

} // namespace android_vulkan
