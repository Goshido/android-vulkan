#include <pbr/uniform_buffer_pool.h>
#include <vulkan_utils.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace pbr {

constexpr static size_t const MEGABYTES_TO_BYTES = 1024U * 1024U;

// see https://vulkan.lunarg.com/doc/view/1.1.108.0/mac/chunked_spec/chap18.html#vkCmdUpdateBuffer
[[maybe_unused]] constexpr static size_t UPDATE_BUFFER_MAX_SIZE = 65536U;

constexpr static VkBufferUsageFlags USAGE = AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ) |
    AV_VK_FLAG ( VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT );

//----------------------------------------------------------------------------------------------------------------------

UniformBufferPool::UniformBufferPool ( eUniformPoolSize size ) noexcept:
    _barrier {},
    _bufferInfo {},
    _gpuMemory ( VK_NULL_HANDLE ),
    _gpuSpecificItemOffset ( 0U ),
    _index ( 0U ),
    _itemSize ( 0U ),
    _pool {},
    _size ( MEGABYTES_TO_BYTES * static_cast<size_t> ( size ) )
{
    // NOTHING
}

VkBuffer UniformBufferPool::Acquire ( android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer,
    void const* data,
    VkPipelineStageFlags targetStages
)
{
    assert ( _index < _pool.capacity () );

    if ( _pool.size () <= _index && !AllocateItem ( renderer ) )
        return VK_NULL_HANDLE;

    _barrier.buffer = _pool[ _index++ ];
    vkCmdUpdateBuffer ( commandBuffer, _barrier.buffer, 0U, _itemSize, data );

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        targetStages,
        0U,
        0U,
        nullptr,
        1U,
        &_barrier,
        0U,
        nullptr
    );

    return _barrier.buffer;
}

void UniformBufferPool::Reset ()
{
    _index = 0U;
}

size_t UniformBufferPool::GetItemCount () const
{
    return _pool.capacity ();
}

bool UniformBufferPool::Init ( android_vulkan::Renderer &renderer, size_t itemSize )
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

    AV_REGISTER_DEVICE_MEMORY ( "UniformBufferPool::_gpuMemory" )

    _bufferInfo =
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

    _barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    _barrier.pNext = nullptr;
    _barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    _barrier.dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT;
    _barrier.srcQueueFamilyIndex = _barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    _barrier.offset = 0U;
    _barrier.size = _itemSize;

    return true;
}

void UniformBufferPool::Destroy ( VkDevice device )
{
    for ( auto item : _pool )
    {
        vkDestroyBuffer ( device, item, nullptr );
        AV_UNREGISTER_BUFFER ( "UniformBufferPool::_pool::item" )
    }

    _pool.clear ();

    if ( _gpuMemory == VK_NULL_HANDLE )
        return;

    vkFreeMemory ( device, _gpuMemory, nullptr );
    _gpuMemory = VK_NULL_HANDLE;
    AV_UNREGISTER_DEVICE_MEMORY ( "UniformBufferPool::_gpuMemory" )
}

bool UniformBufferPool::AllocateItem ( android_vulkan::Renderer &renderer )
{
    VkDevice device = renderer.GetDevice ();
    VkBuffer buffer = VK_NULL_HANDLE;

    bool result = android_vulkan::Renderer::CheckVkResult ( vkCreateBuffer ( device, &_bufferInfo, nullptr, &buffer ),
        "pbr::UniformBufferPool::AllocateItem",
        "Can't create uniform buffer"
    );

    if ( !result )
        return false;

    AV_REGISTER_BUFFER ( "UniformBufferPool::_pool::item" )

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements ( device, buffer, &requirements );

    assert ( ( _pool.size () + 1U ) * _gpuSpecificItemOffset <= _size );

    result = android_vulkan::Renderer::CheckVkResult (
        vkBindBufferMemory ( device,
            buffer,
            _gpuMemory,
            static_cast<VkDeviceSize> ( _pool.size () * _gpuSpecificItemOffset )
        ),

        "pbr::UniformBufferPool::AllocateItem",
        "Can't bind uniform buffer memory"
    );

    if ( !result )
        return false;

    _pool.push_back ( buffer );
    return true;
}

bool UniformBufferPool::ResolveAlignment ( android_vulkan::Renderer &renderer,
    size_t &alignment,
    size_t itemSize
)
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

    AV_REGISTER_BUFFER ( "UniformBufferPool::buffer" )

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements ( device, buffer, &requirements );
    alignment = static_cast<size_t> ( requirements.alignment );

    vkDestroyBuffer ( device, buffer, nullptr );
    AV_UNREGISTER_BUFFER ( "UniformBufferPool::buffer" )
    return true;
}

} // namespace pbr
