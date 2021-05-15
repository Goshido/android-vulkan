#include <mesh_geometry.h>

GX_DISABLE_COMMON_WARNINGS

#include <array>
#include <cassert>
#include <regex>
#include <thread>

GX_RESTORE_WARNING_STATE

#include <file.h>
#include <logger.h>
#include <mesh2.h>
#include <vertex_info.h>
#include <vulkan_utils.h>
#include <GXCommon/GXNativeMesh.h>


namespace android_vulkan {

constexpr static size_t const UV_THREADS = 4U;

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
    ):
        _dstAccessMask ( dstAccessMask ),
        _dstStage ( dstStage ),
        _srcAccessMask ( srcAccessMask ),
        _srcStage ( srcStage )
    {
        // NOTHING
    }
};

static std::map<VkBufferUsageFlags, BufferSyncItem> const g_accessMapper =
{
    {
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,

        BufferSyncItem ( VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT
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

//----------------------------------------------------------------------------------------------------------------------

MeshGeometry::MeshGeometry ():
    _bounds {},
    _indexBuffer ( VK_NULL_HANDLE ),
    _vertexBuffer ( VK_NULL_HANDLE ),
    _bufferMemory ( VK_NULL_HANDLE ),
    _transferBuffer ( VK_NULL_HANDLE ),
    _transferMemory ( VK_NULL_HANDLE ),
    _vertexCount ( 0U )
{
    // NOTHING
}

void MeshGeometry::FreeResources ( VkDevice device )
{
    FreeTransferResources ( device );
    FreeResourceInternal ( device );
    _fileName.clear ();
}

void MeshGeometry::FreeTransferResources ( VkDevice device )
{
    if ( _transferMemory != VK_NULL_HANDLE )
    {
        vkFreeMemory ( device, _transferMemory, nullptr );
        _transferMemory = VK_NULL_HANDLE;
        AV_UNREGISTER_DEVICE_MEMORY ( "MeshGeometry::_transferMemory" )
    }

    if ( _transferBuffer == VK_NULL_HANDLE )
        return;

    vkDestroyBuffer ( device, _transferBuffer, nullptr );
    _transferBuffer = VK_NULL_HANDLE;
    AV_UNREGISTER_BUFFER ( "MeshGeometry::_transferBuffer" )
}

GXAABB const& MeshGeometry::GetBounds () const
{
    return _bounds;
}

VkBuffer const& MeshGeometry::GetVertexBuffer () const
{
    return _vertexBuffer;
}

VkBuffer const& MeshGeometry::GetIndexBuffer () const
{
    return _indexBuffer;
}

std::string const& MeshGeometry::GetName () const
{
    return _fileName;
}

uint32_t MeshGeometry::GetVertexCount () const
{
    return _vertexCount;
}

[[maybe_unused]] bool MeshGeometry::IsIndexBufferPresent () const
{
    return _indexBuffer != VK_NULL_HANDLE;
}

bool MeshGeometry::IsUnique () const
{
    return _fileName.empty ();
}

bool MeshGeometry::LoadMesh ( std::string &&fileName,
    VkBufferUsageFlags usage,
    Renderer &renderer,
    VkCommandBuffer commandBuffer
)
{
    if ( fileName.empty () )
    {
        LogError ( "MeshGeometry::LoadMesh - Can't upload data. Filename is empty." );
        return false;
    }

    static std::regex const isMesh2 ( R"__(^.+?\.mesh2$)__" );
    std::smatch match;

    if ( std::regex_match ( fileName, match, isMesh2 ) )
        return LoadFromMesh2 ( std::move ( fileName ), renderer, commandBuffer );

    static std::regex const isMesh ( R"__(^.+?\.mesh$)__" );

    if ( std::regex_match ( fileName, match, isMesh ) )
        return LoadFromMesh ( std::move ( fileName ), usage, renderer, commandBuffer );

    return false;
}

[[maybe_unused]] bool MeshGeometry::LoadMesh ( uint8_t const* data,
    size_t size,
    uint32_t vertexCount,
    VkBufferUsageFlags usage,
    Renderer &renderer,
    VkCommandBuffer commandBuffer
)
{
    FreeResources ( renderer.GetDevice () );
    return UploadSimple ( data, size, vertexCount, usage, renderer, commandBuffer );
}

bool MeshGeometry::LoadMesh ( uint8_t const* vertexData,
    size_t vertexDataSize,
    uint32_t const* indices,
    uint32_t indexCount,
    GXAABB const &bounds,
    Renderer &renderer,
    VkCommandBuffer commandBuffer
)
{
    FreeResources ( renderer.GetDevice () );

    size_t const vertexOffset = sizeof ( uint32_t ) * indexCount;
    std::vector<uint8_t> storage ( vertexOffset + vertexDataSize );
    uint8_t* data = storage.data ();
    memcpy ( data, indices, vertexOffset );
    memcpy ( data + vertexOffset, vertexData, vertexDataSize );

    bool const result = UploadComplex ( data,
        vertexDataSize,
        indexCount,
        renderer,
        commandBuffer
    );

    if ( result )
        _bounds = bounds;

    return result;
}

void MeshGeometry::FreeResourceInternal ( VkDevice device )
{
    _vertexCount = 0U;

    if ( _bufferMemory != VK_NULL_HANDLE )
    {
        vkFreeMemory ( device, _bufferMemory, nullptr );
        _bufferMemory = VK_NULL_HANDLE;
        AV_UNREGISTER_DEVICE_MEMORY ( "MeshGeometry::_bufferMemory" )
    }

    if ( _indexBuffer != VK_NULL_HANDLE )
    {
        vkDestroyBuffer ( device, _indexBuffer, nullptr );
        _indexBuffer = VK_NULL_HANDLE;
        AV_UNREGISTER_BUFFER ( "MeshGeometry::_indexBuffer" )
    }

    if ( _vertexBuffer == VK_NULL_HANDLE )
        return;

    vkDestroyBuffer ( device, _vertexBuffer, nullptr );
    _vertexBuffer = VK_NULL_HANDLE;
    AV_UNREGISTER_BUFFER ( "MeshGeometry::_vertexBuffer" )
}

bool MeshGeometry::LoadFromMesh ( std::string &&fileName,
    VkBufferUsageFlags usage,
    Renderer &renderer,
    VkCommandBuffer commandBuffer
)
{
    FreeResourceInternal ( renderer.GetDevice () );

    File file ( fileName );

    if ( !file.LoadContent () )
        return false;

    std::vector<uint8_t>& content = file.GetContent ();
    auto const& header = *reinterpret_cast<GXNativeMeshHeader const*> ( content.data () );

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
                GXVec2& uv = vertices[ currentIndex ]._uv;
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

    for ( auto& item : converters )
        item.join ();

    bool const result = UploadSimple ( reinterpret_cast<uint8_t const*> ( vertices ),
        header.totalVertices * sizeof ( VertexInfo ),
        header.totalVertices,
        usage,
        renderer,
        commandBuffer
    );

    if ( !result )
        return false;

    _fileName = std::move ( fileName );
    return true;
}

bool MeshGeometry::LoadFromMesh2 ( std::string &&fileName,
    Renderer &renderer,
    VkCommandBuffer commandBuffer
)
{
    FreeResourceInternal ( renderer.GetDevice () );
    File file ( fileName );

    if ( !file.LoadContent () )
        return false;

    std::vector<uint8_t> const& content = file.GetContent ();
    uint8_t const* rawData = content.data ();
    auto const& header = *reinterpret_cast<Mesh2Header const*> ( rawData );

    bool const result = UploadComplex ( rawData + static_cast<size_t> ( header._indexDataOffset ),
        static_cast<size_t> ( header._vertexCount ) * sizeof ( Mesh2Vertex ),
        static_cast<uint32_t> ( header._indexCount ),
        renderer,
        commandBuffer
    );

    if ( !result )
        return false;

    Vec3 const& mins = header._bounds._min;
    Vec3 const& maxs = header._bounds._max;
    _bounds.Empty ();
    _bounds.AddVertex ( mins[ 0U ], mins[ 1U ], mins[ 2U ] );
    _bounds.AddVertex ( maxs[ 0U ], maxs[ 1U ], maxs[ 2U ] );

    _fileName = std::move ( fileName );
    return true;
}

bool MeshGeometry::UploadComplex ( uint8_t const* data,
    size_t vertexDataSize,
    uint32_t indexCount,
    Renderer &renderer,
    VkCommandBuffer commandBuffer
)
{
    constexpr VkBufferUsageFlags const indexBufferUsageFlags = AV_VK_FLAG ( VK_BUFFER_USAGE_INDEX_BUFFER_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT );

    VkBufferCreateInfo bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = static_cast<VkDeviceSize> ( indexCount * sizeof ( uint32_t ) ),
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

    if ( !result )
        return false;

    AV_REGISTER_BUFFER ( "MeshGeometry::_indexBuffer" )

    VkMemoryRequirements indexBufferMemoryRequirements;
    vkGetBufferMemoryRequirements ( device, _indexBuffer, &indexBufferMemoryRequirements );

    constexpr VkBufferUsageFlags const vertexBufferUsageFlags = AV_VK_FLAG ( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT );

    bufferInfo.size = static_cast<VkDeviceSize> ( vertexDataSize );
    bufferInfo.usage = vertexBufferUsageFlags;

    result = Renderer::CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &_vertexBuffer ),
        "MeshGeometry::UploadComplex",
        "Can't create vertex buffer"
    );

    if ( !result )
    {
        FreeResources ( device );
        return false;
    }

    AV_REGISTER_BUFFER ( "MeshGeometry::_vertexBuffer" )

    VkMemoryRequirements vertexBufferMemoryRequirements;
    vkGetBufferMemoryRequirements ( device, _vertexBuffer, &vertexBufferMemoryRequirements );

    if ( indexBufferMemoryRequirements.memoryTypeBits != vertexBufferMemoryRequirements.memoryTypeBits )
    {
        constexpr char const format[] =
R"__(MeshGeometry::UploadComplex - Memory usage bits are not same:
    index buffer memory bits: 0x%08X
    vertex buffer memory bits: 0x%08X)__";

        LogError ( format,
            indexBufferMemoryRequirements.memoryTypeBits,
            vertexBufferMemoryRequirements.memoryTypeBits
        );

        assert ( indexBufferMemoryRequirements.memoryTypeBits == vertexBufferMemoryRequirements.memoryTypeBits );
        FreeResources ( device );
        return false;
    }

    // Allocate all in one GPU memory.
    void* afterIndexBuffer = reinterpret_cast<void*> ( indexBufferMemoryRequirements.size );

    // Estimation from top.
    size_t rest = vertexBufferMemoryRequirements.size + vertexBufferMemoryRequirements.alignment;

    auto const vertexDataOffset = reinterpret_cast<size_t const> (
        std::align (
            static_cast<size_t> ( vertexBufferMemoryRequirements.alignment ),
            static_cast<size_t> ( vertexBufferMemoryRequirements.size ),
            afterIndexBuffer,
            rest
        )
    );

    VkMemoryRequirements const bufferMemoryRequirements
    {
        .size = static_cast<VkDeviceSize> ( vertexDataOffset ) + vertexBufferMemoryRequirements.size,
        .alignment = indexBufferMemoryRequirements.alignment,
        .memoryTypeBits = vertexBufferMemoryRequirements.memoryTypeBits
    };

    result = renderer.TryAllocateMemory ( _bufferMemory,
        bufferMemoryRequirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate buffer memory (MeshGeometry::UploadComplex)"
    );

    if ( !result )
    {
        FreeResources ( device );
        return false;
    }

    AV_REGISTER_DEVICE_MEMORY ( "MeshGeometry::_bufferMemory" )

    result = Renderer::CheckVkResult ( vkBindBufferMemory ( device, _indexBuffer, _bufferMemory, 0U ),
        "MeshGeometry::UploadComplex",
        "Can't bind index buffer memory"
    );

    if ( !result )
    {
        FreeResources ( device );
        return false;
    }

    result = Renderer::CheckVkResult (
        vkBindBufferMemory ( device, _vertexBuffer, _bufferMemory, static_cast<VkDeviceSize> ( vertexDataOffset ) ),
        "MeshGeometry::UploadComplex",
        "Can't bind vertex buffer memory"
    );

    if ( !result )
    {
        FreeResources ( device );
        return false;
    }

    VkBufferCopy const copyInfo[] =
    {
        {
            .srcOffset = 0U,
            .dstOffset = 0U,
            .size = indexBufferMemoryRequirements.size
        },
        {
            .srcOffset = indexBufferMemoryRequirements.size,
            .dstOffset = 0U,
            .size = vertexBufferMemoryRequirements.size
        }
    };

    constexpr VkBufferUsageFlags const usages[ std::size ( copyInfo ) ] =
    {
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
    };

    VkBuffer const dstBuffers[ std::size ( copyInfo ) ] = { _indexBuffer, _vertexBuffer };

    result = UploadInternal ( std::size ( usages ),
        copyInfo,
        usages,
        dstBuffers,
        data,
        indexBufferMemoryRequirements.size + vertexBufferMemoryRequirements.size,
        renderer,
        commandBuffer
    );

    if ( result )
        _vertexCount = indexCount;

    return result;
}

bool MeshGeometry::UploadInternal ( size_t numUploads,
    VkBufferCopy const* copyJobs,
    VkBufferUsageFlags const* usages,
    VkBuffer const* dstBuffers,
    uint8_t const* data,
    size_t dataSize,
    Renderer &renderer,
    VkCommandBuffer commandBuffer
)
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

    if ( !result )
    {
        FreeResources ( device );
        return false;
    }

    AV_REGISTER_BUFFER ( "MeshGeometry::_transferBuffer" )

    VkMemoryRequirements transferMemoryRequirements;
    vkGetBufferMemoryRequirements ( device, _transferBuffer, &transferMemoryRequirements );

    constexpr VkMemoryPropertyFlags const flags = AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ) |
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );

    result = renderer.TryAllocateMemory ( _transferMemory,
        transferMemoryRequirements,
        flags,
        "Can't allocate transfer memory (MeshGeometry::UploadInternal)"
    );

    if ( !result )
    {
        FreeResources ( device );
        return false;
    }

    AV_REGISTER_DEVICE_MEMORY ( "MeshGeometry::_transferMemory" )

    result = Renderer::CheckVkResult (
        vkBindBufferMemory ( device, _transferBuffer, _transferMemory, 0U ),
        "MeshGeometry::UploadInternal",
        "Can't bind transfer memory"
    );

    if ( !result )
    {
        FreeResources ( device );
        return false;
    }

    void* transferData = nullptr;

    result = Renderer::CheckVkResult (
        vkMapMemory ( device, _transferMemory, 0U, transferMemoryRequirements.size, 0U, &transferData ),
        "MeshGeometry::UploadInternal",
        "Can't map data"
    );

    if ( !result )
    {
        FreeResources ( device );
        return false;
    }

    memcpy ( transferData, data, dataSize );
    vkUnmapMemory ( device, _transferMemory );

    VkCommandBufferBeginInfo const commandBufferBeginInfo
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

    if ( !result )
    {
        FreeResources ( device );
        return false;
    }

    // Note most extreme case is 2 upload jobs (vertex buffer and index buffer).
    VkBufferMemoryBarrier barrierInfo[ 2U ];
    VkPipelineStageFlags srcStages = 0U;
    VkPipelineStageFlags dstStages = 0U;

    for ( size_t i = 0U; i < numUploads; ++i )
    {
        VkBufferCopy const& copyBuffer = copyJobs[ i ];
        auto const findResult = g_accessMapper.find ( usages[ i ] );

        if ( findResult == g_accessMapper.cend () )
        {
            LogError ( "MeshGeometry::UploadInternal - Unexpected usage 0x%08X", usages[ i ] );
            FreeResources ( device );
            return false;
        }

        BufferSyncItem const& syncItem = findResult->second;
        srcStages |= syncItem._srcStage;
        dstStages |= syncItem._dstStage;

        VkBufferMemoryBarrier& barrier = barrierInfo[ i ];
        barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        barrier.pNext = nullptr;
        barrier.buffer = dstBuffers[ i ];
        barrier.srcAccessMask = syncItem._srcAccessMask;
        barrier.dstAccessMask = syncItem._dstAccessMask;
        barrier.size = copyBuffer.size;
        barrier.offset = 0U;
        barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        vkCmdCopyBuffer ( commandBuffer, _transferBuffer, barrier.buffer, 1U, &copyBuffer );
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

    if ( !result )
    {
        FreeResources ( device );
        return false;
    }

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

    result = Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &submitInfo, VK_NULL_HANDLE ),
        "MeshGeometry::LoadFromMesh2",
        "Can't submit command"
    );

    if ( result )
        return true;

    FreeResources ( device );
    return false;
}

bool MeshGeometry::UploadSimple ( uint8_t const* data,
    size_t size,
    uint32_t vertexCount,
    VkBufferUsageFlags usage,
    Renderer &renderer,
    VkCommandBuffer commandBuffer
)
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

    if ( !result )
        return false;

    AV_REGISTER_BUFFER ( "MeshGeometry::_vertexBuffer" )

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements ( device, _vertexBuffer, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _bufferMemory,
        memoryRequirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate buffer memory (MeshGeometry::LoadMeshInternal)"
    );

    if ( !result )
    {
        FreeResources ( device );
        return false;
    }

    AV_REGISTER_DEVICE_MEMORY ( "MeshGeometry::_bufferMemory" )

    result = Renderer::CheckVkResult ( vkBindBufferMemory ( device, _vertexBuffer, _bufferMemory, 0U ),
        "MeshGeometry::UploadSimple",
        "Can't bind buffer memory"
    );

    if ( !result )
    {
        FreeResources ( device );
        return false;
    }

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
        commandBuffer
    );

    if ( !result )
        return false;

    _vertexCount = vertexCount;
    return true;
}

} // namespace android_vulkan
