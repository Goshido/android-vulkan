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

std::map<VkBufferUsageFlags, BufferSyncItem> const MeshGeometry::_accessMapper =
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

MeshGeometry::MeshGeometry ():
    _bounds {},
    _vertexBuffer ( VK_NULL_HANDLE ),
    _indexBuffer ( VK_NULL_HANDLE ),
    _bufferMemory ( VK_NULL_HANDLE ),
    _transferBuffer ( VK_NULL_HANDLE ),
    _transferMemory ( VK_NULL_HANDLE ),
    _vertexCount ( 0U )
{
    // NOTHING
}

void MeshGeometry::FreeResources ( android_vulkan::Renderer &renderer )
{
    FreeTransferResources ( renderer );
    FreeResourceInternal ( renderer );
    _fileName.clear ();
}

void MeshGeometry::FreeTransferResources ( android_vulkan::Renderer &renderer )
{
    VkDevice device = renderer.GetDevice ();

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

VkBuffer const& MeshGeometry::GetBuffer () const
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

bool MeshGeometry::IsIndexBufferPresent () const
{
    return _indexBuffer != VK_NULL_HANDLE;
}

bool MeshGeometry::IsUnique () const
{
    return _fileName.empty ();
}

bool MeshGeometry::LoadMesh ( std::string &&fileName,
    VkBufferUsageFlags usage,
    android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer
)
{
    if ( fileName.empty () )
    {
        android_vulkan::LogError ( "MeshGeometry::LoadMesh - Can't upload data. Filename is empty." );
        return false;
    }

    static std::regex const isMesh2 ( R"__(^.+?\.mesh2$)__" );
    std::smatch match;

    if ( std::regex_match ( fileName, match, isMesh2 ) )
        return LoadFromMesh2 ( std::move ( fileName ), usage, renderer, commandBuffer );

    static std::regex const isMesh ( R"__(^.+?\.mesh$)__" );

    if ( std::regex_match ( fileName, match, isMesh ) )
        return LoadFromMesh ( std::move ( fileName ), usage, renderer, commandBuffer );

    return false;
}

bool MeshGeometry::LoadMesh ( uint8_t const* data,
    size_t size,
    uint32_t vertexCount,
    VkBufferUsageFlags usage,
    android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer
)
{
    FreeResources ( renderer );
    return LoadMeshInternal ( data, size, vertexCount, usage, renderer, commandBuffer );
}

void MeshGeometry::FreeResourceInternal ( android_vulkan::Renderer &renderer )
{
    _vertexCount = 0U;
    VkDevice device = renderer.GetDevice ();

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
    android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer
)
{
    FreeResourceInternal ( renderer );

    android_vulkan::File file ( fileName );

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
    auto* vertices = reinterpret_cast<android_vulkan::VertexInfo*> ( content.data () + header.vboOffset );

    auto converter = [ & ] ( android_vulkan::VertexInfo* vertices,
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

    for ( auto &item : converters )
        item.join ();

    bool const result = LoadMeshInternal ( reinterpret_cast<uint8_t const*> ( vertices ),
        header.totalVertices * sizeof ( android_vulkan::VertexInfo ),
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
    VkBufferUsageFlags /*usage*/,
    android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer
)
{
    FreeResourceInternal ( renderer );

    android_vulkan::File file ( fileName );

    if ( !file.LoadContent () )
        return false;

    std::vector<uint8_t> const& content = file.GetContent ();
    uint8_t const* rawData = content.data ();
    auto const& header = *reinterpret_cast<Mesh2Header const*> ( rawData );

    constexpr VkBufferUsageFlags const indexBufferUsageFlags = AV_VK_FLAG ( VK_BUFFER_USAGE_INDEX_BUFFER_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT );

    VkBufferCreateInfo bufferInfo;
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.flags = 0U;
    bufferInfo.size = static_cast<VkDeviceSize> ( header._indexCount * sizeof ( Mesh2Index ) );
    bufferInfo.usage = indexBufferUsageFlags;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 0U;
    bufferInfo.pQueueFamilyIndices = nullptr;

    VkDevice device = renderer.GetDevice ();

    bool result = renderer.CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &_indexBuffer ),
        "MeshGeometry::LoadFromMesh2",
        "Can't create index buffer"
    );

    if ( !result )
        return false;

    AV_REGISTER_BUFFER ( "MeshGeometry::_indexBuffer" )

    VkMemoryRequirements indexBufferMemoryRequirements;
    vkGetBufferMemoryRequirements ( device, _indexBuffer, &indexBufferMemoryRequirements );

    constexpr VkBufferUsageFlags const vertexBufferUsageFlags = AV_VK_FLAG ( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT );

    bufferInfo.size = static_cast<VkDeviceSize> ( header._vertexCount * sizeof ( Mesh2Vertex ) );
    bufferInfo.usage = vertexBufferUsageFlags;

    result = renderer.CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &_vertexBuffer ),
        "MeshGeometry::LoadFromMesh2",
        "Can't create vertex buffer"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    AV_REGISTER_BUFFER ( "MeshGeometry::_vertexBuffer" )

    VkMemoryRequirements vertexBufferMemoryRequirements;
    vkGetBufferMemoryRequirements ( device, _vertexBuffer, &vertexBufferMemoryRequirements );

    if ( indexBufferMemoryRequirements.memoryTypeBits != vertexBufferMemoryRequirements.memoryTypeBits )
    {
        constexpr char const format[] =
R"__(MeshGeometry::LoadFromMesh2 - Memory usage bits are not same:
    index buffer memory bits: 0x%08X
    vertex buffer memory bits: 0x%08X)__";

        android_vulkan::LogError ( format,
            indexBufferMemoryRequirements.memoryTypeBits,
            vertexBufferMemoryRequirements.memoryTypeBits
        );

        assert ( indexBufferMemoryRequirements.memoryTypeBits == vertexBufferMemoryRequirements.memoryTypeBits );
        FreeResources ( renderer );
        return false;
    }

    // Allocate all in one GPU memory.
    void* afterIndexBuffer = reinterpret_cast<void*> ( indexBufferMemoryRequirements.size );

    // Estimation from top.
    size_t rest = vertexBufferMemoryRequirements.size + vertexBufferMemoryRequirements.alignment;

    size_t const vertexDataOffset = reinterpret_cast<size_t> (
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
        "Can't allocate buffer memory (MeshGeometry::LoadFromMesh2)"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    AV_REGISTER_DEVICE_MEMORY ( "MeshGeometry::_bufferMemory" )

    result = renderer.CheckVkResult ( vkBindBufferMemory ( device, _indexBuffer, _bufferMemory, 0U ),
        "MeshGeometry::LoadFromMesh2",
        "Can't bind index buffer memory"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    result = renderer.CheckVkResult (
        vkBindBufferMemory ( device, _vertexBuffer, _bufferMemory, static_cast<VkDeviceSize> ( vertexDataOffset ) ),
        "MeshGeometry::LoadFromMesh2",
        "Can't bind vertex buffer memory"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.size = bufferMemoryRequirements.size;

    result = renderer.CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &_transferBuffer ),
        "MeshGeometry::LoadFromMesh2",
        "Can't create transfer buffer"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    AV_REGISTER_BUFFER ( "MeshGeometry::_transferBuffer" )

    VkMemoryRequirements transferMemoryRequirements;
    vkGetBufferMemoryRequirements ( device, _transferBuffer, &transferMemoryRequirements );

    result = renderer.TryAllocateMemory ( _transferMemory,
        transferMemoryRequirements,
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ) | AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ),
        "Can't allocate transfer memory (MeshGeometry::LoadFromMesh2)"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    AV_REGISTER_DEVICE_MEMORY ( "MeshGeometry::_transferMemory" )

    result = renderer.CheckVkResult ( vkBindBufferMemory ( device, _transferBuffer, _transferMemory, 0U ),
        "MeshGeometry::LoadFromMesh2",
        "Can't bind transfer memory"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    void* transferData = nullptr;

    result = renderer.CheckVkResult (
        vkMapMemory ( device, _transferMemory, 0U, transferMemoryRequirements.size, 0U, &transferData ),
        "MeshGeometry::LoadFromMesh2",
        "Can't map data"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    memcpy ( transferData,
        rawData + static_cast<size_t> ( header._indexDataOffset ),
        static_cast<size_t> ( indexBufferMemoryRequirements.size )
    );

    memcpy ( static_cast<uint8_t*> ( transferData ) + vertexDataOffset,
        rawData + static_cast<size_t> ( header._vertexDataOffset ),
        static_cast<size_t> ( vertexBufferMemoryRequirements.size )
    );

    vkUnmapMemory ( device, _transferMemory );

    VkCommandBufferBeginInfo commandBufferBeginInfo;
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = nullptr;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    commandBufferBeginInfo.pInheritanceInfo = nullptr;

    result = renderer.CheckVkResult ( vkBeginCommandBuffer ( commandBuffer, &commandBufferBeginInfo ),
        "MeshGeometry::LoadFromMesh2",
        "Can't begin command buffer"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    VkBufferCopy copyInfo[ 2U ];
    VkBufferCopy& indexCopy = copyInfo[ 0U ];
    indexCopy.size = indexBufferMemoryRequirements.size;
    indexCopy.srcOffset = indexCopy.dstOffset = 0U;

    vkCmdCopyBuffer ( commandBuffer,
        _transferBuffer,
        _indexBuffer,
        1U,
        copyInfo
    );

    VkBufferCopy& vertexCopy = copyInfo[ 1U ];
    vertexCopy.size = vertexBufferMemoryRequirements.size;
    vertexCopy.srcOffset = static_cast<VkDeviceSize> ( vertexDataOffset );
    vertexCopy.dstOffset = 0U;

    vkCmdCopyBuffer ( commandBuffer,
        _transferBuffer,
        _vertexBuffer,
        1U,
        copyInfo + 1U
    );

    VkBufferMemoryBarrier barrierInfo[ 2U ];

    auto const indexFindResult = _accessMapper.find ( VK_BUFFER_USAGE_INDEX_BUFFER_BIT );

    if ( indexFindResult == _accessMapper.cend () )
    {
        android_vulkan::LogError ( "MeshGeometry::LoadFromMesh2 - Unexpected usage 0x%08X", indexFindResult );
        return false;
    }

    BufferSyncItem const& indexSyncItem = indexFindResult->second;

    VkBufferMemoryBarrier& indexBarrier = barrierInfo[ 0U ];
    indexBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    indexBarrier.pNext = nullptr;
    indexBarrier.buffer = _indexBuffer;
    indexBarrier.srcAccessMask = indexSyncItem._srcAccessMask;
    indexBarrier.dstAccessMask = indexSyncItem._dstAccessMask;
    indexBarrier.size = indexBufferMemoryRequirements.size;
    indexBarrier.offset = 0U;
    indexBarrier.srcQueueFamilyIndex = indexBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    auto const vertexFindResult = _accessMapper.find ( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT );

    if ( vertexFindResult == _accessMapper.cend () )
    {
        android_vulkan::LogError ( "MeshGeometry::LoadFromMesh2 - Unexpected usage 0x%08X", vertexFindResult );
        return false;
    }

    BufferSyncItem const& vertexSyncItem = vertexFindResult->second;

    VkBufferMemoryBarrier& vertexBarrier = barrierInfo[ 1U ];
    vertexBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    vertexBarrier.pNext = nullptr;
    vertexBarrier.buffer = _vertexBuffer;
    vertexBarrier.srcAccessMask = vertexSyncItem._srcAccessMask;
    vertexBarrier.dstAccessMask = vertexSyncItem._dstAccessMask;
    vertexBarrier.size = vertexBufferMemoryRequirements.size;
    vertexBarrier.offset = 0U;
    vertexBarrier.srcQueueFamilyIndex = vertexBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    vkCmdPipelineBarrier ( commandBuffer,
        indexSyncItem._srcStage | vertexSyncItem._srcStage,
        indexSyncItem._dstStage | vertexSyncItem._dstStage,
        0U,
        0U,
        nullptr,
        static_cast<uint32_t> ( std::size ( barrierInfo ) ),
        barrierInfo,
        0U,
        nullptr
    );

    result = renderer.CheckVkResult ( vkEndCommandBuffer ( commandBuffer ),
        "MeshGeometry::LoadFromMesh2",
        "Can't end command buffer"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.commandBufferCount = 1U;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.waitSemaphoreCount = 0U;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.signalSemaphoreCount = 0U;
    submitInfo.pSignalSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;

    result = renderer.CheckVkResult ( vkQueueSubmit ( renderer.GetQueue (), 1U, &submitInfo, VK_NULL_HANDLE ),
        "MeshGeometry::LoadFromMesh2",
        "Can't submit command"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    Vec3 const& mins = header._bounds._min;
    Vec3 const& maxs = header._bounds._max;
    _bounds.Empty ();
    _bounds.AddVertex ( mins[ 0U ], mins[ 1U ], mins[ 2U ] );
    _bounds.AddVertex ( maxs[ 0U ], maxs[ 1U ], maxs[ 2U ] );

    _vertexCount = static_cast<uint32_t> ( header._indexCount );
    _fileName = std::move ( fileName );
    return true;
}

bool MeshGeometry::LoadMeshInternal ( uint8_t const* data,
    size_t size,
    uint32_t vertexCount,
    VkBufferUsageFlags usage,
    android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer
)
{
    VkBufferCreateInfo bufferInfo;
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.flags = 0U;
    bufferInfo.usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 0U;
    bufferInfo.pQueueFamilyIndices = nullptr;
    bufferInfo.size = size;

    VkDevice device = renderer.GetDevice ();

    bool result = renderer.CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &_vertexBuffer ),
        "MeshGeometry::LoadMeshInternal",
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
        FreeResources ( renderer );
        return false;
    }

    AV_REGISTER_DEVICE_MEMORY ( "MeshGeometry::_bufferMemory" )

    result = renderer.CheckVkResult ( vkBindBufferMemory ( device, _vertexBuffer, _bufferMemory, 0U ),
        "MeshGeometry::LoadMeshInternal",
        "Can't bind buffer memory"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    result = renderer.CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &_transferBuffer ),
        "MeshGeometry::LoadMeshInternal",
        "Can't create transfer buffer"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    AV_REGISTER_BUFFER ( "MeshGeometry::_transferBuffer" )

    vkGetBufferMemoryRequirements ( device, _transferBuffer, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _transferMemory,
        memoryRequirements,
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        "Can't allocate transfer memory (MeshGeometry::LoadMeshInternal)"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    AV_REGISTER_DEVICE_MEMORY ( "MeshGeometry::_transferMemory" )

    result = renderer.CheckVkResult ( vkBindBufferMemory ( device, _transferBuffer, _transferMemory, 0U ),
        "MeshGeometry::LoadMeshInternal",
        "Can't bind transfer memory"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    void* transferData = nullptr;

    result = renderer.CheckVkResult ( vkMapMemory ( device, _transferMemory, 0U, size, 0U, &transferData ),
        "MeshGeometry::LoadMeshInternal",
        "Can't map data"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    memcpy ( transferData, data, size );
    vkUnmapMemory ( device, _transferMemory );

    VkCommandBufferBeginInfo commandBufferBeginInfo;
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = nullptr;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    commandBufferBeginInfo.pInheritanceInfo = nullptr;

    result = renderer.CheckVkResult ( vkBeginCommandBuffer ( commandBuffer, &commandBufferBeginInfo ),
        "MeshGeometry::LoadMeshInternal",
        "Can't begin command buffer"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    VkBufferCopy copyInfo;
    copyInfo.size = size;
    copyInfo.srcOffset = copyInfo.dstOffset = 0U;

    vkCmdCopyBuffer ( commandBuffer, _transferBuffer, _vertexBuffer, 1U, &copyInfo );

    auto const findResult = _accessMapper.find ( usage );

    if ( findResult == _accessMapper.cend () )
    {
        android_vulkan::LogError ( "MeshGeometry::LoadMeshInternal - Unexpected usage 0x%08X", usage );
        return false;
    }

    BufferSyncItem const& syncItem = findResult->second;

    VkBufferMemoryBarrier barrierInfo;
    barrierInfo.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    barrierInfo.pNext = nullptr;
    barrierInfo.buffer = _vertexBuffer;
    barrierInfo.srcAccessMask = syncItem._srcAccessMask;
    barrierInfo.dstAccessMask = syncItem._dstAccessMask;
    barrierInfo.size = size;
    barrierInfo.offset = 0U;
    barrierInfo.srcQueueFamilyIndex = barrierInfo.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    vkCmdPipelineBarrier ( commandBuffer,
        syncItem._srcStage,
        syncItem._dstStage,
        0U,
        0U,
        nullptr,
        1U,
        &barrierInfo,
        0U,
        nullptr
    );

    result = renderer.CheckVkResult ( vkEndCommandBuffer ( commandBuffer ),
        "MeshGeometry::LoadMeshInternal",
        "Can't end command buffer"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.commandBufferCount = 1U;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.waitSemaphoreCount = 0U;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.signalSemaphoreCount = 0U;
    submitInfo.pSignalSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;

    result = renderer.CheckVkResult ( vkQueueSubmit ( renderer.GetQueue (), 1U, &submitInfo, VK_NULL_HANDLE ),
        "MeshGeometry::LoadMeshInternal",
        "Can't submit command"
    );

    if ( result )
    {
        _vertexCount = vertexCount;
        return true;
    }

    FreeResources ( renderer );
    return false;
}

} // namespace android_vulkan
