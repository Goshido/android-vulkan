#include <precompiled_headers.hpp>
#include <platform/windows/pbr/stream_buffer.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

bool StreamBuffer::Init ( android_vulkan::Renderer &renderer,
    size_t count,
    size_t itemSize,
    char const* name
) noexcept
{
    _count = count;
    _itemSize = itemSize;

    constexpr VkBufferUsageFlags gpuUsage = AV_VK_FLAG ( VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT );

    VkBufferCreateInfo bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = static_cast<VkDeviceSize> ( count * itemSize ),
        .usage = gpuUsage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    if ( !CreateBuffer ( _gpu, renderer, bufferInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, name ) ) [[unlikely]]
        return false;

    _barrier.buffer = _gpu._buffer;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    constexpr VkMemoryPropertyFlags cpuProperties = AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ) |
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );

    bool const result = CreateBuffer ( _staging, renderer, bufferInfo, cpuProperties, name ) &&
        renderer.MapMemory ( reinterpret_cast<void* &> ( _data ),
            _staging._memory,
            _staging._offset,
            "pbr::StreamBuffer::Init",
            "Can't map memory"
        );

    if ( !result ) [[unlikely]]
        return false;

    VkBufferDeviceAddressInfo const bdaInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .pNext = nullptr,
        .buffer = _gpu._buffer
    };

    _bda = vkGetBufferDeviceAddress ( renderer.GetDevice (), &bdaInfo );
    return true;
}

void StreamBuffer::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( std::exchange ( _data, nullptr ) ) [[likely]]
        renderer.UnmapMemory ( _staging._memory );

    auto const freeBuffer = [ &renderer, device = renderer.GetDevice () ] ( Buffer &buffer ) noexcept {
        if ( VkBuffer b = std::exchange ( buffer._buffer, VK_NULL_HANDLE ); b != VK_NULL_HANDLE ) [[likely]]
            vkDestroyBuffer ( device, b, nullptr );

        if ( buffer._memory == VK_NULL_HANDLE ) [[unlikely]]
            return;

        renderer.FreeMemory ( std::exchange ( buffer._memory, VK_NULL_HANDLE ), std::exchange ( buffer._offset, 0U ) );
    };

    freeBuffer ( _staging );
    freeBuffer ( _gpu );
    _bda = 0U;
}

VkDeviceAddress StreamBuffer::AcquireAndConsume ( size_t count ) noexcept
{
    return _bda + static_cast<VkDeviceAddress> ( _itemSize * std::exchange ( _readIndex, _readIndex + count ) );
}

void StreamBuffer::Commit () noexcept
{
    _baseIndex = _writeIndex;
    _written = 0U;
}

void StreamBuffer::IssueSync ( VkCommandBuffer commandBuffer ) noexcept
{
    auto const from = static_cast<VkDeviceSize> ( _baseIndex * _itemSize );

    VkBufferCopy const copy
    {
        .srcOffset = from,
        .dstOffset = from,
        .size = static_cast<VkDeviceSize> ( _written * _itemSize )
    };

    vkCmdCopyBuffer ( commandBuffer, _staging._buffer, _gpu._buffer, 1U, &copy );

    _barrier.size = copy.size;
    _barrier.offset = copy.srcOffset;
    vkCmdPipelineBarrier2 ( commandBuffer, &_depInfo );
}

void StreamBuffer::Push ( void const* item ) noexcept
{
    if ( _writeIndex >= _count ) [[unlikely]]
    {
        if ( _written > 0U ) [[likely]]
            std::memcpy ( _data, _data + _itemSize * _baseIndex, _itemSize * _written );

        _baseIndex = 0U;
        _readIndex = 0U;
        _writeIndex = _written;
    }

    std::memcpy ( _data + _itemSize * _writeIndex++, item, _itemSize );
    ++_written;
}

bool StreamBuffer::CreateBuffer ( Buffer &buffer,
    android_vulkan::Renderer &renderer,
    VkBufferCreateInfo const &bufferInfo,
    VkMemoryPropertyFlags memoryProperties,
    [[maybe_unused]] char const* name
) noexcept
{
    VkDevice device = renderer.GetDevice ();

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateBuffer ( device, &bufferInfo, nullptr, &buffer._buffer ),
        "StreamBuffer::CreateBuffer",
        "Can't create buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, buffer._buffer, VK_OBJECT_TYPE_BUFFER, "%s", name )

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements ( device, buffer._buffer, &memoryRequirements );

    return
        renderer.TryAllocateMemory ( buffer._memory,
            buffer._offset,
            memoryRequirements,
            memoryProperties,
            "Can't allocate memory (StreamBuffer::CreateBuffer)"
        ) &&

        android_vulkan::Renderer::CheckVkResult (
            vkBindBufferMemory ( device, buffer._buffer, buffer._memory, buffer._offset ),
            "StreamBuffer::Init",
            "Can't bind memory"
        );
}

} // namespace pbr
