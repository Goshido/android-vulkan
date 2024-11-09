#include <pbr/point_light_lightup.hpp>
#include <pbr/coordinate_system.hpp>
#include <pbr/point_light_descriptor_set_layout.hpp>
#include <pbr/point_light_pass.hpp>


namespace pbr {

namespace {

constexpr size_t BIND_PER_SET = 3U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void PointLightLightup::Commit () noexcept
{
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
    uint32_t subpass,
    VkExtent2D const &resolution
) noexcept
{
    constexpr VkSamplerCreateInfo samplerInfo
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .magFilter = VK_FILTER_NEAREST,
        .minFilter = VK_FILTER_NEAREST,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .mipLodBias = 0.0F,
        .anisotropyEnable = VK_FALSE,
        .maxAnisotropy = 1.0F,
        .compareEnable = VK_TRUE,
        .compareOp = VK_COMPARE_OP_GREATER,
        .minLod = 0.0F,
        .maxLod = 0.0F,
        .borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };

    return _program.Init ( renderer, renderPass, subpass, resolution ) &&
        _sampler.Init ( renderer.GetDevice (), samplerInfo, "Point light lightup" ) &&
        AllocateDescriptorSets ( renderer );
}

void PointLightLightup::Destroy ( android_vulkan::Renderer &renderer ) noexcept
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

    clean ( _barriers );
    clean ( _bufferInfo );
    clean ( _descriptorSets );
    clean ( _imageInfo );
    clean ( _writeSets );

    _uniformPool.Destroy ( renderer );
    _sampler.Destroy ( device );
    _program.Destroy ( device );
}

void PointLightLightup::Lightup ( VkCommandBuffer commandBuffer,
    VkDescriptorSet transform,
    android_vulkan::MeshGeometry &unitCube
) noexcept
{
    _program.SetLightData ( commandBuffer, transform, _descriptorSets[ _itemReadIndex ] );
    vkCmdDrawIndexed ( commandBuffer, unitCube.GetVertexCount (), 1U, 0U, 0, 0U );
    _itemReadIndex = ( _itemReadIndex + 1U ) % _descriptorSets.size ();
}

void PointLightLightup::UpdateGPUData ( VkDevice device,
    VkCommandBuffer commandBuffer,
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

    for ( size_t i = 0U; i < lightCount; ++i )
    {
        auto const [light, shadowmap] = pointLightPass.GetPointLightInfo ( i );
        _imageInfo[ _itemWriteIndex ].imageView = shadowmap->GetImageView ();
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
        _uniformPool.Push ( commandBuffer, &lightData, sizeof ( lightData ) );

        _itemWriteIndex = ( _itemWriteIndex + 1U ) % _descriptorSets.size ();

        if ( _itemWriteIndex == 0U )
            _uniformPool.Reset ();

        ++_itemWritten;
    }

    IssueSync ( device, commandBuffer );
}

bool PointLightLightup::AllocateDescriptorSets ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( !_uniformPool.Init ( renderer, sizeof ( PointLightLightupProgram::LightData ), "Point light lightup" ) )
    {
        [[unlikely]]
        return false;
    }

    size_t const setCount = _uniformPool.GetAvailableItemCount ();
    VkDevice device = renderer.GetDevice ();

    VkDescriptorPoolSize const poolSizes[] =
    {
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = static_cast<uint32_t> ( setCount )
        },
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLER,
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

#if defined ( ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS ) ||       \
    defined ( ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION )

    for ( size_t i = 0U; i < setCount; ++i )
        AV_SET_VULKAN_OBJECT_NAME ( device, descriptorSets[ i ], VK_OBJECT_TYPE_DESCRIPTOR_SET, "Point light #%zu", i )

#endif // ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS || ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION

    // Initialize all immutable constant fields.

    VkDescriptorImageInfo const image
    {
        .sampler = _sampler.GetSampler (),
        .imageView = VK_NULL_HANDLE,
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL
    };

    _imageInfo.resize ( setCount, image );

    constexpr VkDescriptorBufferInfo uniform
    {
        .buffer = VK_NULL_HANDLE,
        .offset = 0U,
        .range = static_cast<VkDeviceSize> ( sizeof ( PointLightLightupProgram::LightData ) )
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
        .size = static_cast<VkDeviceSize> ( sizeof ( PointLightLightupProgram::LightData ) )
    };

    _barriers.resize ( setCount, bufferBarrier );

    for ( size_t i = 0U; i < setCount; ++i )
    {
        size_t const base = BIND_PER_SET * i;
        VkDescriptorSet set = descriptorSets[ i ];

        VkBuffer buffer = _uniformPool.GetBuffer ( i );
        _barriers[ i ].buffer = buffer;

        VkDescriptorBufferInfo &bufferInfo = _bufferInfo[ i ];
        bufferInfo.buffer = buffer;

        VkWriteDescriptorSet &imageSet = _writeSets[ base ];
        imageSet.dstSet = set;
        imageSet.dstBinding = 0U;
        imageSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        imageSet.pImageInfo = &_imageInfo[ i ];

        VkWriteDescriptorSet &samplerSet = _writeSets[ base + 1U ];
        samplerSet.dstSet = set;
        samplerSet.dstBinding = 1U;
        samplerSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        samplerSet.pImageInfo = &_imageInfo[ i ];

        VkWriteDescriptorSet &bufferSet = _writeSets[ base + 2U ];
        bufferSet.dstSet = set;
        bufferSet.dstBinding = 2U;
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

void PointLightLightup::IssueSync ( VkDevice device, VkCommandBuffer commandBuffer ) const noexcept
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
