#include <mesh_geometry.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <array>
#include <regex>
#include <thread>

GX_RESTORE_WARNING_STATE

#include <file.hpp>
#include <logger.hpp>
#include <vertex_info.hpp>
#include <vulkan_utils.hpp>
#include <android_vulkan_sdk/mesh2.hpp>
#include <GXCommon/GXNativeMesh.hpp>


namespace android_vulkan {

namespace {

constexpr size_t UV_THREADS = 4U;

constexpr VkFlags VERTEX_BUFFER_USAGE = AV_VK_FLAG ( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ) |
    AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT );

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

    if ( _transferBufferMemory == VK_NULL_HANDLE )
        return;

    renderer.FreeMemory ( _transferBufferMemory, _transferBufferOffset );
    _transferBufferMemory = VK_NULL_HANDLE;
    _transferBufferOffset = std::numeric_limits<VkDeviceSize>::max ();
}

GXAABB const &MeshGeometry::GetBounds () const noexcept
{
    return _bounds;
}

VkBuffer const &MeshGeometry::GetVertexBuffer () const noexcept
{
    return _vertexBuffer;
}

BufferInfo MeshGeometry::GetVertexBufferInfo () const noexcept
{
    return
    {
        ._buffer = _vertexBuffer,
        ._range = _vertexBufferRange
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

bool MeshGeometry::LoadMesh ( std::string &&fileName,
    Renderer &renderer,
    VkCommandBuffer commandBuffer,
    VkFence fence
) noexcept
{
    if ( fileName.empty () ) [[unlikely]]
    {
        LogError ( "MeshGeometry::LoadMesh - Can't upload data. Filename is empty." );
        return false;
    }

    static std::regex const isMesh2 ( R"__(^.+?\.mesh2$)__" );
    std::smatch match;

    if ( std::regex_match ( fileName, match, isMesh2 ) )
        return LoadFromMesh2 ( std::move ( fileName ), renderer, commandBuffer, fence );

    static std::regex const isMesh ( R"__(^.+?\.mesh$)__" );

    if ( std::regex_match ( fileName, match, isMesh ) ) [[likely]]
        return LoadFromMesh ( std::move ( fileName ), renderer, commandBuffer, fence );

    return false;
}

[[maybe_unused]] bool MeshGeometry::LoadMesh ( uint8_t const* data,
    size_t size,
    uint32_t vertexCount,
    Renderer &renderer,
    VkCommandBuffer commandBuffer,
    VkFence fence
) noexcept
{
    FreeResources ( renderer );
    return UploadSimple ( data, size, vertexCount, VERTEX_BUFFER_USAGE, renderer, commandBuffer, fence );
}

bool MeshGeometry::LoadMesh ( uint8_t const* vertexData,
    size_t vertexDataSize,
    uint32_t vertexCount,
    uint32_t const* indices,
    uint32_t indexCount,
    GXAABB const &bounds,
    Renderer &renderer,
    VkCommandBuffer commandBuffer,
    VkFence fence
) noexcept
{
    FreeResources ( renderer );

    size_t const vertexOffset = sizeof ( uint32_t ) * indexCount;
    std::vector<uint8_t> storage ( vertexOffset + vertexDataSize );
    uint8_t* data = storage.data ();
    std::memcpy ( data, indices, vertexOffset );
    std::memcpy ( data + vertexOffset, vertexData, vertexDataSize );

    bool const result = UploadComplex ( data,
        vertexDataSize,
        indexCount,
        renderer,
        commandBuffer,
        fence
    );

    if ( result ) [[likely]]
    {
        _vertexCount = indexCount,
        _vertexBufferVertexCount = vertexCount;
        _bounds = bounds;
    }

    return result;
}

void MeshGeometry::FreeResourceInternal ( Renderer &renderer ) noexcept
{
    _vertexCount = 0U;
    VkDevice device = renderer.GetDevice ();

    if ( _vertexBuffer != VK_NULL_HANDLE )
    {
        vkDestroyBuffer ( device, _vertexBuffer, nullptr );
        _vertexBuffer = VK_NULL_HANDLE;
    }

    if ( _indexBuffer != VK_NULL_HANDLE )
    {
        vkDestroyBuffer ( device, _indexBuffer, nullptr );
        _indexBuffer = VK_NULL_HANDLE;
    }

    if ( _indexBufferMemory != VK_NULL_HANDLE )
    {
        renderer.FreeMemory ( _indexBufferMemory, _indexBufferOffset );
        _indexBufferMemory = VK_NULL_HANDLE;
        _indexBufferOffset = std::numeric_limits<VkDeviceSize>::max ();
    }

    if ( _vertexBufferMemory == VK_NULL_HANDLE )
        return;

    renderer.FreeMemory ( _vertexBufferMemory, _vertexBufferOffset );
    _vertexBufferMemory = VK_NULL_HANDLE;
    _vertexBufferOffset = std::numeric_limits<VkDeviceSize>::max ();
}

bool MeshGeometry::LoadFromMesh ( std::string &&fileName,
    Renderer &renderer,
    VkCommandBuffer commandBuffer,
    VkFence fence
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
    auto* vertices = reinterpret_cast<VertexInfo*> ( content.data () + header.vboOffset );

    auto converter = [ & ] ( VertexInfo* vertices,
        size_t currentIndex,
        size_t lastIndex,
        size_t toNextBatch,
        size_t verticesPerBatch
    ) {
        while ( currentIndex <= lastIndex )
        {
            size_t const limit = std::min ( lastIndex + 1U, currentIndex + verticesPerBatch );

            for ( ; currentIndex < limit; ++currentIndex )
            {
                GXVec2 &uv = vertices[ currentIndex ]._uv;
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

    bool const result = UploadSimple ( reinterpret_cast<uint8_t const*> ( vertices ),
        header.totalVertices * sizeof ( VertexInfo ),
        header.totalVertices,
        VERTEX_BUFFER_USAGE,
        renderer,
        commandBuffer,
        fence
    );

    if ( !result ) [[unlikely]]
        return false;

    _fileName = std::move ( fileName );
    return true;
}

bool MeshGeometry::LoadFromMesh2 ( std::string &&fileName,
    Renderer &renderer,
    VkCommandBuffer commandBuffer,
    VkFence fence
) noexcept
{
    FreeResourceInternal ( renderer );
    File file ( fileName );

    if ( !file.LoadContent () )
        return false;

    std::vector<uint8_t> const &content = file.GetContent ();
    uint8_t const* rawData = content.data ();
    auto const &header = *reinterpret_cast<Mesh2Header const*> ( rawData );

    bool const result = UploadComplex ( rawData + static_cast<size_t> ( header._indexDataOffset ),
        static_cast<size_t> ( header._vertexCount ) * sizeof ( Mesh2Vertex ),
        static_cast<uint32_t> ( header._indexCount ),
        renderer,
        commandBuffer,
        fence
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

bool MeshGeometry::UploadComplex ( uint8_t const* data,
    size_t vertexDataSize,
    uint32_t indexCount,
    Renderer &renderer,
    VkCommandBuffer commandBuffer,
    VkFence fence
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

    result = renderer.TryAllocateMemory ( _indexBufferMemory,
        _indexBufferOffset,
        memoryRequirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate index buffer memory (MeshGeometry::UploadComplex)"
    );

    if ( !result ) [[unlikely]]
        return false;

    result = Renderer::CheckVkResult (
        vkBindBufferMemory ( device, _indexBuffer, _indexBufferMemory, _indexBufferOffset ),
        "MeshGeometry::UploadComplex",
        "Can't bind index buffer memory"
    );

    if ( !result ) [[unlikely]]
        return false;

    constexpr VkBufferUsageFlags vertexBufferUsageFlags = AV_VK_FLAG ( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT );

    bufferInfo.size = static_cast<VkDeviceSize> ( vertexDataSize );
    bufferInfo.usage = vertexBufferUsageFlags;

    result = Renderer::CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &_vertexBuffer ),
        "MeshGeometry::UploadComplex",
        "Can't create vertex buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    _vertexBufferRange = bufferInfo.size;
    AV_SET_VULKAN_OBJECT_NAME ( device, _vertexBuffer, VK_OBJECT_TYPE_BUFFER, "Mesh vertices" )

    vkGetBufferMemoryRequirements ( device, _vertexBuffer, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _vertexBufferMemory,
        _vertexBufferOffset,
        memoryRequirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate vertex buffer memory (MeshGeometry::UploadComplex)"
    );

    if ( !result ) [[unlikely]]
        return false;

    result = Renderer::CheckVkResult (
        vkBindBufferMemory ( device, _vertexBuffer, _vertexBufferMemory, _vertexBufferOffset ),
        "MeshGeometry::UploadComplex",
        "Can't bind vertex buffer memory"
    );

    if ( !result ) [[unlikely]]
        return false;

    VkBufferCopy const copyInfo[] =
    {
        {
            .srcOffset = 0U,
            .dstOffset = 0U,
            .size = indexBufferSize
        },
        {
            .srcOffset = indexBufferSize,
            .dstOffset = 0U,
            .size = static_cast<VkDeviceSize> ( vertexDataSize )
        }
    };

    constexpr VkBufferUsageFlags const usages[ std::size ( copyInfo ) ] =
    {
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        AV_VK_FLAG ( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ) | AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT )
    };

    VkBuffer const dstBuffers[ std::size ( copyInfo ) ] = { _indexBuffer, _vertexBuffer };

    return UploadInternal ( std::size ( usages ),
        copyInfo,
        usages,
        dstBuffers,
        data,
        static_cast<size_t> ( indexBufferSize + bufferInfo.size ),
        renderer,
        commandBuffer,
        fence
    );
}

bool MeshGeometry::UploadInternal ( size_t numUploads,
    VkBufferCopy const* copyJobs,
    VkBufferUsageFlags const* usages,
    VkBuffer const* dstBuffers,
    uint8_t const* data,
    size_t dataSize,
    Renderer &renderer,
    VkCommandBuffer commandBuffer,
    VkFence fence
) noexcept
{
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

    result = renderer.TryAllocateMemory ( _transferBufferMemory,
        _transferBufferOffset,
        transferMemoryRequirements,
        flags,
        "Can't allocate transfer memory (MeshGeometry::UploadInternal)"
    );

    if ( !result ) [[unlikely]]
        return false;

    result = Renderer::CheckVkResult (
        vkBindBufferMemory ( device, _transferBuffer, _transferBufferMemory, _transferBufferOffset ),
        "MeshGeometry::UploadInternal",
        "Can't bind transfer memory"
    );

    if ( !result ) [[unlikely]]
        return false;

    void* transferData = nullptr;

    result = renderer.MapMemory ( transferData,
        _transferBufferMemory,
        _transferBufferOffset,
        "MeshGeometry::UploadInternal",
        "Can't map data"
    );

    if ( !result ) [[unlikely]]
        return false;

    std::memcpy ( transferData, data, dataSize );
    renderer.UnmapMemory ( _transferBufferMemory );

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
        return false;

    // Note most extreme case is 2 upload jobs (vertex buffer and index buffer).
    VkBufferMemoryBarrier barrierInfo[ 2U ];
    VkPipelineStageFlags srcStages = 0U;
    VkPipelineStageFlags dstStages = 0U;

    for ( size_t i = 0U; i < numUploads; ++i )
    {
        VkBufferCopy const &copyBuffer = copyJobs[ i ];
        auto const findResult = g_accessMapper.find ( usages[ i ] );

        if ( findResult == g_accessMapper.cend () )
        {
            LogError ( "MeshGeometry::UploadInternal - Unexpected usage 0x%08X", usages[ i ] );
            return false;
        }

        BufferSyncItem const &syncItem = findResult->second;
        srcStages |= syncItem._srcStage;
        dstStages |= syncItem._dstStage;

        VkBuffer buffer = dstBuffers[ i ];

        barrierInfo[ i ] =
        {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = syncItem._srcAccessMask,
            .dstAccessMask = syncItem._dstAccessMask,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = buffer,
            .offset = 0U,
            .size = copyBuffer.size
        };

        vkCmdCopyBuffer ( commandBuffer, _transferBuffer, buffer, 1U, &copyBuffer );
    }

    vkCmdPipelineBarrier ( commandBuffer,
        srcStages,
        dstStages,
        0U,
        0U,
        nullptr,
        static_cast<uint32_t> ( numUploads ),
        barrierInfo,
        0U,
        nullptr
    );

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

bool MeshGeometry::UploadSimple ( uint8_t const* data,
    size_t size,
    uint32_t vertexCount,
    VkBufferUsageFlags usage,
    Renderer &renderer,
    VkCommandBuffer commandBuffer,
    VkFence fence
) noexcept
{
    VkBufferCreateInfo const bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = size,
        .usage = usage | AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    VkDevice device = renderer.GetDevice ();

    bool result = Renderer::CheckVkResult (
        vkCreateBuffer ( device, &bufferInfo, nullptr, &_vertexBuffer ),
        "MeshGeometry::UploadSimple",
        "Can't create buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    _vertexBufferRange = bufferInfo.size;
    AV_SET_VULKAN_OBJECT_NAME ( device, _vertexBuffer, VK_OBJECT_TYPE_BUFFER, "Mesh vertices" )

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements ( device, _vertexBuffer, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _vertexBufferMemory,
        _vertexBufferOffset,
        memoryRequirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate buffer memory (MeshGeometry::LoadMeshInternal)"
    );

    if ( !result ) [[unlikely]]
        return false;

    result = Renderer::CheckVkResult (
        vkBindBufferMemory ( device, _vertexBuffer, _vertexBufferMemory, _vertexBufferOffset ),
        "MeshGeometry::UploadSimple",
        "Can't bind buffer memory"
    );

    if ( !result ) [[unlikely]]
        return false;

    VkBufferCopy const copyInfo
    {
        .srcOffset = 0U,
        .dstOffset = 0U,
        .size = bufferInfo.size
    };

    result = UploadInternal ( 1U,
        &copyInfo,
        &usage,
        &_vertexBuffer,
        data,
        size,
        renderer,
        commandBuffer,
        fence
    );

    if ( !result ) [[unlikely]]
        return false;

    _vertexCount = vertexCount;
    _vertexBufferVertexCount = vertexCount;
    return true;
}

} // namespace android_vulkan
