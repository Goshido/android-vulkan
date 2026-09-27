#include <precompiled_headers.hpp>
#include <platform/windows/pbr/gpu_buffer.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

GPUBuffer::GPUBuffer ( GPUBuffer &&other ) noexcept:
    _buffer ( std::exchange ( other._buffer, VK_NULL_HANDLE ) ),
    _bda ( std::exchange ( other._bda, 0U ) ),
    _memory ( std::exchange ( other._memory, VK_NULL_HANDLE ) ),
    _offset ( std::exchange ( other._offset, std::numeric_limits<VkDeviceSize>::max () ) ),
    _range ( std::exchange ( other._range, 0U ) ),
    _heapIndex ( std::exchange ( other._heapIndex, 0U ) )
{
    // NOTHING
}

GPUBuffer &GPUBuffer::operator = ( GPUBuffer &&other ) noexcept
{
    if ( this == &other ) [[unlikely]]
        return *this;

    _buffer = std::exchange ( other._buffer, VK_NULL_HANDLE );
    _bda = std::exchange ( other._bda, 0U );
    _memory = std::exchange ( other._memory, VK_NULL_HANDLE );
    _offset = std::exchange ( other._offset, std::numeric_limits<VkDeviceSize>::max () );
    _range = std::exchange ( other._range, 0U );
    _heapIndex = std::exchange ( other._heapIndex, 0U );

    return *this;
}

bool GPUBuffer::IsInit () const noexcept
{
    return _memory != VK_NULL_HANDLE;
}

bool GPUBuffer::IsConnectedToResourceHeap () const noexcept
{
    return _heapIndex != 0U;
}

uint32_t GPUBuffer::GetHeapIndex () const noexcept
{
    return _heapIndex;
}

VkDeviceAddress GPUBuffer::GetBDA () const noexcept
{
    return _bda;
}

VkBuffer GPUBuffer::GetBuffer () const noexcept
{
    return _buffer;
}

bool GPUBuffer::Init ( android_vulkan::Renderer &renderer,
    size_t size,
    VkBufferUsageFlags usage,
    [[maybe_unused]] char const *name
) noexcept
{
    VkBufferCreateInfo const bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = static_cast<VkDeviceSize> ( size ),
        .usage = usage | AV_VK_FLAG ( VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT ),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateBuffer ( device, &bufferInfo, nullptr, &_buffer ),
        "pbr::GPUBuffer::Init",
        "Can't create buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _buffer, VK_OBJECT_TYPE_BUFFER, "%s", name )

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements ( device, _buffer, &memoryRequirements );

    result =
        renderer.TryAllocateMemory ( _memory,
            _offset,
            memoryRequirements,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            "Can't allocate memory (pbr::GPUBuffer::Init)"
        ) &&

        android_vulkan::Renderer::CheckVkResult (
            vkBindBufferMemory ( device, _buffer, _memory, _offset ),
            "pbr::GPUBuffer::Init",
            "Can't bind memory"
        );

    if ( !result ) [[unlikely]]
        return false;

    VkBufferDeviceAddressInfo const bdaInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .pNext = nullptr,
        .buffer = _buffer
    };

    _bda = vkGetBufferDeviceAddress ( device, &bdaInfo );
    return true;
}

bool GPUBuffer::Init ( android_vulkan::Renderer &renderer,
    ResourceHeap &resourceHeap,
    size_t size,
    VkBufferUsageFlags usage,
    char const *name
) noexcept
{
    if ( !Init ( renderer, size, usage, name ) ) [[unlikely]]
        return false;

    auto const idx = resourceHeap.RegisterBuffer ( renderer.GetDevice (), _buffer, static_cast<VkDeviceSize> ( size ) );

    if ( !idx ) [[unlikely]]
        return false;

    _heapIndex = *idx;
    return true;
}

void GPUBuffer::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( _buffer != VK_NULL_HANDLE ) [[likely]]
        vkDestroyBuffer ( renderer.GetDevice (), std::exchange ( _buffer, VK_NULL_HANDLE ), nullptr );

    if ( _memory != VK_NULL_HANDLE ) [[likely]]
    {
        renderer.FreeMemory ( std::exchange ( _memory, VK_NULL_HANDLE ), std::exchange ( _offset, 0U ) );
    }
}

void GPUBuffer::Destroy ( android_vulkan::Renderer &renderer, ResourceHeap &resourceHeap ) noexcept
{
    if ( _heapIndex ) [[likely]]
        resourceHeap.UnregisterResource ( std::exchange ( _heapIndex, 0U ) );

    Destroy ( renderer );
}

} // namespace pbr
