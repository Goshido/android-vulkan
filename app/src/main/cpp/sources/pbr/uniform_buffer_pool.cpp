#include <pbr/uniform_buffer_pool.h>
#include <vulkan_utils.h>


namespace pbr {

constexpr static size_t const VRAM_PER_POOL_MEGABYTES = 64U;
constexpr static size_t const VRAM_PER_POOL_BYTES = VRAM_PER_POOL_MEGABYTES * 1024U * 1024U;

// see https://vulkan.lunarg.com/doc/view/1.1.108.0/mac/chunked_spec/chap18.html#vkCmdUpdateBuffer
constexpr static size_t const UPDATE_BUFFER_MAX_SIZE = 65536U;

//----------------------------------------------------------------------------------------------------------------------

UniformBufferPool::UniformBufferPool ():
    _gpuMemory ( VK_NULL_HANDLE ),
    _index ( 0U ),
    _itemSize ( 0U ),
    _pool {}
{
    // NOTHING
}

VkBuffer UniformBufferPool::Acquire ( VkCommandBuffer commandBuffer, void const* data,
    android_vulkan::Renderer &renderer
)
{
    assert ( _index >= _pool.size () );

    size_t const usedItems = _pool.size ();

    if ( usedItems <= _index && !AllocateItem ( renderer ) )
        return VK_NULL_HANDLE;

    // TODO sync here!

    VkBuffer buffer = _pool[ _index ];
    vkCmdUpdateBuffer ( commandBuffer, buffer, 0U, _itemSize, data );

    ++_index;
    return buffer;
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
    size_t const itemCount = VRAM_PER_POOL_BYTES / itemSize;
    _pool.reserve ( itemCount );

    bool const result = renderer.TryAllocateMemory ( _gpuMemory,
        VRAM_PER_POOL_BYTES,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate GPU memory"
    );

    if ( result )
        return true;

    Destroy ( renderer );
    return false;
}

void UniformBufferPool::Destroy ( android_vulkan::Renderer &renderer )
{
    VkDevice device = renderer.GetDevice ();

    for ( auto const item : _pool )
    {
        vkDestroyBuffer ( device, item, nullptr );
        AV_UNREGISTER_BUFFER ( "UniformBuffer::_pool::item" )
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

    bool const result = renderer.CheckVkResult ( vkCreateBuffer ( device, &bufferInfo, nullptr, &buffer ),
        "UniformBufferPool::AllocateItem",
        "Can't create uniform buffer"
    );

    if ( !result )
        return false;

    AV_REGISTER_BUFFER ( "UniformBuffer::_pool::item" )

    VkDeviceSize const offset = _itemSize * _pool.size ();

    return renderer.CheckVkResult ( vkBindBufferMemory ( device, buffer, _gpuMemory, offset ),
        "UniformBufferPool::AllocateItem",
        "Can't bind uniform buffer memory"
    );
}

} // namespace pbr
