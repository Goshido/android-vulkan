#include <precompiled_headers.hpp>
#include <platform/windows/pbr/stream_buffer.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

bool StreamBuffer::Init ( android_vulkan::Renderer &renderer,
    size_t elementSize,
    size_t elements,
    char const* name
) noexcept
{
    constexpr VkBufferUsageFlags gpuUsage = AV_VK_FLAG ( VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT );

    VkBufferCreateInfo bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = static_cast<VkDeviceSize> ( elementSize * elements ),
        .usage = gpuUsage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    if ( !CreateBuffer ( _gpu, renderer, bufferInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, name ) ) [[unlikely]]
        return false;

    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    constexpr VkMemoryPropertyFlags cpuProperties = AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ) |
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );

    if ( !CreateBuffer ( _staging, renderer, bufferInfo, cpuProperties, name ) ) [[unlikely]]
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

VkDeviceAddress StreamBuffer::AcquireAndConsume ( size_t /*elements*/ ) noexcept
{
    // FUCK
    return _bda;
}

void StreamBuffer::Commit () noexcept
{
    // FUCK
}

void StreamBuffer::IssueSync ( VkDevice /*device*/, VkCommandBuffer /*commandBuffer*/ ) const noexcept
{
    // FUCK
}

void StreamBuffer::Push ( void const* /*item*/ ) noexcept
{
    // FUCK
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
