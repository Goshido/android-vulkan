#include <precompiled_headers.hpp>
#include <pbr/uniform_pool.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

UniformPool::UniformPool ( eUniformSize size, VkPipelineStageFlags syncFlags ) noexcept:
    _syncFlags ( syncFlags ),
    _uniformBuffer ( size )
{
    // NOTHING
}

VkDescriptorSet UniformPool::Acquire () noexcept
{
    return _descriptorSets[ std::exchange ( _readIndex, ( _readIndex + 1U ) % _descriptorSets.size () ) ];
}

void UniformPool::Commit () noexcept
{
    _baseIndex = _writeIndex;
    _readIndex = _writeIndex;
    _written = 0U;
}

void UniformPool::IssueSync ( VkDevice device, VkCommandBuffer commandBuffer ) const noexcept
{
    size_t const count = _descriptorSets.size ();
    size_t const idx = _baseIndex + _written;
    size_t const cases[] = { 0U, idx - count };
    size_t const more = cases[ static_cast<size_t> ( idx > count ) ];
    size_t const available = _written - more;

    VkBufferMemoryBarrier const* barriers = _barriers.data ();

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        _syncFlags,
        0U,
        0U,
        nullptr,
        static_cast<uint32_t> ( available ),
        barriers + _baseIndex,
        0U,
        nullptr
    );

    VkWriteDescriptorSet const* writeSets = _writeSets.data ();
    vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( available ), writeSets + _baseIndex, 0U, nullptr );

    if ( more < 1U ) [[likely]]
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

void UniformPool::Push ( VkCommandBuffer commandBuffer, void const* item, size_t size ) noexcept
{
    _uniformBuffer.Push ( commandBuffer, item, size );

    _barriers[ std::exchange ( _writeIndex, ( _writeIndex + 1U ) % _descriptorSets.size () ) ].size =
        static_cast<VkDeviceSize> ( size );

    if ( _writeIndex == 0U ) [[unlikely]]
        _uniformBuffer.Reset ();

    ++_written;
}

bool UniformPool::Init ( android_vulkan::Renderer &renderer,
    DescriptorSetLayout const &descriptorSetLayout,
    size_t itemSize,
    uint32_t bind,
    [[maybe_unused]] char const* name
) noexcept
{
    if ( !_uniformBuffer.Init ( renderer, itemSize, name ) ) [[unlikely]]
        return false;

    size_t const setCount = _uniformBuffer.GetAvailableItemCount ();
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
        "pbr::UniformPool::Init",
        "Can't create descriptor pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _descriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL, "%s", name )

    _descriptorSets.resize ( setCount, VK_NULL_HANDLE );
    VkDescriptorSet* descriptorSets = _descriptorSets.data ();
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
        vkAllocateDescriptorSets ( device, &allocateInfo, descriptorSets ),
        "pbr::UniformPool::Init",
        "Can't allocate descriptor sets"
    );

    if ( !result ) [[unlikely]]
        return false;

#if defined ( AV_ENABLE_VVL ) || defined ( AV_ENABLE_RENDERDOC )

    for ( size_t i = 0U; i < setCount; ++i )
    {
        AV_SET_VULKAN_OBJECT_NAME ( device,
            descriptorSets[ i ],
            VK_OBJECT_TYPE_DESCRIPTOR_SET,
            "%s #%zu",
            name,
            i
        )
    }

#endif // AV_ENABLE_VVL || AV_ENABLE_RENDERDOC

    // Initialize all immutable constant fields.

    constexpr VkDescriptorBufferInfo bufferTemplate
    {
        .buffer = VK_NULL_HANDLE,
        .offset = 0U,
        .range = VK_WHOLE_SIZE
    };

    _bufferInfo.resize ( setCount, bufferTemplate );

    _writeSets.resize ( setCount,
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
        }
    );

    constexpr VkBufferMemoryBarrier bufferBarrierTemplate
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = VK_NULL_HANDLE,
        .offset = 0U,
        .size = 0U
    };

    _barriers.resize ( setCount, bufferBarrierTemplate );

    for ( size_t i = 0U; i < setCount; ++i )
    {
        VkBuffer buffer = _uniformBuffer.GetBuffer ( i );

        VkDescriptorBufferInfo &bufferInfo = _bufferInfo[ i ];
        bufferInfo.buffer = buffer;
        _barriers[ i ].buffer = buffer;

        VkWriteDescriptorSet &uniformWriteSet = _writeSets[ i ];
        uniformWriteSet.dstSet = descriptorSets[ i ];
        uniformWriteSet.pBufferInfo = &bufferInfo;
    }

    // Now all what is needed to do is to init "_bufferInfo::buffer". Then to invoke vkUpdateDescriptorSets.
    _baseIndex = 0U;
    return true;
}

void UniformPool::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( _descriptorPool != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyDescriptorPool ( renderer.GetDevice (), _descriptorPool, nullptr );
        _descriptorPool = VK_NULL_HANDLE;
    }

    constexpr auto clean = [] ( auto &vector ) noexcept {
        vector.clear ();
        vector.shrink_to_fit ();
    };

    clean ( _barriers );
    clean ( _descriptorSets );
    clean ( _bufferInfo );
    clean ( _writeSets );

    _uniformBuffer.Destroy ( renderer );
}

} // namespace pbr
