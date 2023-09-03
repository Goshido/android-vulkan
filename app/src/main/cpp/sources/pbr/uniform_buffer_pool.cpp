#include <pbr/uniform_buffer_pool.hpp>
#include <av_assert.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

namespace {

constexpr size_t KILOBYTES_TO_BYTES = 1024U;

// see https://vulkan.lunarg.com/doc/view/1.1.108.0/mac/chunked_spec/chap18.html#vkCmdUpdateBuffer
[[maybe_unused]] constexpr size_t UPDATE_BUFFER_MAX_SIZE = 65536U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

UniformBufferPool::UniformBufferPool ( eUniformPoolSize size ) noexcept:
    _size ( KILOBYTES_TO_BYTES * static_cast<size_t> ( size ) )
{
    // NOTHING
}

VkBuffer UniformBufferPool::Push ( VkCommandBuffer commandBuffer, void const* data, size_t size ) noexcept
{
    AV_ASSERT ( size <= _itemSize )
    AV_ASSERT ( _buffers.size () > _index )

    VkBuffer buffer = _buffers[ _index++ ];
    vkCmdUpdateBuffer ( commandBuffer, buffer, 0U, size, data );
    return buffer;
}

void UniformBufferPool::Reset () noexcept
{
    _index = 0U;
}

size_t UniformBufferPool::GetAvailableItemCount () const noexcept
{
    return _buffers.capacity () - _index;
}

VkBuffer UniformBufferPool::GetBuffer ( size_t bufferIndex ) const noexcept
{
    AV_ASSERT ( bufferIndex < _buffers.size () )
    return _buffers[ bufferIndex ];
}

bool UniformBufferPool::Init ( android_vulkan::Renderer &renderer, size_t itemSize ) noexcept
{
    AV_ASSERT ( itemSize <= renderer.GetMaxUniformBufferRange () )
    AV_ASSERT ( itemSize <= UPDATE_BUFFER_MAX_SIZE )

    VkBufferCreateInfo const bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = static_cast<VkDeviceSize> ( itemSize ),
        .usage = AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ) | AV_VK_FLAG ( VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT ),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    VkMemoryRequirements requirements;

    if ( !ResolveMemoryRequirements ( renderer, requirements, bufferInfo ) )
        return false;

    auto const alignment = static_cast<size_t> ( requirements.alignment );
    size_t const alignMultiplier = ( itemSize + alignment - 1U ) / alignment;
    size_t const alignedBlockSize = alignMultiplier * alignment;
    requirements.size = static_cast<VkDeviceSize> ( alignedBlockSize );

    _index = 0U;
    return AllocateBuffers ( renderer, requirements, _size / alignedBlockSize, bufferInfo );
}

void UniformBufferPool::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    for ( VkBuffer buffer : _buffers )
    {
        vkDestroyBuffer ( device, buffer, nullptr );
        AV_UNREGISTER_BUFFER ( "pbr::UniformBufferPool::_buffer::item" )
    }

    _buffers.clear ();

    if ( _memory == VK_NULL_HANDLE )
        return;

    renderer.FreeMemory ( _memory, _offset );
    _memory = VK_NULL_HANDLE;
    _offset = std::numeric_limits<VkDeviceSize>::max ();
    AV_UNREGISTER_DEVICE_MEMORY ( "pbr::UniformBufferPool::_memory" )
}

bool UniformBufferPool::AllocateBuffers ( android_vulkan::Renderer &renderer,
    VkMemoryRequirements &requirements,
    size_t itemCount,
    VkBufferCreateInfo const &bufferInfo
) noexcept
{
    VkDevice device = renderer.GetDevice ();
    _buffers.reserve ( itemCount );

    VkDeviceSize const alignedBlockSize = requirements.size;
    requirements.size *= static_cast<VkDeviceSize> ( itemCount );

    bool result = renderer.TryAllocateMemory ( _memory,
        _offset,
        requirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate GPU memory (pbr::UniformBufferPool::AllocateBuffers)"
    );

    if ( !result )
        return false;

    AV_REGISTER_DEVICE_MEMORY ( "pbr::UniformBufferPool::_memory" )
    VkDeviceSize offset = _offset;

    for ( size_t i = 0U; i < itemCount; ++i )
    {
        VkBuffer buffer;

        result = android_vulkan::Renderer::CheckVkResult (
            vkCreateBuffer ( device, &bufferInfo, nullptr, &buffer ),
            "pbr::UniformBufferPool::AllocateBuffers",
            "Can't create uniform buffer"
        );

        if ( !result )
            return false;

        AV_REGISTER_BUFFER ( "pbr::UniformBufferPool::_buffer::item" )

        vkGetBufferMemoryRequirements ( device, buffer, &requirements );

        result = android_vulkan::Renderer::CheckVkResult (
            vkBindBufferMemory ( device, buffer, _memory, offset ),
            "pbr::UniformBufferPool::AllocateBuffers",
            "Can't bind uniform buffer memory"
        );

        if ( !result )
            return false;

        offset += alignedBlockSize;
        _buffers.push_back ( buffer );
    }

    _itemSize = static_cast<size_t> ( bufferInfo.size );
    return true;
}

bool UniformBufferPool::ResolveMemoryRequirements ( android_vulkan::Renderer &renderer,
    VkMemoryRequirements &requirements,
    VkBufferCreateInfo const &bufferInfo
) noexcept
{
    VkDevice device = renderer.GetDevice ();
    VkBuffer buffer;

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateBuffer ( device, &bufferInfo, nullptr, &buffer ),
        "pbr::UniformBufferPool::ResolveAlignment",
        "Can't create uniform buffer"
    );

    if ( !result )
        return false;

    vkGetBufferMemoryRequirements ( device, buffer, &requirements );
    vkDestroyBuffer ( device, buffer, nullptr );
    return true;
}

} // namespace pbr
