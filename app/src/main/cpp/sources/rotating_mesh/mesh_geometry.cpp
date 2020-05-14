#include <rotating_mesh/mesh_geometry.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE

#include <logger.h>
#include <vulkan_utils.h>


namespace rotating_mesh {

const std::map<VkBufferUsageFlags, BufferSyncItem> MeshGeometry::_accessMapper =
{
    {
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,

        BufferSyncItem ( VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_ACCESS_INDEX_READ_BIT,
            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT
        )
    },

    {
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,

        BufferSyncItem ( VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT
        )
    }
};

MeshGeometry::MeshGeometry ():
    _buffer ( VK_NULL_HANDLE ),
    _bufferMemory ( VK_NULL_HANDLE ),
    _transferBuffer ( VK_NULL_HANDLE ),
    _transferMemory ( VK_NULL_HANDLE )
{
    // NOTHING

    // TODO REMOVE THIS
    printf ( "%zu", _fileName.size () );
}

void MeshGeometry::FreeResources ( android_vulkan::Renderer &renderer )
{
    FreeTransferResources ( renderer );

    const VkDevice device = renderer.GetDevice ();

    if ( _bufferMemory != VK_NULL_HANDLE )
    {
        vkFreeMemory ( device, _bufferMemory, nullptr );
        _bufferMemory = VK_NULL_HANDLE;
        AV_UNREGISTER_DEVICE_MEMORY ( "MeshGeometry::_bufferMemory" )
    }

    if ( _buffer == VK_NULL_HANDLE )
        return;

    vkDestroyBuffer ( device, _buffer, nullptr );
    _buffer = VK_NULL_HANDLE;
    AV_UNREGISTER_BUFFER ( "MeshGeometry::_buffer" )
}

void MeshGeometry::FreeTransferResources ( android_vulkan::Renderer &renderer )
{
    const VkDevice device = renderer.GetDevice ();

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

const VkBuffer& MeshGeometry::GetBuffer () const
{
    return _buffer;
}

bool MeshGeometry::LoadMesh ( std::string&& /*fileName*/,
    VkBufferUsageFlags /*usage*/,
    android_vulkan::Renderer& /*renderer*/,
    VkCommandBuffer /*commandBuffer*/
)
{
    assert ( !"MeshGeometry::LoadMesh - Implement me!" );
    return false;
}

bool MeshGeometry::LoadMesh ( const uint8_t* data,
    size_t size,
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

    const VkDevice device = renderer.GetDevice ();

    bool result = renderer.CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &_buffer ),
        "MeshGeometry::LoadMesh",
        "Can't create buffer"
    );

    if ( !result )
        return false;

    AV_REGISTER_BUFFER ( "MeshGeometry::_buffer" )

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements ( device, _buffer, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _bufferMemory,
        memoryRequirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate buffer memory (MeshGeometry::LoadMesh)"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    AV_REGISTER_DEVICE_MEMORY ( "MeshGeometry::_bufferMemory" )

    result = renderer.CheckVkResult ( vkBindBufferMemory ( device, _buffer, _bufferMemory, 0U ),
        "MeshGeometry::LoadMesh",
        "Can't bind buffer memory"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    result = renderer.CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &_transferBuffer ),
        "MeshGeometry::LoadMesh",
        "Can't create transfer buffer"
    );

    if ( !result )
        return false;

    AV_REGISTER_BUFFER ( "MeshGeometry::_transferBuffer" )

    vkGetBufferMemoryRequirements ( device, _transferBuffer, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _transferMemory,
        memoryRequirements,
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        "Can't allocate transfer memory (MeshGeometry::LoadMesh)"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    AV_REGISTER_DEVICE_MEMORY ( "MeshGeometry::_transferMemory" )

    result = renderer.CheckVkResult ( vkBindBufferMemory ( device, _transferBuffer, _transferMemory, 0U ),
        "MeshGeometry::LoadMesh",
        "Can't bind transfer memory"
    );

    if ( !result )
    {
        FreeResources ( renderer );
        return false;
    }

    void* transferData = nullptr;

    result = renderer.CheckVkResult ( vkMapMemory ( device, _transferMemory, 0U, size, 0U, &transferData ),
        "MeshGeometry::LoadMesh",
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
        "MeshGeometry::LoadMesh",
        "Can't begin command buffer"
    );

    VkBufferCopy copyInfo;
    copyInfo.size = size;
    copyInfo.srcOffset = copyInfo.dstOffset = 0U;

    vkCmdCopyBuffer ( commandBuffer, _transferBuffer, _buffer, 1U, &copyInfo );

    const auto findResult = _accessMapper.find ( usage );

    if ( findResult == _accessMapper.cend () )
    {
        android_vulkan::LogError ( "MeshGeometry::LoadMesh - Unexpected usage 0x%08X", usage );
        return false;
    }

    const BufferSyncItem& syncItem = findResult->second;

    VkBufferMemoryBarrier barrierInfo;
    barrierInfo.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    barrierInfo.pNext = nullptr;
    barrierInfo.buffer = _buffer;
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
        "MeshGeometry::LoadMesh",
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
        "MeshGeometry::LoadMesh",
        "Can't submit command"
    );

    if ( result )
        return true;

    FreeResources ( renderer );
    return false;
}

} // namespace rotating_mesh
