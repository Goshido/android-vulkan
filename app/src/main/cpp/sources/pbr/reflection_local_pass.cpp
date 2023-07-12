#include <pbr/reflection_local_pass.hpp>
#include <trace.hpp>


namespace pbr {

constexpr static size_t BIND_PER_SET = 2U;

//----------------------------------------------------------------------------------------------------------------------

ReflectionLocalPass::Call::Call ( GXVec3 const &location, TextureCubeRef &prefilter, float size ) noexcept:
    _location ( location ),
    _prefilter ( prefilter ),
    _size ( size )
{
    // NOTHING
}

//----------------------------------------------------------------------------------------------------------------------

void ReflectionLocalPass::Commit () noexcept
{
    _itemBaseIndex = _itemWriteIndex;
    _itemReadIndex = _itemWriteIndex;
    _itemWritten = 0U;
}

void ReflectionLocalPass::Append ( TextureCubeRef &prefilter, GXVec3 const &location, float size ) noexcept
{
    _calls.emplace_back ( Call ( location, prefilter, size ) );
}

void ReflectionLocalPass::Execute ( VkCommandBuffer commandBuffer,
    android_vulkan::MeshGeometry &unitCube,
    UniformBufferPoolManager &volumeBufferPool
) noexcept
{
    _program.Bind ( commandBuffer );
    size_t const count = _calls.size ();

    for ( size_t i = 0U; i < count; ++i )
    {
        _program.SetLightData ( commandBuffer, volumeBufferPool.Acquire (), _descriptorSets[ _itemReadIndex ] );
        vkCmdDrawIndexed ( commandBuffer, unitCube.GetVertexCount (), 1U, 0U, 0, 0U );
        _itemReadIndex = ( _itemReadIndex + 1U ) % _descriptorSets.size ();
    }

    Commit();
}

size_t ReflectionLocalPass::GetReflectionLocalCount () const noexcept
{
    return _calls.size ();
}

bool ReflectionLocalPass::Init ( android_vulkan::Renderer &renderer,
    VkRenderPass renderPass,
    uint32_t subpass,
    VkExtent2D const &viewport
) noexcept
{
    return _program.Init ( renderer, renderPass, subpass, viewport ) && AllocateDescriptorSets ( renderer );
}

void ReflectionLocalPass::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    if ( _descriptorPool != VK_NULL_HANDLE )
    {
        vkDestroyDescriptorPool ( device, _descriptorPool, nullptr );
        _descriptorPool = VK_NULL_HANDLE;
        AV_UNREGISTER_DESCRIPTOR_POOL ( "pbr::ReflectionLocalPass::_descriptorPool" )
    }

    auto const clean = [] ( auto &vector ) noexcept {
        vector.clear ();
        vector.shrink_to_fit ();
    };

    clean ( _barriers );
    clean ( _bufferInfo );
    clean ( _calls );
    clean ( _descriptorSets );
    clean ( _imageInfo );
    clean ( _writeSets );

    _uniformPool.Destroy ( renderer );
    _program.Destroy ( device );
}

void ReflectionLocalPass::Reset () noexcept
{
    AV_TRACE ( "Reflection local reset" )
    _calls.clear ();
}

void ReflectionLocalPass::UploadGPUData ( VkDevice device,
    VkCommandBuffer commandBuffer,
    UniformBufferPoolManager &volumeBufferPool,
    GXMat4 const &view,
    GXMat4 const &viewProjection
) noexcept
{
    size_t const callCount = _calls.size ();

    if ( !callCount )
        return;

    ReflectionLocalProgram::LightData lightData {};
    GXMat4 alpha {};
    alpha.Identity ();

    ReflectionLocalProgram::VolumeData volumeData {};
    GXMat4 &transform = volumeData._transform;
    GXMat4 local {};
    local.Identity ();

    for ( auto const &call : _calls )
    {
        local._data[ 0U ] = call._size;
        local._data[ 5U ] = call._size;
        local._data[ 10U ] = call._size;
        local.SetW ( call._location );

        transform.Multiply ( local, viewProjection );
        volumeBufferPool.Push ( commandBuffer, &volumeData, sizeof ( volumeData ) );

        _imageInfo[ _itemWriteIndex ].imageView = call._prefilter->GetImageView ();

        lightData._invSize = 2.0F / call._size;
        view.MultiplyAsPoint ( lightData._locationView, call._location );
        _uniformPool.Push ( commandBuffer, &lightData, sizeof ( lightData ) );

        _itemWriteIndex = ( _itemWriteIndex + 1U ) % _descriptorSets.size ();

        if ( _itemWriteIndex == 0U )
            _uniformPool.Reset ();

        ++_itemWritten;
    }

    IssueSync ( device, commandBuffer );
}

bool ReflectionLocalPass::AllocateDescriptorSets ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( !_uniformPool.Init ( renderer, sizeof ( ReflectionLocalProgram::LightData ) ) )
        return false;

    size_t const setCount = _uniformPool.GetAvailableItemCount ();
    VkDevice device = renderer.GetDevice ();

    VkDescriptorPoolSize const poolSizes[] =
    {
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = static_cast<uint32_t> ( setCount )
        },
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
        "pbr::ReflectionLocalPass::AllocateNativeDescriptorSets",
        "Can't create descriptor pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_POOL ( "pbr::ReflectionLocalPass::_descriptorPool" )

    _descriptorSets.resize ( setCount );
    std::vector<VkDescriptorSetLayout> const layouts ( setCount, ReflectionLocalDescriptorSetLayout ().GetLayout () );

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
        "pbr::PointLightLightup::AllocateNativeDescriptorSets",
        "Can't allocate descriptor sets"
    );

    if ( !result )
        return false;

    // Initialize all immutable constant fields.

    constexpr VkDescriptorImageInfo image
    {
        .sampler = VK_NULL_HANDLE,
        .imageView = VK_NULL_HANDLE,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    _imageInfo.resize ( setCount, image );

    constexpr VkDescriptorBufferInfo uniform
    {
        .buffer = VK_NULL_HANDLE,
        .offset = 0U,
        .range = static_cast<VkDeviceSize> ( sizeof ( ReflectionLocalProgram::LightData ) )
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
        .descriptorType = VK_DESCRIPTOR_TYPE_MAX_ENUM,
        .pImageInfo = nullptr,
        .pBufferInfo = nullptr,
        .pTexelBufferView = nullptr
    };

    _writeSets.resize ( BIND_PER_SET * setCount, writeSet );

    constexpr VkBufferMemoryBarrier bufferBarrier
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = VK_NULL_HANDLE,
        .offset = 0U,
        .size = static_cast<VkDeviceSize> ( sizeof ( ReflectionLocalProgram::LightData ) )
    };

    _barriers.resize ( setCount, bufferBarrier );

    for ( size_t i = 0U; i < setCount; ++i )
    {
        size_t const base = BIND_PER_SET * i;
        VkDescriptorSet set = _descriptorSets[ i ];
        VkBuffer buffer = _uniformPool.GetBuffer ( i );

        VkDescriptorBufferInfo &bufferInfo = _bufferInfo[ i ];
        bufferInfo.buffer = buffer;

        _barriers[ i ].buffer = buffer;

        VkWriteDescriptorSet &imageSet = _writeSets[ base ];
        imageSet.dstSet = set;
        imageSet.dstBinding = 0U;
        imageSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        imageSet.pImageInfo = &_imageInfo[ i ];

        VkWriteDescriptorSet &bufferSet = _writeSets[ base + 1U ];
        bufferSet.dstSet = set;
        bufferSet.dstBinding = 1U;
        bufferSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        bufferSet.pBufferInfo = &bufferInfo;
    }

    // Now all what is needed to do is to init "_bufferInfo::buffer" and "_imageInfo::imageView".
    // Then to invoke vkUpdateDescriptorSets.

    _itemBaseIndex = 0U;
    _itemReadIndex = 0U;
    _itemWriteIndex = 0U;
    _itemWritten = 0U;

    return true;
}

void ReflectionLocalPass::IssueSync ( VkDevice device, VkCommandBuffer commandBuffer ) const noexcept
{
    size_t const count = _descriptorSets.size ();
    size_t const idx = _itemBaseIndex + _itemWritten;
    size_t const cases[] = { 0U, idx - count };
    size_t const more = cases[ static_cast<size_t> ( idx > count ) ];
    size_t const available = _itemWritten - more;

    VkBufferMemoryBarrier const* barriers = _barriers.data ();

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0U,
        0U,
        nullptr,
        static_cast<uint32_t> ( available ),
        barriers + _itemBaseIndex,
        0U,
        nullptr
    );

    VkWriteDescriptorSet const* writeSets = _writeSets.data ();

    vkUpdateDescriptorSets ( device,
        static_cast<uint32_t> ( BIND_PER_SET * available ),
        writeSets + BIND_PER_SET * _itemBaseIndex,
        0U,
        nullptr
    );

    if ( more < 1U )
        return;

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0U,
        0U,
        nullptr,
        static_cast<uint32_t> ( more ),
        barriers,
        0U,
        nullptr
    );

    vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( BIND_PER_SET * more ), writeSets, 0U, nullptr );
}

} // namespace pbr
