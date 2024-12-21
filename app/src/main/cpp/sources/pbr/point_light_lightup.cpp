#include <precompiled_headers.hpp>
#include <pbr/point_light_lightup.hpp>
#include <pbr/coordinate_system.hpp>
#include <pbr/point_light.inc>
#include <pbr/point_light_descriptor_set_layout.hpp>
#include <pbr/point_light_pass.hpp>


namespace pbr {

void PointLightLightup::Commit () noexcept
{
    if ( !_itemWritten )
        return;

    auto &[mainRange, overflowRange] = _ranges;
    VkDeviceSize const cases[] = { overflowRange.offset, mainRange.offset + mainRange.size };
    mainRange.offset = cases[ static_cast<size_t> ( _uniformPool.GetAvailableItemCount () > 0U ) ];
    mainRange.size = 0U;
    overflowRange.size = 0U;

    _itemBaseIndex = _itemWriteIndex;
    _itemReadIndex = _itemWriteIndex;
    _itemWritten = 0U;
}

void PointLightLightup::BindProgram ( VkCommandBuffer commandBuffer ) noexcept
{
    _program.Bind ( commandBuffer );
}

bool PointLightLightup::Init ( android_vulkan::Renderer &renderer,
    VkRenderPass renderPass,
    VkExtent2D const &resolution
) noexcept
{
    return _program.Init ( renderer, renderPass, resolution ) && AllocateDescriptorSets ( renderer );
}

void PointLightLightup::Destroy ( android_vulkan::Renderer &renderer ) noexcept
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

    clean ( _descriptorSets );
    clean ( _imageInfo );
    clean ( _writeSets );

    _uniformPool.Destroy ( renderer );
    _program.Destroy ( device );
}

void PointLightLightup::Lightup ( VkCommandBuffer commandBuffer,
    VkDescriptorSet transform,
    uint32_t volumeVertices
) noexcept
{
    _program.SetLightData ( commandBuffer, transform, _descriptorSets[ _itemReadIndex ] );
    vkCmdDrawIndexed ( commandBuffer, volumeVertices, 1U, 0U, 0, 0U );
    _itemReadIndex = ( _itemReadIndex + 1U ) % _descriptorSets.size ();
}

bool PointLightLightup::UpdateGPUData ( VkDevice device,
    PointLightPass const &pointLightPass,
    GXMat4 const &viewerLocal,
    GXMat4 const &view
) noexcept
{
    size_t const lightCount = pointLightPass.GetPointLightCount ();
    GXVec3 alpha {};
    GXVec3 betta {};

    PointLightLightupProgram::LightData lightData {};
    lightData._sceneScaleFactor = METERS_IN_UNIT;

    auto const stepSize = static_cast<VkDeviceSize> ( _uniformPool.GetBufferInfo ()._stepSize );
    VkMappedMemoryRange* range = _ranges;
    uint32_t rangeCount = 1U;
    size_t const setCount = _descriptorSets.size ();

    for ( size_t i = 0U; i < lightCount; ++i )
    {
        auto const [light, shadowmap] = pointLightPass.GetPointLightInfo ( i );
        _imageInfo[ _itemWriteIndex ].imageView = shadowmap->GetImageView ();
        _itemWriteIndex = ( _itemWriteIndex + 1U ) % setCount;

        lightData._lightProjection = light->GetProjection ();

        // Matrix optimization:
        // Point light has identity orientation by design. Point light changes its location only. Because of that
        // point light view matrix is negative "w" component. Additionally this method receives viewer local matrix
        // which is an inverse transform from view matrix. So "view to point light" transform simplifies to
        // one subtraction operation of point light location vector from "w" component of the viewer local matrix.

        GXMat4 &viewToPointLight = lightData._viewToLight;
        viewToPointLight = viewerLocal;

        viewToPointLight.GetW ( alpha );
        GXVec3 const &location = light->GetLocation ();
        betta.Subtract ( alpha, location );
        viewToPointLight.SetW ( betta );

        lightData._hue = light->GetHue ();
        lightData._intensity = light->GetIntensity ();

        view.MultiplyAsPoint ( lightData._lightLocationView, location );

        if ( _itemWriteIndex == 0U ) [[unlikely]]
        {
            ++range;
            rangeCount = 2U;
            _uniformPool.Reset ();
        }

        _uniformPool.Push ( &lightData, sizeof ( lightData ) );
        range->size += stepSize;
        ++_itemWritten;
    }

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkFlushMappedMemoryRanges ( device, rangeCount, _ranges ),
        "pbr::PointLightLightup::UpdateGPUData",
        "Can't flush memory ranges"
    );

    if ( !result ) [[unlikely]]
        return false;

    size_t const count = _descriptorSets.size ();
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

    if ( more < 1U ) [[likely]]
        return true;

    vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( more ), writeSets, 0U, nullptr );
    return true;
}

bool PointLightLightup::AllocateDescriptorSets ( android_vulkan::Renderer &renderer ) noexcept
{
    constexpr auto lightDataSize = static_cast<VkDeviceSize> ( sizeof ( PointLightLightupProgram::LightData ) );

    bool result = _uniformPool.Init ( renderer,
        eUniformPoolSize::Tiny_4M,
        static_cast<size_t> ( lightDataSize ),
        "Point light lightup"
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

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "pbr::PointLightLightup::AllocateDescriptorSets",
        "Can't create descriptor pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _descriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL, "Point light lightup" )

    _descriptorSets.resize ( setCount );
    VkDescriptorSet* descriptorSets = _descriptorSets.data ();
    std::vector<VkDescriptorSetLayout> const layouts ( setCount, PointLightDescriptorSetLayout ().GetLayout () );

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
        "pbr::PointLightLightup::AllocateDescriptorSets",
        "Can't allocate descriptor sets"
    );

    if ( !result ) [[unlikely]]
        return false;

#if defined ( AV_ENABLE_VVL ) || defined ( AV_ENABLE_RENDERDOC )

    for ( size_t i = 0U; i < setCount; ++i )
        AV_SET_VULKAN_OBJECT_NAME ( device, descriptorSets[ i ], VK_OBJECT_TYPE_DESCRIPTOR_SET, "Point light #%zu", i )

#endif // AV_ENABLE_VVL || AV_ENABLE_RENDERDOC

    // Initialize all immutable constant fields.

    constexpr VkDescriptorImageInfo imageTemplate
    {
        .sampler = VK_NULL_HANDLE,
        .imageView = VK_NULL_HANDLE,
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL
    };

    _imageInfo.resize ( setCount, imageTemplate );

    VkDescriptorImageInfo const* imageInfo = _imageInfo.data ();
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
        .dstBinding = BIND_SHADOWMAP_TEXTURE,
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

} // namespace pbr
