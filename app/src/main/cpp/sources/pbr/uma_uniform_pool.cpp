#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <pbr/uma_uniform_pool.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

VkDescriptorSet UMAUniformPool::Acquire () noexcept
{
    VkDescriptorSet set = _sets[ _readIndex ];
    _readIndex = ( _readIndex + 1U ) % _sets.size ();
    return set;
}

void UMAUniformPool::Commit () noexcept
{
    if ( !_written )
        return;

    _readIndex = _writeIndex;
    _written = false;
    auto &[mainRange, overflowRange] = _ranges;

    if ( mainRange.size + _stepSize <= _size ) [[likely]]
    {
        mainRange.offset += mainRange.size;
    }
    else
    {
        mainRange.offset = overflowRange.offset;
        _rangeWritten = 0U;
    }

    mainRange.size = 0U;
    overflowRange.size = 0U;
    _rangeIndex = 0U;
}

size_t UMAUniformPool::GetAvailableItemCount () const noexcept
{
    return _sets.size ();
}

bool UMAUniformPool::IssueSync ( VkDevice device ) const noexcept
{
    if ( !_written )
        return true;

    return android_vulkan::Renderer::CheckVkResult (
        vkFlushMappedMemoryRanges ( device, static_cast<uint32_t> ( _rangeIndex + 1U ), _ranges ),
        "UMAUniformPool::IssueSync",
        "Can't flush memory ranges"
    );
}

void UMAUniformPool::Push ( void const* item ) noexcept
{
    VkMappedMemoryRange* range = _ranges + _rangeIndex;
    size_t offset = std::exchange ( _rangeWritten, _rangeWritten + _stepSize );

    if ( _rangeWritten > _size ) [[unlikely]]
    {
        _rangeIndex = 1U;
        _rangeWritten = _stepSize;
        range = _ranges + 1U;
        offset = 0U;
    }

    std::memcpy ( _data + offset, item, _itemSize );
    range->size += _stepSize;

    _writeIndex = ( _writeIndex + 1U ) % _sets.size ();
    _written = true;
}

bool UMAUniformPool::Init ( android_vulkan::Renderer &renderer,
    DescriptorSetLayout const &descriptorSetLayout,
    eUniformPoolSize size,
    size_t itemSize,
    uint32_t bind,
    [[maybe_unused]] char const* name
) noexcept
{
    AV_ASSERT ( itemSize > 0U )
    AV_ASSERT ( itemSize <= renderer.GetMaxUniformBufferRange () )

    _itemSize = itemSize;

    constexpr size_t kilobytesToBytesShift = 10U;
    _size = static_cast<size_t> ( size ) << kilobytesToBytesShift;

    VkBufferCreateInfo const bufferCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = static_cast<VkDeviceSize> ( _size ),
        .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateBuffer ( device, &bufferCreateInfo, nullptr, &_buffer ),
        "pbr::UMAUniformPool::Init",
        "Can't create uniform buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements ( device, _buffer, &requirements );

    constexpr VkMemoryPropertyFlags memoryFlags = AV_VK_FLAG ( VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ) |
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ) |
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_CACHED_BIT );

    auto &[mainRange, overflowRange] = _ranges;

    overflowRange =
    {
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .pNext = nullptr,
        .memory = VK_NULL_HANDLE,
        .offset = 0U,
        .size = 0U
    };

    VkDeviceMemory &memory = overflowRange.memory;
    VkDeviceSize &offset = overflowRange.offset;

    result = renderer.TryAllocateMemory ( memory,
        offset,
        requirements,
        memoryFlags,
        "Can't allocate GPU memory (pbr::UMAUniformPool::Init)"
    );

    if ( !result ) [[unlikely]]
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( vkBindBufferMemory ( device, _buffer, memory, offset ),
        "pbr::UMAUniformPool::Init",
        "Can't bind memory"
    );

    if ( !result ) [[unlikely]]
        return false;

    void* ptr;

    if ( !renderer.MapMemory ( ptr, memory, offset, "pbr::UMAUniformPool::Init", "Can't map memory" ) )
    {
        [[unlikely]]
        return false;
    }

    mainRange = overflowRange;
    _data = static_cast<uint8_t*> ( ptr );
    size_t nonCoherentAtomSize = renderer.GetNonCoherentAtomSize ();
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
    _stepSize = alpha + alignment - ( alpha % alignment );
    size_t const items = _size / _stepSize;

    VkDescriptorPoolSize const poolSizes =
    {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = static_cast<uint32_t> ( items )
    };

    VkDescriptorPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .maxSets = static_cast<uint32_t> ( items ),
        .poolSizeCount = 1U,
        .pPoolSizes = &poolSizes
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_pool ),
        "pbr::UMAUniformPool::Init",
        "Can't create descriptor pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _pool, VK_OBJECT_TYPE_DESCRIPTOR_POOL, "%s", name )

    _sets.resize ( items );
    VkDescriptorSet* sets = _sets.data ();
    std::vector<VkDescriptorSetLayout> const layouts ( items, descriptorSetLayout.GetLayout () );

    VkDescriptorSetAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _pool,
        .descriptorSetCount = poolInfo.maxSets,
        .pSetLayouts = layouts.data ()
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &allocateInfo, sets ),
        "pbr::UMAUniformPool::Init",
        "Can't allocate descriptor sets"
    );

    if ( !result ) [[unlikely]]
        return false;

#if defined ( AV_ENABLE_VVL ) || defined ( AV_ENABLE_RENDERDOC )

    for ( size_t i = 0U; i < items; ++i )
    {
        AV_SET_VULKAN_OBJECT_NAME ( device,
            sets[ i ],
            VK_OBJECT_TYPE_DESCRIPTOR_SET,
            "%s #%zu",
            name,
            i
        )
    }

#endif // AV_ENABLE_VVL || AV_ENABLE_RENDERDOC

    std::vector<VkDescriptorBufferInfo> bufferInfo ( items,
        {
            .buffer = _buffer,
            .offset = 0U,
            .range = static_cast<VkDeviceSize> ( _stepSize )
        }
    );

    VkDescriptorBufferInfo* bi = bufferInfo.data ();

    std::vector<VkWriteDescriptorSet> writeSets ( items,
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

    VkWriteDescriptorSet* writes = writeSets.data ();
    auto const inc = static_cast<VkDeviceSize> ( _stepSize );

    for ( VkDeviceSize i = 0U; i < bufferCreateInfo.size; i += inc )
    {
        bi->offset = i;

        writes->dstSet = *sets++;
        ( writes++ )->pBufferInfo = bi++;
    }

    vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( items ), writeSets.data (), 0U, nullptr );
    return true;
}

void UMAUniformPool::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    _sets.clear ();
    _sets.shrink_to_fit ();

    VkDevice device = renderer.GetDevice ();

    if ( _pool != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyDescriptorPool ( device, _pool, nullptr );
        _pool = VK_NULL_HANDLE;
    }

    if ( _buffer != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyBuffer ( device, _buffer, nullptr );
        _buffer = VK_NULL_HANDLE;
    }

    auto &[mainRange, overflowRange] = _ranges;

    if ( overflowRange.memory == VK_NULL_HANDLE ) [[unlikely]]
        return;

    if ( _data ) [[likely]]
    {
        renderer.UnmapMemory ( overflowRange.memory );
        _data = nullptr;
    }

    renderer.FreeMemory ( overflowRange.memory, overflowRange.offset );
    overflowRange.memory = VK_NULL_HANDLE;
    overflowRange.offset = 0U;
    mainRange = overflowRange;
}

} // namespace pbr
