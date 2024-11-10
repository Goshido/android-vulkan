#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <mesh_geometry.hpp>
#include <file.hpp>
#include <logger.hpp>
#include <vulkan_utils.hpp>
#include <android_vulkan_sdk/mesh2.hpp>
#include <GXCommon/GXNativeMesh.hpp>


namespace android_vulkan {

namespace {

constexpr size_t UV_THREADS = 4U;

constexpr VkFlags VERTEX_BUFFER_USAGE = AV_VK_FLAG ( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ) |
    AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT );

constexpr size_t POSITION_BUFFER_INDEX = 0U;
constexpr size_t REST_DATA_BUFFER_INDEX = 1U;

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

#pragma pack ( push, 1 )

struct OldFormat final
{
    PositionInfo            _position {};
    VertexInfo              _vertex {};
};

#pragma pack ( pop )

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

BufferInfo MeshGeometry::GetVertexBufferInfo () const noexcept
{
    return
    {
        ._buffer = _vertexBuffers[ POSITION_BUFFER_INDEX ],
        ._range = _vertexAllocations[ POSITION_BUFFER_INDEX ]._range
    };
}

uint32_t MeshGeometry::GetVertexBufferVertexCount () const noexcept
{
    return _vertexBufferVertexCount;
}

VkBuffer const &MeshGeometry::GetIndexBuffer () const noexcept
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

[[maybe_unused]] bool MeshGeometry::IsIndexBufferPresent () const noexcept
{
    return _indexBuffer != VK_NULL_HANDLE;
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

    if ( std::regex_match ( fileName, match, isMesh2 ) )
        return LoadFromMesh2 ( renderer, commandBuffer, externalCommandBuffer, fence, std::move ( fileName ) );

    static std::regex const isMesh ( R"__(^.+?\.mesh$)__" );

    if ( std::regex_match ( fileName, match, isMesh ) ) [[likely]]
        return LoadFromMesh ( renderer, commandBuffer, externalCommandBuffer, fence, std::move ( fileName ) );

    return false;
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

    return UploadSimple ( renderer,
        commandBuffer,
        externalCommandBuffer,
        fence,
        data,
        vertexCount,
        VERTEX_BUFFER_USAGE
    );
}

bool MeshGeometry::LoadMesh ( Renderer &renderer,
    VkCommandBuffer commandBuffer,
    bool externalCommandBuffer,
    VkFence fence,
    Indices indices,
    Positions positions,
    GXAABB const &bounds
) noexcept
{
    FreeResources ( renderer );

    size_t const indexCount = indices.size ();
    size_t const positionOffset = indexCount * sizeof ( uint32_t );
    size_t const positionCount = positions.size ();
    size_t const positionSize = positionCount * sizeof ( PositionInfo );

    size_t const size = positionOffset + positionSize;
    std::vector<uint8_t> storage ( size );
    uint8_t* data = storage.data ();
    std::memcpy ( data, indices.data (), positionOffset );
    std::memcpy ( data + positionOffset, positions.data (), positionSize );

    bool const result = UploadComplex ( renderer,
        commandBuffer,
        externalCommandBuffer,
        fence,
        { data, size },
        static_cast<uint32_t> ( indexCount ),
        static_cast<uint32_t> ( positionCount ),
        false,
        false
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
    Indices indices,
    Positions positions,
    Vertices vertices,
    GXAABB const &bounds
) noexcept
{
    FreeResources ( renderer );

    size_t const indexCount = indices.size ();
    size_t const positionOffset = indexCount * sizeof ( uint32_t );
    size_t const positionSize = positions.size () * sizeof ( PositionInfo );
    size_t const vertexOffset = positionOffset + positionSize;
    size_t const vertexCount = vertices.size ();
    size_t const vertexSize = vertexCount * sizeof ( VertexInfo );

    size_t const size = vertexOffset + vertexSize;
    std::vector<uint8_t> storage ( size );
    uint8_t* data = storage.data ();
    std::memcpy ( data, indices.data (), positionOffset );
    std::memcpy ( data + positionOffset, positions.data (), positionSize );
    std::memcpy ( data + vertexOffset, vertices.data (), vertexSize );

    bool const result = UploadComplex ( renderer,
        commandBuffer,
        externalCommandBuffer,
        fence,
        { data, size },
        static_cast<uint32_t> ( indexCount ),
        static_cast<uint32_t> ( vertexCount ),
        true,
        false
    );

    if ( !result ) [[unlikely]]
        return false;

    _vertexCount = static_cast<uint32_t> ( indexCount );
    _vertexBufferVertexCount = static_cast<uint32_t> ( vertexCount );
    _bounds = bounds;
    return true;
}

void MeshGeometry::FreeResourceInternal ( Renderer &renderer ) noexcept
{
    _vertexCount = 0U;
    VkDevice device = renderer.GetDevice ();

    if ( VkBuffer &positionData = _vertexBuffers[ POSITION_BUFFER_INDEX ]; positionData != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyBuffer ( device, positionData, nullptr );
        positionData = VK_NULL_HANDLE;
    }

    if ( VkBuffer &restData = _vertexBuffers[ REST_DATA_BUFFER_INDEX ]; restData != VK_NULL_HANDLE )
    {
        vkDestroyBuffer ( device, restData, nullptr );
        restData = VK_NULL_HANDLE;
    }

    if ( _indexBuffer != VK_NULL_HANDLE )
    {
        vkDestroyBuffer ( device, _indexBuffer, nullptr );
        _indexBuffer = VK_NULL_HANDLE;
    }

    if ( _indexAllocation._memory != VK_NULL_HANDLE )
    {
        renderer.FreeMemory ( _indexAllocation._memory, _indexAllocation._offset );
        _indexAllocation._memory = VK_NULL_HANDLE;
        _indexAllocation._offset = std::numeric_limits<VkDeviceSize>::max ();
    }

    if ( Allocation &position = _vertexAllocations[ POSITION_BUFFER_INDEX ]; position._memory != VK_NULL_HANDLE )
    {
        [[likely]]
        renderer.FreeMemory ( position._memory, position._offset );
        position._memory = VK_NULL_HANDLE;
        position._offset = std::numeric_limits<VkDeviceSize>::max ();
    }

    Allocation &rest = _vertexAllocations[ REST_DATA_BUFFER_INDEX ];

    if ( rest._memory == VK_NULL_HANDLE )
        return;

    renderer.FreeMemory ( rest._memory, rest._offset );
    rest._memory = VK_NULL_HANDLE;
    rest._offset = std::numeric_limits<VkDeviceSize>::max ();
}

bool MeshGeometry::LoadFromMesh ( Renderer &renderer,
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

    std::vector<uint8_t> &content = file.GetContent ();
    auto const &header = *reinterpret_cast<GXNativeMeshHeader const*> ( content.data () );

    constexpr size_t skipFactor = UV_THREADS - 1U;

    auto const verticesPerBatch = std::max ( static_cast<size_t const> ( 1U ),
        static_cast<size_t const> ( header.totalVertices ) / UV_THREADS
    );

    size_t const toNextBatch = verticesPerBatch * skipFactor;
    auto const lastIndex = static_cast<size_t const> ( header.totalVertices - 1U );
    uint8_t* vertexData = content.data () + header.vboOffset;
    auto* vertices = reinterpret_cast<OldFormat*> ( vertexData );

    auto const converter = [ & ] ( OldFormat* vertices,
        size_t currentIndex,
        size_t lastIndex,
        size_t toNextBatch,
        size_t verticesPerBatch
    ) noexcept {
        while ( currentIndex <= lastIndex )
        {
            size_t const limit = std::min ( lastIndex + 1U, currentIndex + verticesPerBatch );

            for ( ; currentIndex < limit; ++currentIndex )
            {
                GXVec2 &uv = vertices[ currentIndex ]._vertex._uv;
                uv._data[ 1U ] = 1.0F - uv._data[ 1U ];
            }

            currentIndex += toNextBatch;
        }
    };

    std::array<std::thread, UV_THREADS> converters;

    for ( size_t i = 0U; i < UV_THREADS; ++i )
    {
        converters[ i ] = std::thread ( converter,
            vertices,
            i * verticesPerBatch,
            lastIndex,
            toNextBatch,
            verticesPerBatch
        );
    }

    for ( auto &item : converters )
        item.join ();

    bool const result = UploadSimple ( renderer,
        commandBuffer,
        externalCommandBuffer,
        fence,
        { vertexData, header.totalVertices * sizeof ( OldFormat ) },
        header.totalVertices,
        VERTEX_BUFFER_USAGE
    );

    if ( !result ) [[unlikely]]
        return false;

    _fileName = std::move ( fileName );
    return true;
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

    if ( !file.LoadContent () )
        return false;

    std::vector<uint8_t> const &content = file.GetContent ();
    uint8_t const* rawData = content.data ();
    auto const &header = *reinterpret_cast<Mesh2Header const*> ( rawData );

    bool const result = UploadComplex ( renderer,
        commandBuffer,
        externalCommandBuffer,
        fence,

        {
            rawData + static_cast<size_t> ( header._indexDataOffset ),
            header._indexCount * sizeof ( uint32_t ) + header._vertexCount * sizeof ( OldFormat )
        },

        static_cast<uint32_t> ( header._indexCount ),
        static_cast<uint32_t> ( header._vertexCount ),
        true,
        true
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

bool MeshGeometry::UploadComplex ( Renderer &renderer,
    VkCommandBuffer commandBuffer,
    bool externalCommandBuffer,
    VkFence fence,
    AbstractData data,
    uint32_t indexCount,
    uint32_t vertexCount,
    bool hasRestData,
    bool needRepack
) noexcept
{
    constexpr VkBufferUsageFlags indexBufferUsageFlags = AV_VK_FLAG ( VK_BUFFER_USAGE_INDEX_BUFFER_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT );

    auto const indexBufferSize = static_cast<VkDeviceSize> ( indexCount * sizeof ( uint32_t ) );

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

    VkDevice device = renderer.GetDevice ();

    bool result = Renderer::CheckVkResult (
        vkCreateBuffer ( device, &bufferInfo, nullptr, &_indexBuffer ),
        "MeshGeometry::UploadComplex",
        "Can't create index buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _indexBuffer, VK_OBJECT_TYPE_BUFFER, "Mesh indices" )

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements ( device, _indexBuffer, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _indexAllocation._memory,
        _indexAllocation._offset,
        memoryRequirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate index buffer memory (MeshGeometry::UploadComplex)"
    );

    if ( !result ) [[unlikely]]
        return false;

    result = Renderer::CheckVkResult (
        vkBindBufferMemory ( device, _indexBuffer, _indexAllocation._memory, _indexAllocation._offset ),
        "MeshGeometry::UploadComplex",
        "Can't bind index buffer memory"
    );

    if ( !result ) [[unlikely]]
        return false;

    constexpr VkBufferUsageFlags vertexBufferUsageFlags = AV_VK_FLAG ( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT );

    bufferInfo.size = static_cast<VkDeviceSize> ( vertexCount * sizeof ( PositionInfo ) );
    bufferInfo.usage = vertexBufferUsageFlags;
    VkBuffer &positionBuffer = _vertexBuffers[ POSITION_BUFFER_INDEX ];
    Allocation &positionAllocation = _vertexAllocations[ POSITION_BUFFER_INDEX ];

    result = Renderer::CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &positionBuffer ),
        "MeshGeometry::UploadComplex",
        "Can't create position buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    positionAllocation._range = bufferInfo.size;
    AV_SET_VULKAN_OBJECT_NAME ( device, positionBuffer, VK_OBJECT_TYPE_BUFFER, "Mesh positions" )

    vkGetBufferMemoryRequirements ( device, positionBuffer, &memoryRequirements );

    result = renderer.TryAllocateMemory ( positionAllocation._memory,
        positionAllocation._offset,
        memoryRequirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate position buffer memory (MeshGeometry::UploadComplex)"
    );

    if ( !result ) [[unlikely]]
        return false;

    result = Renderer::CheckVkResult (
        vkBindBufferMemory ( device, positionBuffer, positionAllocation._memory, positionAllocation._offset ),
        "MeshGeometry::UploadComplex",
        "Can't bind position buffer memory"
    );

    if ( !result ) [[unlikely]]
        return false;

    constexpr VkBufferUsageFlags vertexDataUsage = AV_VK_FLAG ( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT );

    if ( !hasRestData )
    {
        UploadJob const jobs[]
        {
            {
                ._copyInfo
                {
                    .srcOffset = 0U,
                    .dstOffset = 0U,
                    .size = indexBufferSize
                },

                ._buffer = _indexBuffer,
                ._usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT
            },
            {
                ._copyInfo
                {
                    .srcOffset = indexBufferSize,
                    .dstOffset = 0U,
                    .size = positionAllocation._range
                },

                ._buffer = positionBuffer,
                ._usage = vertexDataUsage
            }
        };

        return UploadInternal ( renderer,
            commandBuffer,
            externalCommandBuffer,
            fence,
            { jobs, std::size ( jobs ) },
            data
        );
    }

    bufferInfo.size = static_cast<VkDeviceSize> ( vertexCount * sizeof ( VertexInfo ) );
    bufferInfo.usage = vertexBufferUsageFlags;
    VkBuffer &restDataBuffer = _vertexBuffers[ REST_DATA_BUFFER_INDEX ];
    Allocation &restDataAllocation = _vertexAllocations[ REST_DATA_BUFFER_INDEX ];

    result = Renderer::CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &restDataBuffer ),
        "MeshGeometry::UploadComplex",
        "Can't create 'rest data' buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    restDataAllocation._range = bufferInfo.size;
    AV_SET_VULKAN_OBJECT_NAME ( device, restDataBuffer, VK_OBJECT_TYPE_BUFFER, "Mesh rest data" )

    vkGetBufferMemoryRequirements ( device, restDataBuffer, &memoryRequirements );

    result = renderer.TryAllocateMemory ( restDataAllocation._memory,
        restDataAllocation._offset,
        memoryRequirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate 'rest data' buffer memory (MeshGeometry::UploadComplex)"
    );

    if ( !result ) [[unlikely]]
        return false;

    result = Renderer::CheckVkResult (
        vkBindBufferMemory ( device, restDataBuffer, restDataAllocation._memory, restDataAllocation._offset ),
        "MeshGeometry::UploadComplex",
        "Can't bind 'rest data' buffer memory"
    );

    if ( !result ) [[unlikely]]
        return false;

    size_t const totalSize = indexBufferSize + positionAllocation._range + restDataAllocation._range;
    uint8_t const* dataSrc = data.data ();
    std::vector<uint8_t> repackData {};

    if ( needRepack )
    {
        repackData.resize ( totalSize );
        uint8_t* repackPtr = repackData.data ();
        dataSrc = data.data ();
        std::memcpy ( repackPtr, dataSrc, indexBufferSize );

        auto const* src = reinterpret_cast<OldFormat const*> ( dataSrc + indexBufferSize );

        uint8_t *vertexData = repackPtr + indexBufferSize;
        auto* positionDst = reinterpret_cast<PositionInfo*> ( vertexData );
        auto* restDst = reinterpret_cast<VertexInfo*> ( vertexData + positionAllocation._range );

        for ( size_t i = 0U; i < vertexCount; ++i )
        {
            OldFormat const &input = src[ i ];
            positionDst[ i ]._position = input._position._position;
            restDst[ i ] = input._vertex;
        }

        dataSrc = repackPtr;
    }

    UploadJob const jobs[]
    {
        {
            ._copyInfo
            {
                .srcOffset = 0U,
                .dstOffset = 0U,
                .size = indexBufferSize
            },

            ._buffer = _indexBuffer,
            ._usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT
        },
        {
            ._copyInfo
            {
                .srcOffset = indexBufferSize,
                .dstOffset = 0U,
                .size = positionAllocation._range
            },

            ._buffer = positionBuffer,
            ._usage = vertexDataUsage
        },
        {
            ._copyInfo
            {
                .srcOffset = indexBufferSize + positionAllocation._range,
                .dstOffset = 0U,
                .size = restDataAllocation._range
            },

            ._buffer = restDataBuffer,
            ._usage = vertexDataUsage
        }
    };

    return UploadInternal ( renderer,
        commandBuffer,
        externalCommandBuffer,
        fence,
        { jobs, std::size ( jobs ) },
        { dataSrc, totalSize }
    );
}

bool MeshGeometry::UploadInternal ( Renderer &renderer,
    VkCommandBuffer commandBuffer,
    bool externalCommandBuffer,
    VkFence fence,
    UploadJobs jobs,
    AbstractData data
) noexcept
{
    size_t const dataSize = data.size ();

    VkBufferCreateInfo const bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = static_cast<VkDeviceSize> ( dataSize ),
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    VkDevice device = renderer.GetDevice ();

    bool result = Renderer::CheckVkResult (
        vkCreateBuffer ( device, &bufferInfo, nullptr, &_transferBuffer ),
        "MeshGeometry::UploadInternal",
        "Can't create transfer buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _transferBuffer, VK_OBJECT_TYPE_BUFFER, "Mesh staging buffer" )

    VkMemoryRequirements transferMemoryRequirements;
    vkGetBufferMemoryRequirements ( device, _transferBuffer, &transferMemoryRequirements );

    constexpr VkMemoryPropertyFlags flags = AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ) |
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );

    result = renderer.TryAllocateMemory ( _transferAllocation._memory,
        _transferAllocation._offset,
        transferMemoryRequirements,
        flags,
        "Can't allocate transfer memory (MeshGeometry::UploadInternal)"
    );

    if ( !result ) [[unlikely]]
        return false;

    result = Renderer::CheckVkResult (
        vkBindBufferMemory ( device, _transferBuffer, _transferAllocation._memory, _transferAllocation._offset ),
        "MeshGeometry::UploadInternal",
        "Can't bind transfer memory"
    );

    if ( !result ) [[unlikely]]
        return false;

    void* transferData = nullptr;

    result = renderer.MapMemory ( transferData,
        _transferAllocation._memory,
        _transferAllocation._offset,
        "MeshGeometry::UploadInternal",
        "Can't map data"
    );

    if ( !result ) [[unlikely]]
        return false;

    std::memcpy ( transferData, data.data (), dataSize );
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
            "MeshGeometry::UploadInternal",
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

    for ( size_t i = 0U; i < jobCount; ++i )
    {
        UploadJob const &job = jobs[ i ];
        VkBufferCopy const &copyBuffer = job._copyInfo;
        auto const findResult = g_accessMapper.find ( job._usage );

        if ( findResult == g_accessMapper.cend () )
        {
            LogError ( "MeshGeometry::UploadInternal - Unexpected usage 0x%08X", job._usage );
            return false;
        }

        BufferSyncItem const &syncItem = findResult->second;
        srcStages |= syncItem._srcStage;
        dstStages |= syncItem._dstStage;

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
            .size = copyBuffer.size
        };

        vkCmdCopyBuffer ( commandBuffer, _transferBuffer, job._buffer, 1U, &copyBuffer );
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
        "MeshGeometry::UploadInternal",
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
        "MeshGeometry::UploadInternal",
        "Can't submit command"
    );
}

bool MeshGeometry::UploadSimple ( Renderer &renderer,
    VkCommandBuffer commandBuffer,
    bool externalCommandBuffer,
    VkFence fence,
    AbstractData data,
    uint32_t vertexCount,
    VkBufferUsageFlags usage
) noexcept
{
    VkBufferCreateInfo bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size =  static_cast<size_t> ( vertexCount ) * sizeof ( PositionInfo ),
        .usage = usage | AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    VkDevice device = renderer.GetDevice ();
    VkBuffer &positionBuffer = _vertexBuffers[ POSITION_BUFFER_INDEX ];
    Allocation &positionAllocation = _vertexAllocations[ POSITION_BUFFER_INDEX ];

    bool result = Renderer::CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &positionBuffer ),
        "MeshGeometry::UploadSimple",
        "Can't create position buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    positionAllocation._range = bufferInfo.size;
    AV_SET_VULKAN_OBJECT_NAME ( device, positionBuffer, VK_OBJECT_TYPE_BUFFER, "Mesh positions" )

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements ( device, positionBuffer, &memoryRequirements );

    result = renderer.TryAllocateMemory ( positionAllocation._memory,
        positionAllocation._offset,
        memoryRequirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate position buffer memory (MeshGeometry::UploadSimple)"
    );

    if ( !result ) [[unlikely]]
        return false;

    result = Renderer::CheckVkResult (
        vkBindBufferMemory ( device, positionBuffer, positionAllocation._memory, positionAllocation._offset ),
        "MeshGeometry::UploadSimple",
        "Can't bind position buffer memory"
    );

    if ( !result ) [[unlikely]]
        return false;

    bufferInfo.size = static_cast<VkDeviceSize> ( vertexCount * sizeof ( VertexInfo ) );
    VkBuffer &restDataBuffer = _vertexBuffers[ REST_DATA_BUFFER_INDEX ];
    Allocation &restDataAllocation = _vertexAllocations[ REST_DATA_BUFFER_INDEX ];

    result = Renderer::CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &restDataBuffer ),
        "MeshGeometry::UploadSimple",
        "Can't create 'rest data' buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    restDataAllocation._range = bufferInfo.size;
    AV_SET_VULKAN_OBJECT_NAME ( device, restDataBuffer, VK_OBJECT_TYPE_BUFFER, "Mesh rest data" )

    vkGetBufferMemoryRequirements ( device, restDataBuffer, &memoryRequirements );

    result = renderer.TryAllocateMemory ( restDataAllocation._memory,
        restDataAllocation._offset,
        memoryRequirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate 'rest data' buffer memory (MeshGeometry::UploadSimple)"
    );

    if ( !result ) [[unlikely]]
        return false;

    result = Renderer::CheckVkResult (
        vkBindBufferMemory ( device, restDataBuffer, restDataAllocation._memory, restDataAllocation._offset ),
        "MeshGeometry::UploadSimple",
        "Can't bind 'rest data' buffer memory"
    );

    if ( !result ) [[unlikely]]
        return false;

    std::vector<uint8_t> repackData {};
    size_t const totalSize = positionAllocation._range + restDataAllocation._range;
    repackData.resize ( totalSize );
    uint8_t *repackPtr = repackData.data ();

    auto const* src = reinterpret_cast<OldFormat const*> ( data.data () );

    auto* positionDst = reinterpret_cast<PositionInfo*> ( repackPtr );
    auto* restDst = reinterpret_cast<VertexInfo*> ( repackPtr + positionAllocation._range );

    for ( size_t i = 0U; i < vertexCount; ++i )
    {
        OldFormat const &input = src[ i ];
        positionDst[ i ]._position = input._position._position;
        restDst[ i ] = input._vertex;
    }

    UploadJob const jobs[]
    {
        {
            ._copyInfo
            {
                .srcOffset = 0U,
                .dstOffset = 0U,
                .size = positionAllocation._range
            },

            ._buffer = positionBuffer,
            ._usage = usage
        },
        {
            ._copyInfo
            {
                .srcOffset = positionAllocation._range,
                .dstOffset = 0U,
                .size = restDataAllocation._range
            },

            ._buffer = restDataBuffer,
            ._usage = usage
        }
    };

    result = UploadInternal ( renderer,
        commandBuffer,
        externalCommandBuffer,
        fence,
        { jobs, std::size ( jobs ) },
        { repackPtr, totalSize }
    );

    if ( !result ) [[unlikely]]
        return false;

    _vertexCount = vertexCount;
    _vertexBufferVertexCount = vertexCount;
    return true;
}

} // namespace android_vulkan
