#include <av_assert.hpp>
#include <uniform_buffer.hpp>
#include <vulkan_utils.hpp>


namespace android_vulkan {

void UniformBuffer::FreeResources ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    for ( VkBuffer buffer : _buffers )
        vkDestroyBuffer ( device, buffer, nullptr );

    if ( _memory != VK_NULL_HANDLE )
    {
        renderer.FreeMemory ( _memory, _offset );
        _memory = VK_NULL_HANDLE;
        _offset = std::numeric_limits<VkDeviceSize>::max ();
    }

    _size = 0U;
    _index = 0U;
}

VkDeviceSize UniformBuffer::GetSize () const noexcept
{
    return static_cast<VkDeviceSize> ( _size );
}

bool UniformBuffer::Init ( android_vulkan::Renderer &renderer, size_t size, size_t amount ) noexcept
{
    _size = static_cast<VkDeviceSize> ( size );

    VkBufferCreateInfo const bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = _size,
        .usage = AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ) | AV_VK_FLAG ( VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT ),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    VkMemoryRequirements requirements;
    VkDevice device = renderer.GetDevice ();
    VkBuffer buffer;

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateBuffer ( device, &bufferInfo, nullptr, &buffer ),
        "UniformBuffer::Init",
        "Can't create uniform buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    vkGetBufferMemoryRequirements ( device, buffer, &requirements );
    vkDestroyBuffer ( device, buffer, nullptr );

    auto const alignment = static_cast<size_t> ( requirements.alignment );
    size_t const alignMultiplier = ( size + alignment - 1U ) / alignment;
    size_t const alignedBlockSize = alignMultiplier * alignment;
    requirements.size = static_cast<VkDeviceSize> ( alignedBlockSize );

    _index = 0U;
    _buffers.reserve ( amount );
    requirements.size *= static_cast<VkDeviceSize> ( amount );

    result = renderer.TryAllocateMemory ( _memory,
        _offset,
        requirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate GPU memory (UniformBuffer::Init)"
    );

    if ( !result ) [[unlikely]]
        return false;

    VkDeviceSize offset = _offset;

    for ( size_t i = 0U; i < amount; ++i )
    {
        result = android_vulkan::Renderer::CheckVkResult (
            vkCreateBuffer ( device, &bufferInfo, nullptr, &buffer ),
            "UniformBuffer::Init",
            "Can't create uniform buffer"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device, buffer, VK_OBJECT_TYPE_BUFFER, "UniformBuffer::_buffers::item" )

        vkGetBufferMemoryRequirements ( device, buffer, &requirements );

        result = android_vulkan::Renderer::CheckVkResult (
            vkBindBufferMemory ( device, buffer, _memory, offset ),
            "UniformBuffer::Init",
            "Can't bind uniform buffer memory"
        );

        if ( !result ) [[unlikely]]
            return false;

        offset += alignedBlockSize;
        _buffers.push_back ( buffer );
    }

    return true;
}

VkBuffer UniformBuffer::Update ( VkCommandBuffer commandBuffer, uint8_t const* data ) noexcept
{
    VkBuffer buffer = _buffers[ _index ];

    VkBufferMemoryBarrier bufferBarrier {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_NONE,
        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = buffer,
        .offset = 0U,
        .size = _size
    };

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0U,
        0U,
        nullptr,
        1U,
        &bufferBarrier,
        0U,
        nullptr
    );

    vkCmdUpdateBuffer ( commandBuffer, buffer, 0U, _size, data );
    _index = ( _index + 1U ) % _buffers.size ();
    return buffer;
}

} // namespace android_vulkan
