#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <pbr/uma_sparse_uniform_buffer.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

namespace {

constexpr size_t KILOBYTES_TO_BYTES = 1024U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

size_t UMASparseUniformBuffer::GetAvailableItemCount () const noexcept
{
    return _size;
}

void UMASparseUniformBuffer::Push ( void const* item, size_t size ) noexcept
{
    std::memcpy ( _data + ( _index++ ) * _bufferInfo._stepSize, item, size );
}

UMASparseUniformBuffer::BufferInfo const &UMASparseUniformBuffer::GetBufferInfo () noexcept
{
    return _bufferInfo;
}

void UMASparseUniformBuffer::Reset () noexcept
{
    _index = 0U;
}

bool UMASparseUniformBuffer::Init ( android_vulkan::Renderer &renderer,
    eUniformPoolSize size,
    size_t itemSize,
    [[maybe_unused]] char const* name
) noexcept
{
    AV_ASSERT ( itemSize > 0U )
    AV_ASSERT ( itemSize <= renderer.GetMaxUniformBufferRange () )

    constexpr VkBufferUsageFlags usageFlags = AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT );

    size_t const bufferSize = KILOBYTES_TO_BYTES * static_cast<size_t> ( size );

    VkBufferCreateInfo const bufferCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = static_cast<VkDeviceSize> ( bufferSize ),
        .usage = usageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateBuffer ( device, &bufferCreateInfo, nullptr, &_bufferInfo._buffer ),
        "pbr::UMASparseUniformBuffer::Init",
        "Can't create uniform buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements ( device, _bufferInfo._buffer, &requirements );

    constexpr VkMemoryPropertyFlags memoryFlags = AV_VK_FLAG ( VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ) |
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ) |
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_CACHED_BIT );

    result = renderer.TryAllocateMemory ( _bufferInfo._memory,
        _bufferInfo._offset,
        requirements,
        memoryFlags,
        "Can't allocate GPU memory (pbr::UMASparseUniformBuffer::Init)"
    );

    if ( !result ) [[unlikely]]
        return false;

    result = android_vulkan::Renderer::CheckVkResult (
        vkBindBufferMemory ( device, _bufferInfo._buffer, _bufferInfo._memory, _bufferInfo._offset ),
        "pbr::UMASparseUniformBuffer::Init",
        "Can't memory"
    );

    if ( !result ) [[unlikely]]
        return false;

    void* ptr;

    result = renderer.MapMemory ( ptr,
        _bufferInfo._memory,
        _bufferInfo._offset,
        "pbr::UMASparseUniformBuffer::Init", "Can't map memory"
    );

    if ( !result ) [[unlikely]]
    {
        [[unlikely]]
        return false;
    }

    _data = static_cast<uint8_t*> ( ptr );
    size_t const nonCoherentAtomSize = renderer.GetNonCoherentAtomSize ();
    size_t const minUniformBufferOffsetAlignment = renderer.GetMinUniformBufferOffsetAlignment ();
    size_t alignment;

    if ( nonCoherentAtomSize != 0U && minUniformBufferOffsetAlignment != 0U ) [[likely]]
    {
        alignment = std::lcm ( nonCoherentAtomSize, minUniformBufferOffsetAlignment );
    }
    else
    {
        size_t const cases[] = { std::max ( nonCoherentAtomSize, minUniformBufferOffsetAlignment ), itemSize };
        alignment = cases[ static_cast<size_t> ( cases[ 0U ] == 0U ) ];
    }

    size_t const alpha = itemSize - 1U;
    _bufferInfo._stepSize = alpha + alignment - ( alpha % alignment );
    _size = bufferSize / _bufferInfo._stepSize;

    return true;
}

void UMASparseUniformBuffer::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( _bufferInfo._buffer != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyBuffer ( renderer.GetDevice (), _bufferInfo._buffer, nullptr );
        _bufferInfo._buffer = VK_NULL_HANDLE;
    }

    if ( _bufferInfo._memory == VK_NULL_HANDLE ) [[unlikely]]
        return;

    if ( _data ) [[likely]]
    {
        renderer.UnmapMemory ( _bufferInfo._memory );
        _data = nullptr;
    }

    renderer.FreeMemory ( _bufferInfo._memory, _bufferInfo._offset );
    _bufferInfo._memory = VK_NULL_HANDLE;
    _bufferInfo._offset = 0U;
}

} // namespace pbr
