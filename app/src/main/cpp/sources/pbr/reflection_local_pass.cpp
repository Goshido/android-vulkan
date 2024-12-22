#include <precompiled_headers.hpp>
#include <pbr/reflection_local.inc>
#include <pbr/reflection_local_pass.hpp>
#include <trace.hpp>


namespace pbr {

ReflectionLocalPass::Call::Call ( GXVec3 const &location, TextureCubeRef &prefilter, float size ) noexcept:
    _location ( location ),
    _prefilter ( prefilter ),
    _size ( size )
{
    // NOTHING
}

//----------------------------------------------------------------------------------------------------------------------

ReflectionLocalPass::ReflectionLocalPass ( UMAUniformPool &volumeDataPool ) noexcept:
    _volumeDataPool ( volumeDataPool )
{
    // NOTHING
}

void ReflectionLocalPass::Append ( TextureCubeRef &prefilter, GXVec3 const &location, float size ) noexcept
{
    _calls.emplace_back ( location, prefilter, size );
}

void ReflectionLocalPass::Execute ( VkCommandBuffer commandBuffer, android_vulkan::MeshGeometry &unitCube ) noexcept
{
    if ( !_itemWritten )
        return;

    _program.Bind ( commandBuffer );
    size_t const count = _calls.size ();
    size_t const dsCount = _descriptorSets.size ();

    for ( size_t i = 0U; i < count; ++i )
    {
        _program.SetLightData ( commandBuffer,
            _volumeDataPool.Acquire (),
            _descriptorSets[ std::exchange ( _itemReadIndex, ( _itemReadIndex + 1U ) % dsCount ) ]
        );

        vkCmdDrawIndexed ( commandBuffer, unitCube.GetVertexCount (), 1U, 0U, 0, 0U );
    }

    auto &[mainRange, overflowRange] = _ranges;
    VkDeviceSize const cases[] = { overflowRange.offset, mainRange.offset + mainRange.size };
    mainRange.offset = cases[ static_cast<size_t> ( _uniformPool.GetAvailableItemCount () > 0U ) ];
    mainRange.size = 0U;
    overflowRange.size = 0U;

    _itemBaseIndex = _itemWriteIndex;
    _itemReadIndex = _itemWriteIndex;
    _itemWritten = 0U;
}

size_t ReflectionLocalPass::GetReflectionLocalCount () const noexcept
{
    return _calls.size ();
}

bool ReflectionLocalPass::Init ( android_vulkan::Renderer &renderer,
    VkRenderPass renderPass,
    VkExtent2D const &viewport
) noexcept
{
    if ( !_program.Init ( renderer, renderPass, viewport ) ) [[unlikely]]
        return false;

    constexpr auto lightDataSize = static_cast<VkDeviceSize> ( sizeof ( ReflectionLocalProgram::LightData ) );

    bool result = _uniformPool.Init ( renderer,
        eUniformSize::Nanoscopic_64KB,
        static_cast<size_t> ( lightDataSize ),
        "Reflection local"
    );

    if ( !result ) [[unlikely]]
        return false;

    UMAUniformBuffer::BufferInfo const umaBufferInfo = _uniformPool.GetBufferInfo ();
    auto &[mainRange, overflowRange] = _ranges;

    overflowRange =
    {
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .pNext = nullptr,
        .memory = umaBufferInfo._memory,
        .offset = umaBufferInfo._offset,
        .size = 0U
    };

    mainRange = overflowRange;

    size_t const setCount = _uniformPool.GetAvailableItemCount ();
    VkDevice device = renderer.GetDevice ();

    VkDescriptorPoolSize const poolSizes[]
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

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "pbr::ReflectionLocalPass::Init",
        "Can't create descriptor pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _descriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL, "Reflection local" )

    _descriptorSets.resize ( setCount );
    VkDescriptorSet* descriptorSets = _descriptorSets.data ();
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
        vkAllocateDescriptorSets ( device, &allocateInfo, descriptorSets ),
        "pbr::ReflectionLocalPass::Init",
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
            "Reflection local #%zu",
            i
        )
    }

#endif // AV_ENABLE_VVL || AV_ENABLE_RENDERDOC

    // Initialize all immutable constant fields.

    constexpr VkDescriptorImageInfo image
        {
            .sampler = VK_NULL_HANDLE,
            .imageView = VK_NULL_HANDLE,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };

    _imageInfo.resize ( setCount, image );
    VkDescriptorImageInfo* imageInfo = _imageInfo.data ();
    VkDeviceSize bufferOffset = 0U;

    std::vector<VkDescriptorBufferInfo> bufferInfo ( setCount,
        {
            .buffer = umaBufferInfo._buffer,
            .offset = 0U,
            .range = lightDataSize
        }
    );

    VkDescriptorBufferInfo* bi = bufferInfo.data ();

    constexpr VkWriteDescriptorSet bufferWriteTemplate
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = VK_NULL_HANDLE,
        .dstBinding = BIND_LIGHT_DATA,
        .dstArrayElement = 0U,
        .descriptorCount = 1U,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pImageInfo = nullptr,
        .pBufferInfo = nullptr,
        .pTexelBufferView = nullptr
    };

    std::vector<VkWriteDescriptorSet> bufferWrites ( setCount, bufferWriteTemplate );
    VkWriteDescriptorSet* bufferWrite = bufferWrites.data ();

    constexpr VkWriteDescriptorSet imageWriteTemplate
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = VK_NULL_HANDLE,
        .dstBinding = BIND_PREFILTER_TEXTURE,
        .dstArrayElement = 0U,
        .descriptorCount = 1U,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .pImageInfo = nullptr,
        .pBufferInfo = nullptr,
        .pTexelBufferView = nullptr
    };

    _writeSets.resize ( setCount, imageWriteTemplate );
    VkWriteDescriptorSet* imageWrites = _writeSets.data ();

    auto const stepSize = static_cast<VkDeviceSize> ( umaBufferInfo._stepSize );

    for ( size_t i = 0U; i < setCount; ++i )
    {
        VkDescriptorSet set = descriptorSets[ i ];

        VkWriteDescriptorSet &imageSet = *imageWrites++;
        imageSet.dstSet = set;
        imageSet.pImageInfo = imageInfo;

        bi->offset = std::exchange ( bufferOffset, bufferOffset + stepSize );

        VkWriteDescriptorSet &bufferSet = *bufferWrite++;
        bufferSet.dstSet = set;
        bufferSet.pBufferInfo = bi++;
    }

    vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( setCount ), bufferWrites.data (), 0U, nullptr );

    // Now all what is needed to do is to init "_imageInfo::imageView" and to invoke vkUpdateDescriptorSets.

    _itemBaseIndex = 0U;
    _itemReadIndex = 0U;
    _itemWriteIndex = 0U;
    _itemWritten = 0U;

    return true;
}

void ReflectionLocalPass::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    if ( _descriptorPool != VK_NULL_HANDLE )
    {
        vkDestroyDescriptorPool ( device, _descriptorPool, nullptr );
        _descriptorPool = VK_NULL_HANDLE;
    }

    constexpr auto clean = [] ( auto &vector ) noexcept {
        vector.clear ();
        vector.shrink_to_fit ();
    };

    clean ( _calls );
    clean ( _descriptorSets );
    clean ( _imageInfo );
    clean ( _writeSets );

    _program.Destroy ( device );
}

void ReflectionLocalPass::Reset () noexcept
{
    AV_TRACE ( "Reflection local reset" )
    _calls.clear ();
}

bool ReflectionLocalPass::UploadGPUData ( VkDevice device, GXMat4 const &view, GXMat4 const &viewProjection ) noexcept
{
    size_t const callCount = _calls.size ();

    if ( !callCount )
        return true;

    ReflectionLocalProgram::LightData lightData {};
    GXMat4 alpha {};
    alpha.Identity ();

    VolumeData volumeData {};
    GXMat4 &transform = volumeData._transform;
    GXMat4 local {};
    local.Identity ();

    auto const stepSize = static_cast<VkDeviceSize> ( _uniformPool.GetBufferInfo ()._stepSize );
    VkMappedMemoryRange* range = _ranges;
    uint32_t rangeCount = 1U;
    size_t const count = _descriptorSets.size ();

    for ( auto const &call : _calls )
    {
        local._data[ 0U ][ 0U ] = call._size;
        local._data[ 1U ][ 1U ] = call._size;
        local._data[ 2U ][ 2U ] = call._size;
        local.SetW ( call._location );

        transform.Multiply ( local, viewProjection );
        _volumeDataPool.Push ( &volumeData );

        _imageInfo[ _itemWriteIndex ].imageView = call._prefilter->GetImageView ();
        _itemWriteIndex = ( _itemWriteIndex + 1U ) % count;

        if ( _itemWriteIndex == 0U ) [[unlikely]]
        {
            ++range;
            rangeCount = 2U;
            _uniformPool.Reset ();
        }

        lightData._invSize = 2.0F / call._size;
        view.MultiplyAsPoint ( lightData._locationView, call._location );
        _uniformPool.Push ( &lightData, sizeof ( lightData ) );

        range->size += stepSize;
        ++_itemWritten;
    }

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkFlushMappedMemoryRanges ( device, rangeCount, _ranges ),
        "pbr::ReflectionLocalPass::UpdateGPUData",
        "Can't flush memory ranges"
    );

    if ( !result ) [[unlikely]]
        return false;

    size_t const idx = _itemBaseIndex + _itemWritten;
    size_t const cases[] = { 0U, idx - count };
    size_t const more = cases[ static_cast<size_t> ( idx > count ) ];
    size_t const available = _itemWritten - more;

    VkWriteDescriptorSet const* writeSets = _writeSets.data ();

    vkUpdateDescriptorSets ( device,
        static_cast<uint32_t> ( available ),
        writeSets + _itemBaseIndex,
        0U,
        nullptr
    );

    if ( more > 0U ) [[unlikely]]
        vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( more ), writeSets, 0U, nullptr );

    return true;
}

} // namespace pbr
