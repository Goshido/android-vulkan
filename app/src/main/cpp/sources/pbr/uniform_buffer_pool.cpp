#include <pbr/uniform_buffer_pool.h>
#include <vulkan_utils.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace pbr {

constexpr static size_t KILOBYTES_TO_BYTES = 1024U;

// see https://vulkan.lunarg.com/doc/view/1.1.108.0/mac/chunked_spec/chap18.html#vkCmdUpdateBuffer
[[maybe_unused]] constexpr static size_t UPDATE_BUFFER_MAX_SIZE = 65536U;

constexpr static VkBufferUsageFlags USAGE = AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ) |
    AV_VK_FLAG ( VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT );

//----------------------------------------------------------------------------------------------------------------------

UniformBufferPool::UniformBufferPool ( eUniformPoolSize size ) noexcept:
    _size ( KILOBYTES_TO_BYTES * static_cast<size_t> ( size ) )
{
    // NOTHING
}

VkBuffer UniformBufferPool::Push ( VkCommandBuffer commandBuffer, void const* data, size_t size ) noexcept
{
    assert ( _index < _pool.capacity () );
    assert ( size <= _itemSize );
    assert ( _pool.size () > _index );

    Item& item = _pool[ _index++ ];
    vkCmdUpdateBuffer ( commandBuffer, item._buffer, 0U, size, data );
    return item._buffer;
}

void UniformBufferPool::Reset () noexcept
{
    _index = 0U;
}

size_t UniformBufferPool::GetAvailableItemCount () const noexcept
{
    return _pool.capacity () - _index;
}

VkBuffer UniformBufferPool::GetBuffer ( size_t bufferIndex ) const noexcept
{
    assert ( bufferIndex < _pool.size () );
    return _pool[ bufferIndex ]._buffer;
}

bool UniformBufferPool::Init ( android_vulkan::Renderer &renderer, size_t itemSize ) noexcept
{
    assert ( itemSize <= renderer.GetMaxUniformBufferRange () );
    assert ( itemSize <= UPDATE_BUFFER_MAX_SIZE );

    size_t alignment = 0U;

    if ( !ResolveAlignment ( renderer, alignment, itemSize ) )
        return false;

    size_t const alignMultiplier = ( itemSize + alignment - 1U ) / alignment;
    size_t const alignedBlockSize = alignMultiplier * alignment;

    _index = 0U;
    return AllocateBuffers ( renderer, _size / alignedBlockSize, itemSize );
}

void UniformBufferPool::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    for ( auto const& item : _pool )
    {
        vkDestroyBuffer ( device, item._buffer, nullptr );
        AV_UNREGISTER_BUFFER ( "pbr::UniformBufferPool::_pool::item" )

        renderer.FreeMemory ( item._memory, item._offset );
        AV_UNREGISTER_DEVICE_MEMORY ( "pbr::UniformBufferPool::_pool::_memory" )
    }

    _pool.clear ();
}

bool UniformBufferPool::AllocateBuffers ( android_vulkan::Renderer &renderer,
    size_t itemCount,
    size_t itemSize
) noexcept
{
    VkBufferCreateInfo const bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = static_cast<VkDeviceSize> ( itemSize ),
        .usage = USAGE,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    VkDevice device = renderer.GetDevice ();
    _pool.reserve ( itemCount );

    Item item {};
    VkMemoryRequirements requirements;

    for ( size_t i = 0U; i < itemCount; ++i )
    {
        bool result = android_vulkan::Renderer::CheckVkResult (
            vkCreateBuffer ( device, &bufferInfo, nullptr, &item._buffer ),
            "pbr::UniformBufferPool::AllocateBuffers",
            "Can't create uniform buffer"
        );

        if ( !result )
            return false;

        AV_REGISTER_BUFFER ( "pbr::UniformBufferPool::_pool::item" )

        vkGetBufferMemoryRequirements ( device, item._buffer, &requirements );

        result = renderer.TryAllocateMemory ( item._memory,
            item._offset,
            requirements,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            "Can't allocate GPU memory (pbr::UniformBufferPool::AllocateBuffers)"
        );

        if ( !result )
            return false;

        AV_REGISTER_DEVICE_MEMORY ( "pbr::UniformBufferPool::_pool::_memory" )

        result = android_vulkan::Renderer::CheckVkResult (
            vkBindBufferMemory ( device, item._buffer, item._memory, item._offset ),
            "pbr::UniformBufferPool::AllocateBuffers",
            "Can't bind uniform buffer memory"
        );

        if ( !result )
            return false;

        _pool.push_back ( item );
    }

    _itemSize = itemSize;
    return true;
}

bool UniformBufferPool::ResolveAlignment ( android_vulkan::Renderer &renderer,
    size_t &alignment,
    size_t itemSize
) noexcept
{
    VkBufferCreateInfo const bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = static_cast<VkDeviceSize> ( itemSize ),
        .usage = USAGE,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    VkDevice device = renderer.GetDevice ();
    VkBuffer buffer;

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateBuffer ( device, &bufferInfo, nullptr, &buffer ),
        "pbr::UniformBufferPool::ResolveAlignment",
        "Can't create uniform buffer"
    );

    if ( !result )
        return false;

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements ( device, buffer, &requirements );
    alignment = static_cast<size_t> ( requirements.alignment );

    vkDestroyBuffer ( device, buffer, nullptr );
    return true;
}

} // namespace pbr
