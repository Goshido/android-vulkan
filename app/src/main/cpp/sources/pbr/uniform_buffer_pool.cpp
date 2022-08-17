#include <pbr/uniform_buffer_pool.h>
#include <vulkan_utils.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace pbr {

constexpr static size_t MEGABYTES_TO_BYTES = 1024U * 1024U;

// see https://vulkan.lunarg.com/doc/view/1.1.108.0/mac/chunked_spec/chap18.html#vkCmdUpdateBuffer
[[maybe_unused]] constexpr static size_t UPDATE_BUFFER_MAX_SIZE = 65536U;

constexpr static VkBufferUsageFlags USAGE = AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ) |
    AV_VK_FLAG ( VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT );

//----------------------------------------------------------------------------------------------------------------------

UniformBufferPool::UniformBufferPool ( eUniformPoolSize size ) noexcept:
    _size ( MEGABYTES_TO_BYTES * static_cast<size_t> ( size ) )
{
    // NOTHING
}

VkBuffer UniformBufferPool::Push ( VkCommandBuffer commandBuffer, void const* data, size_t size ) noexcept
{
    assert ( _index < _pool.capacity () );
    assert ( size <= _itemSize );
    assert ( _pool.size () > _index );

    VkBuffer buffer = _pool[ _index++ ];
    vkCmdUpdateBuffer ( commandBuffer, buffer, 0U, size, data );
    return buffer;
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
    return _pool[ bufferIndex ];
}

bool UniformBufferPool::Init ( android_vulkan::Renderer &renderer, size_t itemSize ) noexcept
{
    assert ( itemSize <= renderer.GetMaxUniformBufferRange () );
    assert ( itemSize <= UPDATE_BUFFER_MAX_SIZE );

    size_t alignment = 0U;

    if ( !ResolveAlignment ( renderer, alignment, itemSize ) )
        return false;

    size_t const blocks = ( itemSize + alignment - 1U ) / alignment;
    _gpuSpecificItemOffset = reinterpret_cast<VkDeviceSize> ( blocks * alignment );

    _itemSize = static_cast<VkDeviceSize> ( itemSize );
    size_t const itemCount = _size / static_cast<size_t> ( _gpuSpecificItemOffset );
    _pool.reserve ( itemCount );

    // Adjust "_size" to real GPU buffer size which will be requested.
    _size = itemCount * static_cast<size_t> ( _gpuSpecificItemOffset );

    bool const result = renderer.TryAllocateMemory ( _gpuMemory,
        _size,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate GPU memory (pbr::UniformBufferPool::Init)"
    );

    if ( !result )
    {
        Destroy ( renderer.GetDevice () );
        return false;
    }

    AV_REGISTER_DEVICE_MEMORY ( "pbr::UniformBufferPool::_gpuMemory" )

    _index = 0U;
    return AllocateBuffers ( renderer, itemCount );
}

void UniformBufferPool::Destroy ( VkDevice device ) noexcept
{
    for ( auto item : _pool )
    {
        vkDestroyBuffer ( device, item, nullptr );
        AV_UNREGISTER_BUFFER ( "pbr::UniformBufferPool::_pool::item" )
    }

    _pool.clear ();

    if ( _gpuMemory == VK_NULL_HANDLE )
        return;

    vkFreeMemory ( device, _gpuMemory, nullptr );
    _gpuMemory = VK_NULL_HANDLE;
    AV_UNREGISTER_DEVICE_MEMORY ( "pbr::UniformBufferPool::_gpuMemory" )
}

bool UniformBufferPool::AllocateBuffers ( android_vulkan::Renderer &renderer, size_t itemCount ) noexcept
{
    VkBufferCreateInfo const bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = _itemSize,
        .usage = USAGE,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    VkDevice device = renderer.GetDevice ();

    for ( size_t i = 0U; i < itemCount; ++i )
    {
        VkBuffer buffer;

        bool result = android_vulkan::Renderer::CheckVkResult (
            vkCreateBuffer ( device, &bufferInfo, nullptr, &buffer ),
            "pbr::UniformBufferPool::AllocateBuffers",
            "Can't create uniform buffer"
        );

        if ( !result )
            return false;

        AV_REGISTER_BUFFER ( "pbr::UniformBufferPool::_pool::item" )

        VkMemoryRequirements requirements;
        vkGetBufferMemoryRequirements ( device, buffer, &requirements );

        result = android_vulkan::Renderer::CheckVkResult (
            vkBindBufferMemory ( device,
                buffer,
                _gpuMemory,
                static_cast<VkDeviceSize> ( _pool.size () * _gpuSpecificItemOffset )
            ),

            "pbr::UniformBufferPool::AllocateBuffers",
            "Can't bind uniform buffer memory"
        );

        if ( !result )
            return false;

        _pool.push_back ( buffer );
    }

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
    VkBuffer buffer = VK_NULL_HANDLE;

    bool result = android_vulkan::Renderer::CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &buffer ),
        "pbr::UniformBufferPool::ResolveAlignment",
        "Can't create uniform buffer"
    );

    if ( !result )
        return false;

    AV_REGISTER_BUFFER ( "pbr::UniformBufferPool::buffer" )

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements ( device, buffer, &requirements );
    alignment = static_cast<size_t> ( requirements.alignment );

    vkDestroyBuffer ( device, buffer, nullptr );
    AV_UNREGISTER_BUFFER ( "pbr::UniformBufferPool::buffer" )
    return true;
}

} // namespace pbr
