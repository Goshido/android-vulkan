#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <pbr/geometry_pass_bindings.inc>
#include <pbr/geometry_pool.hpp>


namespace pbr {

namespace {

constexpr size_t SYNCS_PER_VERTEX_STAGE = 2U;
constexpr size_t WRITES_PER_ITEM = 3U;

constexpr size_t POSITION_INDEX = 0U;
constexpr size_t NORMAL_INDEX = 1U;

} // end of anonymous namepspace

VkDescriptorSet GeometryPool::Acquire () noexcept
{
    VkDescriptorSet set = _descriptorSets[ _readIndex ];
    _readIndex = ( _readIndex + 1U ) % _descriptorSets.size ();
    return set;
}

void GeometryPool::Commit () noexcept
{
    _baseIndex = _writeIndex;
    _readIndex = _writeIndex;
    _written = 0U;
}

void GeometryPool::IssueSync ( VkDevice device, VkCommandBuffer commandBuffer ) const noexcept
{
    size_t const count = _descriptorSets.size ();
    size_t const idx = _baseIndex + _written;
    size_t const cases[] = { 0U, idx - count };
    size_t const more = cases[ static_cast<size_t> ( idx > count ) ];
    size_t const available = _written - more;

    VkBufferMemoryBarrier const* vertexBarriers = _vertexBarriers.data ();

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
        0U,
        0U,
        nullptr,
        static_cast<uint32_t> ( available * SYNCS_PER_VERTEX_STAGE ),
        vertexBarriers + _baseIndex * SYNCS_PER_VERTEX_STAGE,
        0U,
        nullptr
    );

    VkBufferMemoryBarrier const* fragmentBarriers = _fragmentBarriers.data ();

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0U,
        0U,
        nullptr,
        static_cast<uint32_t> ( available ),
        fragmentBarriers + _baseIndex,
        0U,
        nullptr
    );

    VkWriteDescriptorSet const* writeSets = _writeSets.data ();

    vkUpdateDescriptorSets ( device,
        static_cast<uint32_t> ( available * WRITES_PER_ITEM ),
        writeSets + _baseIndex * WRITES_PER_ITEM,
        0U,
        nullptr
    );

    if ( more < 1U ) [[unlikely]]
        return;

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
        0U,
        0U,
        nullptr,
        static_cast<uint32_t> ( more * SYNCS_PER_VERTEX_STAGE ),
        vertexBarriers,
        0U,
        nullptr
    );

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0U,
        0U,
        nullptr,
        static_cast<uint32_t> ( more ),
        fragmentBarriers,
        0U,
        nullptr
    );

    vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( more * WRITES_PER_ITEM ), writeSets, 0U, nullptr );
}

void GeometryPool::Push ( VkCommandBuffer commandBuffer,
    GeometryPassProgram::InstancePositionData const &positionData,
    GeometryPassProgram::InstanceNormalData const &normalData,
    GeometryPassProgram::InstanceColorData const &colorData,
    size_t items
) noexcept
{
    size_t const positionDataSize = items * sizeof ( GXMat4 );
    _positionPool.Push ( commandBuffer, &positionData, positionDataSize );

    size_t const normalDataSize = items * sizeof ( GXQuat );
    _normalPool.Push ( commandBuffer, &normalData, normalDataSize );

    size_t const colorDataSize = items * sizeof ( GeometryPassProgram::ColorData );
    _colorPool.Push ( commandBuffer, &colorData, colorDataSize );

    VkBufferMemoryBarrier *vertexBarriers = _vertexBarriers.data () + _writeIndex * SYNCS_PER_VERTEX_STAGE;
    vertexBarriers[ POSITION_INDEX ].size = static_cast<VkDeviceSize> ( positionDataSize );
    vertexBarriers[ NORMAL_INDEX ].size = static_cast<VkDeviceSize> ( normalDataSize );

    _fragmentBarriers[ _writeIndex ].size = static_cast<VkDeviceSize> ( colorDataSize );

    _writeIndex = ( _writeIndex + 1U ) % _descriptorSets.size ();

    if ( _writeIndex == 0U ) [[unlikely]]
    {
        _positionPool.Reset ();
        _normalPool.Reset ();
        _colorPool.Reset ();
    }

    ++_written;
}

bool GeometryPool::Init ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    bool result = _colorPool.Init ( renderer, sizeof ( GeometryPassProgram::InstanceColorData ), "Color data" ) &&

        _positionPool.Init ( renderer,
            sizeof ( GeometryPassProgram::InstancePositionData ),
            "Position transform"
        ) &&

        _normalPool.Init ( renderer, sizeof ( GeometryPassProgram::InstanceNormalData ), "Normal transform" ) &&
        _descriptorSetLayout.Init ( device );

    if ( !result ) [[unlikely]]
        return false;

    size_t const setCount = _positionPool.GetAvailableItemCount ();

    // Logic will break or will be suboptimal in case of different amount buffers per pool.
    AV_ASSERT ( setCount == _normalPool.GetAvailableItemCount () && setCount == _colorPool.GetAvailableItemCount () )
    size_t const writeCount = setCount * WRITES_PER_ITEM;

    VkDescriptorPoolSize const poolSizes =
    {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = static_cast<uint32_t> ( writeCount )
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

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "pbr::GeometryPool::Init",
        "Can't create descriptor pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _descriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL, "Geometry pool" )

    _descriptorSets.resize ( setCount, VK_NULL_HANDLE );
    VkDescriptorSet* descriptorSets = _descriptorSets.data ();
    std::vector<VkDescriptorSetLayout> const layouts ( setCount, _descriptorSetLayout.GetLayout () );

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
        "pbr::GeometryPool::Init",
        "Can't allocate descriptor sets"
    );

    if ( !result ) [[unlikely]]
        return false;

#if defined ( ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS ) ||       \
    defined ( ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION )

    for ( size_t i = 0U; i < setCount; ++i )
        AV_SET_VULKAN_OBJECT_NAME ( device, descriptorSets[ i ], VK_OBJECT_TYPE_DESCRIPTOR_SET, "Geometry #%zu", i )

#endif // ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS || ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION

    // Initialize all immutable constant fields.

    constexpr VkDescriptorBufferInfo bufferTemplate
    {
        .buffer = VK_NULL_HANDLE,
        .offset = 0U,
        .range = VK_WHOLE_SIZE
    };

    _bufferInfo.resize ( writeCount, bufferTemplate );

    constexpr VkWriteDescriptorSet writeSetTemplate
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

    _writeSets.resize ( writeCount, writeSetTemplate );

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

    _vertexBarriers.resize ( setCount * SYNCS_PER_VERTEX_STAGE, bufferBarrierTemplate );
    _fragmentBarriers.resize ( setCount, bufferBarrierTemplate );

    VkBufferMemoryBarrier* vertexBarriers = _vertexBarriers.data ();
    VkBufferMemoryBarrier* fragmentBarriers = _fragmentBarriers.data ();

    VkDescriptorBufferInfo* bufferInfo = _bufferInfo.data ();
    VkWriteDescriptorSet* writeSets = _writeSets.data ();

    for ( size_t i = 0U; i < setCount; ++i )
    {
        VkBuffer positionBuffer = _positionPool.GetBuffer ( i );
        VkBuffer normalBuffer = _normalPool.GetBuffer ( i );
        VkBuffer colorBuffer = _colorPool.GetBuffer ( i );
        VkDescriptorSet descriptorSet = descriptorSets[ i ];

        vertexBarriers[ POSITION_INDEX ].buffer = positionBuffer;
        VkDescriptorBufferInfo* positionBufferInfo = bufferInfo++;
        positionBufferInfo->buffer = positionBuffer;

        VkWriteDescriptorSet &positionWrite = *writeSets++;
        positionWrite.dstSet = descriptorSet;
        positionWrite.dstBinding = BIND_INSTANCE_POSITON_DATA;
        positionWrite.pBufferInfo = positionBufferInfo;

        vertexBarriers[ NORMAL_INDEX ].buffer = normalBuffer;
        VkDescriptorBufferInfo* normalBufferInfo = bufferInfo++;
        normalBufferInfo->buffer = normalBuffer;

        VkWriteDescriptorSet &normalWrite = *writeSets++;
        normalWrite.dstSet = descriptorSet;
        normalWrite.dstBinding = BIND_INSTANCE_NORMAL_DATA;
        normalWrite.pBufferInfo = normalBufferInfo;

        vertexBarriers += SYNCS_PER_VERTEX_STAGE;

        fragmentBarriers->buffer = colorBuffer;

        VkDescriptorBufferInfo* colorBufferInfo = bufferInfo++;
        colorBufferInfo->buffer = colorBuffer;

        VkWriteDescriptorSet &colorWrite = *writeSets++;
        colorWrite.dstSet = descriptorSet;
        colorWrite.dstBinding = BIND_INSTANCE_COLOR_DATA;
        colorWrite.pBufferInfo = colorBufferInfo;

        ++fragmentBarriers;
    }

    // Now all what is needed to do is to init "_bufferInfo::buffer". Then to invoke vkUpdateDescriptorSets.
    _baseIndex = 0U;
    return true;
}

void GeometryPool::Destroy ( android_vulkan::Renderer &renderer ) noexcept
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

    clean ( _vertexBarriers );
    clean ( _fragmentBarriers );
    clean ( _descriptorSets );
    clean ( _bufferInfo );
    clean ( _writeSets );

    _positionPool.Destroy ( renderer );
    _normalPool.Destroy ( renderer );
    _colorPool.Destroy ( renderer );

    _descriptorSetLayout.Destroy ( device );
}

} // namespace pbr
