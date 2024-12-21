#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <pbr/point_light_shadowmap_generator.inc>
#include <pbr/shadowmap_pool.hpp>


namespace pbr {

VkDescriptorSet ShadowmapPool::Acquire () noexcept
{
    return _descriptorSets[ std::exchange ( _readIndex, ( _readIndex + 1U ) % _descriptorSets.size () ) ];
}

void ShadowmapPool::Commit () noexcept
{
    _baseIndex = _writeIndex;
    _readIndex = _writeIndex;
    _written = 0U;
}

bool ShadowmapPool::HasNewData () const noexcept
{
    return _readIndex != _writeIndex;
}

bool ShadowmapPool::IssueSync ( VkDevice device ) const noexcept
{
    size_t const count = _descriptorSets.size ();
    size_t const idx = _baseIndex + _written;
    size_t const cases[] = { 0U, idx - count };
    size_t const more = cases[ static_cast<size_t> ( idx > count ) ];
    size_t const available = _written - more;

    VkMappedMemoryRange const* ranges = _ranges.data ();
    auto const av = static_cast<uint32_t> ( available );

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkFlushMappedMemoryRanges ( device, av, ranges + _baseIndex ),
        "pbr::ShadowmapPool::IssueSync",
        "Can't flush memory ranges (a)"
    );

    if ( !result ) [[unlikely]]
        return false;

    if ( more < 1U ) [[likely]]
        return true;

    return android_vulkan::Renderer::CheckVkResult (
        vkFlushMappedMemoryRanges ( device, static_cast<uint32_t> ( more ), ranges ),
        "pbr::ShadowmapPool::IssueSync",
        "Can't flush memory ranges (b)"
    );
}

void ShadowmapPool::Push ( PointLightShadowmapGeneratorProgram::InstanceData const &data, size_t items ) noexcept
{
    size_t const dataSize = items * sizeof ( PointLightShadowmapGeneratorProgram::ObjectData );
    _uniformPool.Push ( &data, dataSize );

    size_t const alpha = dataSize - 1U;

    _ranges[ std::exchange ( _writeIndex, ( _writeIndex + 1U ) % _descriptorSets.size () ) ].size =
        static_cast<VkDeviceSize> ( alpha + _nonCoherentAtomSize - ( alpha % _nonCoherentAtomSize ) );

    if ( _writeIndex == 0U ) [[unlikely]]
        _uniformPool.Reset ();

    ++_written;
}

bool ShadowmapPool::Init ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();
    _nonCoherentAtomSize = renderer.GetNonCoherentAtomSize ();
    constexpr size_t itemSize = sizeof ( PointLightShadowmapGeneratorProgram::InstanceData );

    bool result = _descriptorSetLayout.Init ( device ) &&
        _uniformPool.Init ( renderer, eUniformSize::Huge_64M, itemSize, "Instance shadowmap" );

    if ( !result ) [[unlikely]]
        return false;

    size_t const count = _uniformPool.GetAvailableItemCount ();

    VkDescriptorPoolSize const poolSizes =
    {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = static_cast<uint32_t> ( count )
    };

    VkDescriptorPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .maxSets = static_cast<uint32_t> ( count ),
        .poolSizeCount = 1U,
        .pPoolSizes = &poolSizes
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "pbr::ShadowmapPool::Init",
        "Can't create descriptor pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _descriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL, "Geometry pool" )

    _descriptorSets.resize ( count, VK_NULL_HANDLE );
    VkDescriptorSet* descriptorSets = _descriptorSets.data ();
    std::vector<VkDescriptorSetLayout> const layouts ( count, _descriptorSetLayout.GetLayout () );

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
        "pbr::ShadowmapPool::Init",
        "Can't allocate descriptor sets"
    );

    if ( !result ) [[unlikely]]
        return false;

#if defined ( AV_ENABLE_VVL ) || defined ( AV_ENABLE_RENDERDOC )

    for ( size_t i = 0U; i < count; ++i )
    {
        AV_SET_VULKAN_OBJECT_NAME ( device,
            descriptorSets[ i ],
            VK_OBJECT_TYPE_DESCRIPTOR_SET,
            "Instance shadowmap #%zu",
            i
        )
    }

#endif // AV_ENABLE_VVL || AV_ENABLE_RENDERDOC

    // Initialize all immutable constant fields.

    constexpr VkWriteDescriptorSet writeSetTemplate
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = VK_NULL_HANDLE,
        .dstBinding = BIND_INSTANCE_DATA,
        .dstArrayElement = 0U,
        .descriptorCount = 1U,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pImageInfo = nullptr,
        .pBufferInfo = nullptr,
        .pTexelBufferView = nullptr
    };

    std::vector<VkWriteDescriptorSet> writeSets ( count, writeSetTemplate );
    VkWriteDescriptorSet* writeSet = writeSets.data ();

    UMAUniformBuffer::BufferInfo dataInfo = _uniformPool.GetBufferInfo ();
    auto const size = static_cast<VkDeviceSize> ( dataInfo._stepSize );

    _ranges.resize ( count,
        {
            .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
            .pNext = nullptr,
            .memory = dataInfo._memory,
            .offset = 0U,
            .size = 0U
        }
    );

    VkMappedMemoryRange* range = _ranges.data ();

    std::vector<VkDescriptorBufferInfo> bufferInfoStorage ( count,
        {
            .buffer = dataInfo._buffer,
            .offset = 0U,
            .range = static_cast<VkDeviceSize> ( itemSize )
        }
    );

    VkDescriptorBufferInfo* bufferInfo = bufferInfoStorage.data ();
    VkDeviceSize bufferOffset = 0U;

    for ( size_t i = 0U; i < count; ++i )
    {
        ( range++ )->offset = std::exchange ( dataInfo._offset, dataInfo._offset + size );
        bufferInfo->offset = std::exchange ( bufferOffset, bufferOffset + size );

        VkWriteDescriptorSet &write = *writeSet++;
        write.dstSet = descriptorSets[ i ];
        write.pBufferInfo = bufferInfo++;
    }

    vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( count ), writeSets.data (), 0U, nullptr );

    // Now all what is needed to do is to init "VkMappedMemoryRange::size".
    _baseIndex = 0U;
    return true;
}

void ShadowmapPool::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    if ( _descriptorPool != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyDescriptorPool ( device, _descriptorPool, nullptr );
        _descriptorPool = VK_NULL_HANDLE;
    }

    constexpr auto clean = [] ( auto &vector ) noexcept {
        vector.clear ();
        vector.shrink_to_fit ();
    };

    clean ( _ranges );
    clean ( _descriptorSets );

    _uniformPool.Destroy ( renderer );
    _descriptorSetLayout.Destroy ( device );
}

} // namespace pbr

