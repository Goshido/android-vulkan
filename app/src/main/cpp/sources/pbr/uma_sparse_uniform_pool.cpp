#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <pbr/uma_sparse_uniform_pool.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

namespace {

constexpr size_t KILOBYTES_TO_BYTES = 1024U;

} // end of anonymous namespace

VkDescriptorSet UMASparseUniformPool::Acquire () noexcept
{
    VkDescriptorSet set = _sets[ _readIndex ];
    _readIndex = ( _readIndex + 1U ) % _sets.size ();
    return set;
}

void UMASparseUniformPool::Commit () noexcept
{
    _baseIndex = _writeIndex;
    _readIndex = _writeIndex;
    _written = 0U;
}

bool UMASparseUniformPool::IssueSync ( VkDevice device ) const noexcept
{
    size_t const count = _ranges.size ();
    size_t const idx = _baseIndex + _written;
    size_t const cases[] = { 0U, idx - count };
    size_t const more = cases[ static_cast<size_t> ( idx > count ) ];
    size_t const available = _written - more;

    VkMappedMemoryRange const* ranges = _ranges.data ();

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkFlushMappedMemoryRanges ( device, static_cast<uint32_t> ( available ), ranges + _baseIndex ),
        "UMASparseUniformPool::IssueSync",
        "Can't flush memory ranges (a)"
    );

    if ( !result ) [[unlikely]]
        return false;

    if ( more < 1U ) [[likely]]
        return true;

    return android_vulkan::Renderer::CheckVkResult (
        vkFlushMappedMemoryRanges ( device, static_cast<uint32_t> ( more ), ranges ),
        "pbr::UMASparseUniformPool::IssueSync",
        "Can't flush memory ranges (b)"
    );
}

void UMASparseUniformPool::Push ( void const* item, size_t size ) noexcept
{
    VkMappedMemoryRange &range = _ranges[ _writeIndex ];

    size_t const alpha = size - 1U;
    range.size = static_cast<VkDeviceSize> ( alpha + _nonCoherentAtomSize - ( alpha % _nonCoherentAtomSize ) );

    std::memcpy ( _data + static_cast<size_t> ( range.offset ), item, size );

    _writeIndex = ( _writeIndex + 1U ) % _ranges.size ();
    ++_written;
}

bool UMASparseUniformPool::Init ( android_vulkan::Renderer &renderer,
    DescriptorSetLayout const &descriptorSetLayout,
    eUniformPoolSize size,
    size_t itemSize,
    uint32_t bind,
    [[maybe_unused]] char const* name
) noexcept
{
    AV_ASSERT ( itemSize > 0U )
    AV_ASSERT ( itemSize <= renderer.GetMaxUniformBufferRange () )

    constexpr VkBufferUsageFlags usageFlags = AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT );

    size_t const bufferSize = KILOBYTES_TO_BYTES * static_cast<size_t> ( size );

    VkBufferCreateInfo const bufferCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = static_cast<VkDeviceSize> ( bufferSize ),
        .usage = usageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateBuffer ( device, &bufferCreateInfo, nullptr, &_buffer ),
        "pbr::UMASparseUniformPool::Init",
        "Can't create uniform buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements ( device, _buffer, &requirements );

    constexpr VkMemoryPropertyFlags memoryFlags = AV_VK_FLAG ( VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ) |
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ) |
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_CACHED_BIT );

    result = renderer.TryAllocateMemory ( _memory,
        _offset,
        requirements,
        memoryFlags,
        "Can't allocate GPU memory (pbr::UMASparseUniformPool::Init)"
    );

    if ( !result ) [[unlikely]]
        return false;

    void* ptr;

    if ( !renderer.MapMemory ( ptr, _memory, _offset, "pbr::UMASparseUniformPool::Init", "Can't map memory" ) )
    {
        [[unlikely]]
        return false;
    }

    _data = static_cast<uint8_t*> ( ptr );
    _nonCoherentAtomSize = renderer.GetNonCoherentAtomSize ();
    size_t const minUniformBufferOffsetAlignment = renderer.GetMinUniformBufferOffsetAlignment ();
    size_t alignment;

    if ( _nonCoherentAtomSize != 0U && minUniformBufferOffsetAlignment != 0U ) [[likely]]
    {
        alignment = std::lcm ( _nonCoherentAtomSize, minUniformBufferOffsetAlignment );
    }
    else
    {
        size_t const cases[] = { std::max ( _nonCoherentAtomSize, minUniformBufferOffsetAlignment ), itemSize };
        alignment = cases[ static_cast<size_t> ( cases[ 0U ] == 0U ) ];
    }

    size_t const alpha = itemSize - 1U;
    auto const stepSize = static_cast<VkDeviceSize> ( alpha + alignment - ( alpha % alignment ) );
    size_t const items = bufferSize / static_cast<size_t> ( stepSize );

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
        "pbr::UMASparseUniformPool::Init",
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
        "pbr::UMASparseUniformPool::Init",
        "Can't allocate descriptor sets"
    );

    if ( !result ) [[unlikely]]
        return false;

#if defined ( ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS ) ||       \
    defined ( ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION )

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

#endif // ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS || ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION

    std::vector<VkDescriptorBufferInfo> bufferInfo ( items,
        {
            .buffer = _buffer,
            .offset = 0U,
            .range = stepSize
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

    _ranges.resize ( items,
        {
            .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
            .pNext = nullptr,
            .memory = _memory,
            .offset = 0U,
            .size = 0U
        }
    );

    VkMappedMemoryRange* ranges = _ranges.data ();

    for ( VkDeviceSize offset = 0U; offset < bufferCreateInfo.size; offset += stepSize )
    {
        ( ranges++ )->offset = offset;
        bi->offset = offset;

        writes->dstSet = *sets++;
        ( writes++ )->pBufferInfo = bi++;
    }

    vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( items ), writes, 0U, nullptr );
    return true;
}

void UMASparseUniformPool::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    constexpr auto clean = [] ( auto &vector ) noexcept {
        vector.clear ();
        vector.shrink_to_fit ();
    };

    clean ( _sets );
    clean ( _ranges );

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

    if ( _memory == VK_NULL_HANDLE ) [[unlikely]]
        return;

    if ( _data ) [[likely]]
    {
        renderer.UnmapMemory ( _memory );
        _data = nullptr;
    }

    renderer.FreeMemory ( _memory, _offset );
    _memory = VK_NULL_HANDLE;
    _offset = 0U;
}

} // namespace pbr
