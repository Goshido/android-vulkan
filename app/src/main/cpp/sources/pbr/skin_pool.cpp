#include <pbr/command_buffer_count.hpp>
#include <pbr/skin.inc>
#include <pbr/skin_pool.hpp>
#include <renderer.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

namespace {

constexpr size_t MAX_SKIN_MESHES_PER_FRAME = 8096U;
constexpr size_t BIND_PER_SET = 4U;
constexpr size_t SKIN_MESHES = DUAL_COMMAND_BUFFER * MAX_SKIN_MESHES_PER_FRAME;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

VkDescriptorSet SkinPool::Acquire () noexcept
{
    VkDescriptorSet set = _descriptorSets[ _itemReadIndex ];
    _itemReadIndex = ( _itemReadIndex + 1U ) % SKIN_MESHES;
    return set;
}

bool SkinPool::Init ( VkDevice device ) noexcept
{
    constexpr size_t totalBuffers = SKIN_MESHES * BIND_PER_SET;

    VkDescriptorPoolSize const poolSizes[] =
    {
        {
            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = static_cast<uint32_t> ( totalBuffers )
        }
    };

    VkDescriptorPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .maxSets = static_cast<uint32_t> ( SKIN_MESHES ),
        .poolSizeCount = static_cast<uint32_t> ( std::size ( poolSizes ) ),
        .pPoolSizes = poolSizes
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "pbr::SkinPool::Init",
        "Can't create descriptor pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _descriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL, "Skin pool" )

    if ( !_layout.Init ( device ) ) [[unlikely]]
        return false;

    _descriptorSets.resize ( SKIN_MESHES );
    VkDescriptorSet* descriptorSets = _descriptorSets.data ();
    std::vector<VkDescriptorSetLayout> const layouts ( SKIN_MESHES, _layout.GetLayout () );

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
        "pbr::SkinPool::Init",
        "Can't allocate descriptor sets"
    );

    if ( !result ) [[unlikely]]
        return false;

#if defined ( ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS ) ||       \
    defined ( ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION )

    for ( size_t i = 0U; i < SKIN_MESHES; ++i )
        AV_SET_VULKAN_OBJECT_NAME ( device, descriptorSets[ i ], VK_OBJECT_TYPE_DESCRIPTOR_SET, "Skin #%zu", i )

#endif // ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS || ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION

    // Initialize all immutable constant fields.

    constexpr VkDescriptorBufferInfo buffer
    {
        .buffer = VK_NULL_HANDLE,
        .offset = 0U,
        .range = 0U
    };

    _bufferInfo.resize ( totalBuffers, buffer );
    VkDescriptorBufferInfo* bufferInfo = _bufferInfo.data ();

    constexpr VkWriteDescriptorSet writeSet
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = VK_NULL_HANDLE,
        .dstBinding = 0U,
        .dstArrayElement = 0U,
        .descriptorCount = 1U,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .pImageInfo = nullptr,
        .pBufferInfo = nullptr,
        .pTexelBufferView = nullptr
    };

    _writeSets.resize ( totalBuffers, writeSet );
    VkWriteDescriptorSet* writeSets = _writeSets.data ();
    size_t idx = 0U;

    for ( size_t i = 0U; i < SKIN_MESHES; ++i )
    {
        VkDescriptorSet set = descriptorSets[ i ];

        VkWriteDescriptorSet &pose = writeSets[ idx ];
        pose.dstSet = set;
        pose.dstBinding = static_cast<uint32_t> ( BIND_POSE );
        pose.pBufferInfo = bufferInfo + idx++;

        VkWriteDescriptorSet &referenceMesh = writeSets[ idx ];
        referenceMesh.dstSet = set;
        referenceMesh.dstBinding = static_cast<uint32_t> ( BIND_REFERENCE_MESH );
        referenceMesh.pBufferInfo = bufferInfo + idx++;

        VkWriteDescriptorSet &skin = writeSets[ idx ];
        skin.dstSet = set;
        skin.dstBinding = static_cast<uint32_t> ( BIND_SKIN );
        skin.pBufferInfo = bufferInfo + idx++;

        VkWriteDescriptorSet &skinMesh = writeSets[ idx ];
        skinMesh.dstSet = set;
        skinMesh.dstBinding = static_cast<uint32_t> ( BIND_SKIN_MESH );
        skinMesh.pBufferInfo = bufferInfo + idx++;
    }

    // Now all what is needed to do is to init "_bufferInfo::buffer" and "_bufferInfo::range".
    // Then to invoke vkUpdateDescriptorSets.

    constexpr VkBufferMemoryBarrier barrier
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = VK_NULL_HANDLE,
        .offset = 0U,
        .size = 0U
    };

    _barriers.resize ( SKIN_MESHES, barrier );

    // Now all what is needed to do is to init "_barriers::buffer" and "_barriers::size".
    // Then to invoke vkCmdPipelineBarrier.

    _itemBaseIndex = 0U;
    _itemReadIndex = 0U;
    _itemWriteIndex = 0U;
    _itemWritten = 0U;

    return true;
}

void SkinPool::Destroy ( VkDevice device ) noexcept
{
    _layout.Destroy ( device );

    if ( _descriptorPool != VK_NULL_HANDLE )
    {
        vkDestroyDescriptorPool ( device, _descriptorPool, nullptr );
        _descriptorPool = VK_NULL_HANDLE;
    }

    auto const clean = [] ( auto &vector ) noexcept {
        vector.clear ();
        vector.shrink_to_fit ();
    };

    clean ( _barriers );
    clean ( _bufferInfo );
    clean ( _descriptorSets );
    clean ( _writeSets );
}

void SkinPool::Push ( android_vulkan::BufferInfo pose,
    android_vulkan::BufferInfo skin,
    android_vulkan::BufferInfo referenceMesh,
    VkBuffer skinMesh
) noexcept
{
    VkDescriptorBufferInfo* buffers = _bufferInfo.data ();
    size_t const base = _itemWriteIndex * BIND_PER_SET;

    VkDescriptorBufferInfo &poseInfo = buffers[ base + BIND_POSE ];
    poseInfo.buffer = pose._buffer;
    poseInfo.range = pose._range;

    VkDescriptorBufferInfo &skinInfo = buffers[ base + BIND_SKIN ];
    skinInfo.buffer = skin._buffer;
    skinInfo.range = skin._range;

    VkDescriptorBufferInfo &referenceMeshInfo = buffers[ base + BIND_REFERENCE_MESH ];
    referenceMeshInfo.buffer = referenceMesh._buffer;
    referenceMeshInfo.range = referenceMesh._range;

    VkDescriptorBufferInfo &skinMeshInfo = buffers[ base + BIND_SKIN_MESH ];
    skinMeshInfo.buffer = skinMesh;
    skinMeshInfo.range = referenceMesh._range;

    VkBufferMemoryBarrier &barrier = _barriers[ _itemWriteIndex ];
    barrier.buffer = skinMesh;
    barrier.size = referenceMesh._range;

    _itemWriteIndex = ( _itemWriteIndex + 1U ) % SKIN_MESHES;
    ++_itemWritten;
}

void SkinPool::SubmitPipelineBarriers ( VkCommandBuffer commandBuffer ) noexcept
{
    size_t const idx = _itemBaseIndex + _itemWritten;
    size_t const cases[] = { 0U, idx - SKIN_MESHES };
    size_t const more = cases[ static_cast<size_t> ( idx > SKIN_MESHES ) ];
    size_t const available = _itemWritten - more;

    VkBufferMemoryBarrier const* barriers = _barriers.data ();

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
        0U,
        0U,
        nullptr,
        static_cast<uint32_t> ( available ),
        barriers + _itemBaseIndex,
        0U,
        nullptr
    );

    if ( more > 0U ) [[unlikely]]
    {
        vkCmdPipelineBarrier ( commandBuffer,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
            0U,
            0U,
            nullptr,
            static_cast<uint32_t> ( more ),
            barriers,
            0U,
            nullptr
        );
    }

    _itemBaseIndex = _itemWriteIndex;
    _itemReadIndex = _itemWriteIndex;
    _itemWritten = 0U;
}

void SkinPool::UpdateDescriptorSets ( VkDevice device ) const noexcept
{
    size_t const idx = _itemBaseIndex + _itemWritten;
    size_t const cases[] = { 0U, idx - SKIN_MESHES };
    size_t const more = cases[ static_cast<size_t> ( idx > SKIN_MESHES ) ];
    size_t const available = _itemWritten - more;

    VkWriteDescriptorSet const* writeSets = _writeSets.data ();

    vkUpdateDescriptorSets ( device,
        static_cast<uint32_t> ( BIND_PER_SET * available ),
        writeSets + BIND_PER_SET * _itemBaseIndex,
        0U,
        nullptr
    );

    if ( more > 0U ) [[unlikely]]
    {
        vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( BIND_PER_SET * more ), writeSets, 0U, nullptr );
    }
}

} // namespace pbr
