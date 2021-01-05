#include <pbr/uniform_buffer_pool.h>
#include <vulkan_utils.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace pbr {

constexpr static size_t const MEGABYTES_TO_BYTES = 1024U * 1024U;

// see https://vulkan.lunarg.com/doc/view/1.1.108.0/mac/chunked_spec/chap18.html#vkCmdUpdateBuffer
[[maybe_unused]] constexpr static size_t const UPDATE_BUFFER_MAX_SIZE = 65536U;

//----------------------------------------------------------------------------------------------------------------------

UniformBufferPool::UniformBufferPool ( eUniformPoolSize size ) noexcept:
    _gpuMemory ( VK_NULL_HANDLE ),
    _index ( 0U ),
    _itemSize ( 0U ),
    _pool {},
    _size ( MEGABYTES_TO_BYTES * static_cast<size_t> ( size ) )
{
    // NOTHING
}

VkBuffer UniformBufferPool::Acquire ( VkCommandBuffer commandBuffer,
    void const* data,
    VkPipelineStageFlags targetStages,
    android_vulkan::Renderer &renderer
)
{
    assert ( _index < _pool.capacity () );

    size_t const usedItems = _pool.size ();

    if ( usedItems <= _index && !AllocateItem ( renderer ) )
        return VK_NULL_HANDLE;

    VkBufferMemoryBarrier barrier;
    barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    barrier.pNext = nullptr;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT;
    barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.buffer = _pool[ _index ];
    barrier.offset = 0U;
    barrier.size = _itemSize;

    vkCmdUpdateBuffer ( commandBuffer, barrier.buffer, 0U, _itemSize, data );

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        targetStages,
        0U,
        0U,
        nullptr,
        1U,
        &barrier,
        0U,
        nullptr
    );

    ++_index;
    return barrier.buffer;
}

void UniformBufferPool::Reset ()
{
    _index = 0U;
}

size_t UniformBufferPool::GetItemCount () const
{
    return _pool.capacity ();
}

bool UniformBufferPool::Init ( size_t itemSize, android_vulkan::Renderer &renderer )
{
    assert ( itemSize <= renderer.GetMaxUniformBufferRange () );
    assert ( itemSize <= UPDATE_BUFFER_MAX_SIZE );

    _itemSize = static_cast<uint32_t> ( itemSize );
    size_t const itemCount = _size / itemSize;
    _pool.reserve ( itemCount );

    bool const result = renderer.TryAllocateMemory ( _gpuMemory,
        itemCount * itemSize,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate GPU memory"
    );

    if ( result )
    {
        AV_REGISTER_DEVICE_MEMORY ( "UniformBufferPool::_gpuMemory" )
        return true;
    }

    Destroy ( renderer );
    return false;
}

void UniformBufferPool::Destroy ( android_vulkan::Renderer &renderer )
{
    VkDevice device = renderer.GetDevice ();

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
    VkBufferCreateInfo bufferInfo;
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.flags = 0U;
    bufferInfo.size = _itemSize;

    bufferInfo.usage =
        AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT );

    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 0U;
    bufferInfo.pQueueFamilyIndices = nullptr;

    VkDevice device = renderer.GetDevice ();
    VkBuffer buffer = VK_NULL_HANDLE;

    bool result = android_vulkan::Renderer::CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &buffer ),
        "UniformBufferPool::AllocateItem",
        "Can't create uniform buffer"
    );

    if ( !result )
        return false;

    AV_REGISTER_BUFFER ( "UniformBufferPool::_pool::item" )

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements ( device, buffer, &requirements );

    assert ( ( _pool.size () + 1U ) * requirements.size <= _size );

    result = android_vulkan::Renderer::CheckVkResult (
        vkBindBufferMemory ( device,
            buffer,
            _gpuMemory,
            static_cast<VkDeviceSize> ( _pool.size () * requirements.size )
        ),

        "UniformBufferPool::AllocateItem",
        "Can't bind uniform buffer memory"
    );

    if ( !result )
        return false;

    _pool.push_back ( buffer );
    return true;
}

} // namespace pbr
