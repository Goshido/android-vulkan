#include <av_assert.hpp>
#include <uniform_buffer.hpp>
#include <vulkan_utils.hpp>


namespace android_vulkan {

void UniformBuffer::FreeResources ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    if ( _transfer != VK_NULL_HANDLE )
    {
        vkDestroyBuffer ( device, _transfer, nullptr );
        _transfer = VK_NULL_HANDLE;
        AV_UNREGISTER_BUFFER ( "UniformBuffer::_transfer" )
    }

    if ( _transferMemory != VK_NULL_HANDLE )
    {
        renderer.FreeMemory ( _transferMemory, _transferOffset );
        _transferMemory = VK_NULL_HANDLE;
        _transferOffset = std::numeric_limits<VkDeviceSize>::max ();
        AV_UNREGISTER_DEVICE_MEMORY ( "UniformBuffer::_transferMemory" )
    }

    if ( _buffer != VK_NULL_HANDLE )
    {
        vkDestroyBuffer ( device, _buffer, nullptr );
        _buffer = VK_NULL_HANDLE;
        AV_UNREGISTER_BUFFER ( "UniformBuffer::_buffer" )
    }

    if ( _bufferMemory != VK_NULL_HANDLE )
    {
        renderer.FreeMemory ( _bufferMemory, _bufferOffset );
        _bufferMemory = VK_NULL_HANDLE;
        _bufferOffset = std::numeric_limits<VkDeviceSize>::max ();
        AV_UNREGISTER_DEVICE_MEMORY ( "UniformBuffer::_bufferMemory" )
    }

    if ( _commandBuffer != VK_NULL_HANDLE )
    {
        vkFreeCommandBuffers ( device, _commandPool, 1U, &_commandBuffer );
        _commandBuffer = VK_NULL_HANDLE;
    }

    _size = 0U;
    _commandPool = VK_NULL_HANDLE;
    _targetStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
}

VkBuffer UniformBuffer::GetBuffer () const noexcept
{
    return _buffer;
}

size_t UniformBuffer::GetSize () const noexcept
{
    return _size;
}

bool UniformBuffer::Init ( android_vulkan::Renderer &renderer,
    VkCommandPool commandPool,
    VkPipelineStageFlags targetStages
) noexcept
{
    if ( _commandPool != VK_NULL_HANDLE )
        return true;

    _commandPool = commandPool;
    _targetStages = targetStages;

    VkCommandBufferAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1U
    };

    return Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( renderer.GetDevice (), &allocateInfo, &_commandBuffer ),
        "UniformBuffer::Init",
        "Can't allocate command buffer"
    );
}

bool UniformBuffer::Update ( android_vulkan::Renderer &renderer,
    VkFence fence,
    uint8_t const* data,
    size_t size
) noexcept
{
    AV_ASSERT ( size )

    if ( !_size && !InitResources ( renderer, size ) )
        return false;

    if ( !data )
        return true;

    void* dst = nullptr;

    bool const result = renderer.MapMemory ( dst,
        _transferMemory,
        _transferOffset,
        "UniformBuffer::Update",
        "Can't map transfer memory"
    );

    if ( !result )
        return false;

    std::memcpy ( dst, data, size );
    renderer.UnmapMemory ( _transferMemory );

    VkSubmitInfo const submitInfo
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 0U,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = nullptr,
        .commandBufferCount = 1U,
        .pCommandBuffers = &_commandBuffer,
        .signalSemaphoreCount = 0U,
        .pSignalSemaphores = nullptr
    };

    return Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &submitInfo, fence ),
        "UniformBuffer::Update",
        "Can't submit upload command"
    );
}

bool UniformBuffer::InitResources ( android_vulkan::Renderer &renderer, size_t size ) noexcept
{
    VkBufferCreateInfo bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = size,
        .usage = AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ) | AV_VK_FLAG ( VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT ),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    VkDevice device = renderer.GetDevice ();

    bool result = Renderer::CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &_buffer ),
        "UniformBuffer::InitResources",
        "Can't create buffer"
    );

    if ( !result )
        return false;

    AV_REGISTER_BUFFER ( "UniformBuffer::_buffer" )

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements ( device, _buffer, &requirements );

    result = renderer.TryAllocateMemory ( _bufferMemory,
        _bufferOffset,
        requirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate buffer memory (UniformBuffer::InitResources)"
    );

    if ( !result )
        return false;

    AV_REGISTER_DEVICE_MEMORY ( "UniformBuffer::_bufferMemory" )

    result = Renderer::CheckVkResult ( vkBindBufferMemory ( device, _buffer, _bufferMemory, _bufferOffset ),
        "UniformBuffer::InitResources",
        "Can't bind buffer memory"
    );

    if ( !result )
        return false;

    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    result = Renderer::CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &_transfer ),
        "UniformBuffer::InitResources",
        "Can't create transfer buffer"
    );

    if ( !result )
        return false;

    AV_REGISTER_BUFFER ( "UniformBuffer::_transfer" )
    vkGetBufferMemoryRequirements ( device, _transfer, &requirements );

    result = renderer.TryAllocateMemory ( _transferMemory,
        _transferOffset,
        requirements,
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ) | AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ),
        "Can't allocate transfer memory (UniformBuffer::InitResources)"
    );

    if ( !result )
        return false;

    AV_REGISTER_DEVICE_MEMORY ( "UniformBuffer::_transferMemory" )

    result = Renderer::CheckVkResult ( vkBindBufferMemory ( device, _transfer, _transferMemory, _transferOffset ),
        "UniformBuffer::InitResources",
        "Can't bind transfer memory"
    );

    if ( !result )
        return false;

    constexpr VkCommandBufferBeginInfo beginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .pInheritanceInfo = nullptr
    };

    result = Renderer::CheckVkResult ( vkBeginCommandBuffer ( _commandBuffer, &beginInfo ),
        "UniformBuffer::InitResources",
        "Can't begin command buffer"
    );

    if ( !result )
        return false;

    VkBufferMemoryBarrier barrier
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = 0U,
        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = _buffer,
        .offset = 0U,
        .size = size
    };

    vkCmdPipelineBarrier ( _commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0U,
        0U,
        nullptr,
        1U,
        &barrier,
        0U,
        nullptr
    );

    VkBufferCopy copyInfo;
    copyInfo.size = size;
    copyInfo.srcOffset = copyInfo.dstOffset = 0U;

    vkCmdCopyBuffer ( _commandBuffer, _transfer, _buffer, 1U, &copyInfo );

    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT;

    vkCmdPipelineBarrier ( _commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        _targetStages,
        0U,
        0U,
        nullptr,
        1U,
        &barrier,
        0U,
        nullptr
    );

    result = Renderer::CheckVkResult ( vkEndCommandBuffer ( _commandBuffer ),
        "UniformBuffer::InitResources",
        "Can't end command buffer"
    );

    if ( !result )
        return false;

    _size = size;
    return true;
}

} // namespace android_vulkan
