#include <pbr/uniform_buffer_pool_manager.h>
#include <vulkan_utils.h>


namespace pbr {

UniformBufferPoolManager::UniformBufferPoolManager ( eUniformPoolSize size, VkPipelineStageFlags syncFlags ) noexcept:
    _syncFlags ( syncFlags ),
    _uniformPool ( size )
{
    // NOTHING
}

VkDescriptorSet UniformBufferPoolManager::Acquire () noexcept
{
    VkDescriptorSet set = _descriptorSets[ _uniformReadIndex ];
    _uniformReadIndex = ( _uniformReadIndex + 1U ) % _descriptorSets.size ();
    return set;
}

void UniformBufferPoolManager::Commit () noexcept
{
    _uniformBaseIndex = _uniformWriteIndex;
    _uniformReadIndex = _uniformWriteIndex;
    _uniformWritten = 0U;
}

void UniformBufferPoolManager::IssueSync ( VkDevice device, VkCommandBuffer commandBuffer ) const noexcept
{
    size_t const count = _descriptorSets.size ();
    size_t const idx = _uniformBaseIndex + _uniformWritten;
    size_t const cases[] = { 0U, idx - count };
    size_t const more = cases[ static_cast<size_t> ( idx > count ) ];
    size_t const available = _uniformWritten - more;

    VkBufferMemoryBarrier const* barriers = _barriers.data ();

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        _syncFlags,
        0U,
        0U,
        nullptr,
        static_cast<uint32_t> ( available ),
        barriers + _uniformBaseIndex,
        0U,
        nullptr
    );

    VkWriteDescriptorSet const* writeSets = _writeSets.data ();
    vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( available ), writeSets + _uniformBaseIndex, 0U, nullptr );

    if ( more < 1U )
        return;

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        _syncFlags,
        0U,
        0U,
        nullptr,
        static_cast<uint32_t> ( more ),
        barriers,
        0U,
        nullptr
    );

    vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( more ), writeSets, 0U, nullptr );
}

void UniformBufferPoolManager::Push ( android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer,
    void const *item
) noexcept
{
    VkBuffer buffer = _uniformPool.Acquire ( renderer, commandBuffer, item );
    _bufferInfo[ _uniformWriteIndex ].buffer = buffer;
    _barriers[ _uniformWriteIndex ].buffer = buffer;
    _uniformWriteIndex = ( _uniformWriteIndex + 1U ) % _descriptorSets.size ();

    if ( _uniformWriteIndex == 0U )
        _uniformPool.Reset ();

    ++_uniformWritten;
}

bool UniformBufferPoolManager::Init ( android_vulkan::Renderer &renderer,
    DescriptorSetLayout const &descriptorSetLayout,
    size_t itemSize,
    [[maybe_unused]] char const* name
) noexcept
{
    if ( !_uniformPool.Init ( renderer, itemSize ) )
        return false;

    size_t const setCount = _uniformPool.GetAvailableItemCount ();
    VkDevice device = renderer.GetDevice ();

    VkDescriptorPoolSize const poolSizes[] =
    {
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = static_cast<uint32_t> ( setCount )
        }
    };

    VkDescriptorPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .maxSets = static_cast<uint32_t> ( setCount ),
        .poolSizeCount = static_cast<uint32_t> ( std::size ( poolSizes ) ),
        .pPoolSizes = poolSizes
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "pbr::UniformBufferPoolManager::Init",
        "Can't create descriptor pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_POOL ( name )

    _descriptorSets.resize ( setCount, VK_NULL_HANDLE );
    std::vector<VkDescriptorSetLayout> const layouts ( setCount, descriptorSetLayout.GetLayout () );

    VkDescriptorSetAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _descriptorPool,
        .descriptorSetCount = poolInfo.maxSets,
        .pSetLayouts = layouts.data ()
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &allocateInfo, _descriptorSets.data () ),
        "pbr::UniformBufferPoolManager::Init",
        "Can't allocate descriptor sets"
    );

    if ( !result )
        return false;

    // Initialize all immutable constant fields.

    VkDescriptorBufferInfo const uniform
    {
        .buffer = VK_NULL_HANDLE,
        .offset = 0U,
        .range = static_cast<VkDeviceSize> ( itemSize )
    };

    _bufferInfo.resize ( setCount, uniform );

    constexpr VkWriteDescriptorSet writeSet
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = VK_NULL_HANDLE,
        .dstBinding = 0U,
        .dstArrayElement = 0U,
        .descriptorCount = 1U,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pImageInfo = nullptr,
        .pBufferInfo = nullptr,
        .pTexelBufferView = nullptr
    };

    _writeSets.resize ( setCount, writeSet );

    for ( size_t i = 0U; i < setCount; ++i )
    {
        VkWriteDescriptorSet& uniformWriteSet = _writeSets[ i ];
        uniformWriteSet.dstSet = _descriptorSets[ i ];
        uniformWriteSet.pBufferInfo = &_bufferInfo[ i ];
    }

    VkBufferMemoryBarrier const bufferBarrier
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = VK_NULL_HANDLE,
        .offset = 0U,
        .size = static_cast<VkDeviceSize> ( itemSize )
    };

    _barriers.resize ( setCount, bufferBarrier );

    // Now all what is needed to do is to init "_bufferInfo::buffer". Then to invoke vkUpdateDescriptorSets.
    _uniformBaseIndex = 0U;
    return true;
}

void UniformBufferPoolManager::Destroy ( VkDevice device, [[maybe_unused]] char const *name ) noexcept
{
    if ( _descriptorPool != VK_NULL_HANDLE )
    {
        vkDestroyDescriptorPool ( device, _descriptorPool, nullptr );
        _descriptorPool = VK_NULL_HANDLE;
        AV_UNREGISTER_DESCRIPTOR_POOL ( name )
    }

    auto const clean = [] ( auto &vector ) noexcept {
        vector.clear ();
        vector.shrink_to_fit ();
    };

    clean ( _barriers );
    clean ( _descriptorSets );
    clean ( _bufferInfo );
    clean ( _writeSets );

    _uniformPool.Destroy ( device );
}

} // namespace pbr
