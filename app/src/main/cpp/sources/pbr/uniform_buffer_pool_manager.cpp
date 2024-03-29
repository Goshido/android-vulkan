#include <pbr/uniform_buffer_pool_manager.hpp>
#include <vulkan_utils.hpp>


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

    if ( more < 1U ) [[unlikely]]
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

void UniformBufferPoolManager::Push ( VkCommandBuffer commandBuffer, void const* item, size_t size ) noexcept
{
    _uniformPool.Push ( commandBuffer, item, size );

    VkBufferMemoryBarrier &barrier = _barriers[ _uniformWriteIndex ];
    barrier.size = static_cast<VkDeviceSize> ( size );

    _uniformWriteIndex = ( _uniformWriteIndex + 1U ) % _descriptorSets.size ();

    if ( _uniformWriteIndex == 0U )
        _uniformPool.Reset ();

    ++_uniformWritten;
}

bool UniformBufferPoolManager::Init ( android_vulkan::Renderer &renderer,
    DescriptorSetLayout const &descriptorSetLayout,
    size_t itemSize,
    uint32_t bind,
    char const* name
) noexcept
{
    if ( !_uniformPool.Init ( renderer, itemSize, name ) ) [[unlikely]]
        return false;

    size_t const setCount = _uniformPool.GetAvailableItemCount ();
    VkDevice device = renderer.GetDevice ();

    VkDescriptorPoolSize const poolSizes =
    {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = static_cast<uint32_t> ( setCount )
    };

    VkDescriptorPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .maxSets = static_cast<uint32_t> ( setCount ),
        .poolSizeCount = 1U,
        .pPoolSizes = &poolSizes
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "pbr::UniformBufferPoolManager::Init",
        "Can't create descriptor pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _descriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL, "%s", name )

    _descriptorSets.resize ( setCount, VK_NULL_HANDLE );
    VkDescriptorSet* descriptorSet = _descriptorSets.data ();
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
        vkAllocateDescriptorSets ( device, &allocateInfo, descriptorSet ),
        "pbr::UniformBufferPoolManager::Init",
        "Can't allocate descriptor sets"
    );

    if ( !result ) [[unlikely]]
        return false;

#if defined ( ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS ) ||       \
    defined ( ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION )

    for ( size_t i = 0U; i < setCount; ++i )
    {
        AV_SET_VULKAN_OBJECT_NAME ( device,
            descriptorSet[ i ],
            VK_OBJECT_TYPE_DESCRIPTOR_SET,
            "%s #%zu",
            name,
            i
        )
    }

#endif // ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS || ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION

    // Initialize all immutable constant fields.

    VkDescriptorBufferInfo const uniform
    {
        .buffer = VK_NULL_HANDLE,
        .offset = 0U,
        .range = static_cast<VkDeviceSize> ( itemSize )
    };

    _bufferInfo.resize ( setCount, uniform );

    VkWriteDescriptorSet const writeSet
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = VK_NULL_HANDLE,
        .dstBinding = bind,
        .dstArrayElement = 0U,
        .descriptorCount = 1U,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pImageInfo = nullptr,
        .pBufferInfo = nullptr,
        .pTexelBufferView = nullptr
    };

    _writeSets.resize ( setCount, writeSet );

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

    for ( size_t i = 0U; i < setCount; ++i )
    {
        VkBuffer buffer = _uniformPool.GetBuffer ( i );

        VkDescriptorBufferInfo &bufferInfo = _bufferInfo[ i ];
        bufferInfo.buffer = buffer;
        _barriers[ i ].buffer = buffer;

        VkWriteDescriptorSet &uniformWriteSet = _writeSets[ i ];
        uniformWriteSet.dstSet = descriptorSet[ i ];
        uniformWriteSet.pBufferInfo = &bufferInfo;
    }

    // Now all what is needed to do is to init "_bufferInfo::buffer". Then to invoke vkUpdateDescriptorSets.
    _uniformBaseIndex = 0U;
    return true;
}

void UniformBufferPoolManager::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( _descriptorPool != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyDescriptorPool ( renderer.GetDevice (), _descriptorPool, nullptr );
        _descriptorPool = VK_NULL_HANDLE;
    }

    auto const clean = [] ( auto &vector ) noexcept {
        vector.clear ();
        vector.shrink_to_fit ();
    };

    clean ( _barriers );
    clean ( _descriptorSets );
    clean ( _bufferInfo );
    clean ( _writeSets );

    _uniformPool.Destroy ( renderer );
}

} // namespace pbr
