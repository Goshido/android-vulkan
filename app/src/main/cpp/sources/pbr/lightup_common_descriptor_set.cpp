#include <vulkan_utils.h>
#include <pbr/lightup_common_descriptor_set.h>
#include <pbr/light_lightup_base_program.h>


namespace pbr {

constexpr static char const BRDF_LUT[] = "pbr/system/brdf-lut.png";

// 256 128 64 32 16 8 4 2 1
// counting from 0.0F
constexpr static float MAX_PREFILTER_LOD = 8.0F;

//----------------------------------------------------------------------------------------------------------------------

void LightupCommonDescriptorSet::Bind ( VkCommandBuffer commandBuffer, size_t swapchainImageIndex ) noexcept
{
    vkCmdBindDescriptorSets ( commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        _pipelineLayout,
        0U,
        1U,
        &_sets[ swapchainImageIndex ],
        0U,
        nullptr
    );
}

bool LightupCommonDescriptorSet::Init ( android_vulkan::Renderer &renderer,
    VkCommandPool commandPool,
    GBuffer &gBuffer
) noexcept
{
    VkDevice device = renderer.GetDevice ();
    auto const swapchainImages = static_cast<uint32_t> ( renderer.GetPresentImageCount () );

    VkDescriptorPoolSize const poolSizes[] =
    {
        {
            .type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            .descriptorCount = 4U * swapchainImages
        },
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = swapchainImages
        },
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = 2U * swapchainImages
        },
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = swapchainImages
        }
    };

    VkDescriptorPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .maxSets = swapchainImages,
        .poolSizeCount = static_cast<uint32_t> ( std::size ( poolSizes ) ),
        .pPoolSizes = poolSizes
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "pbr::LightupCommonDescriptorSet::Init",
        "Can't create descriptor pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_POOL ( "pbr::LightupCommonDescriptorSet::_descriptorPool" )

    if ( !_layout.Init ( device ) )
        return false;

    VkDescriptorSetLayout layout = _layout.GetLayout ();
    std::vector<VkDescriptorSetLayout> layouts ( static_cast<size_t> ( swapchainImages ), layout );

    VkDescriptorSetAllocateInfo const descriptorSetAllocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _descriptorPool,
        .descriptorSetCount = swapchainImages,
        .pSetLayouts = layouts.data ()
    };

    _sets.resize ( static_cast<size_t> ( swapchainImages ) );
    VkDescriptorSet* sets = _sets.data ();

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &descriptorSetAllocateInfo, sets ),
        "pbr::LightupCommonDescriptorSet::Init",
        "Can't allocate descriptor set"
    );

    if ( !result )
        return false;

    if ( !_uniforms.Init ( renderer, sizeof ( LightLightupBaseProgram::ViewData ) ) )
        return false;

    VkCommandBufferAllocateInfo const bufferAllocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1U
    };

    VkCommandBuffer textureCommandBuffer;

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &bufferAllocateInfo, &textureCommandBuffer ),
        "pbr::LightupCommonDescriptorSet::Init",
        "Can't allocate command buffer"
    );

    if ( !result )
        return false;

    if ( !_brdfLUT.UploadData ( renderer, BRDF_LUT, android_vulkan::eFormat::Unorm, false, textureCommandBuffer ) )
        return false;

    constexpr VkSamplerCreateInfo brdfSamplerInfo
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .mipLodBias = 0.0F,
        .anisotropyEnable = VK_FALSE,
        .maxAnisotropy = 1.0F,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0F,
        .maxLod = 0.0F,
        .borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };

    if ( !_brdfLUTSampler.Init ( device, brdfSamplerInfo ) )
        return false;

    constexpr VkSamplerCreateInfo prefilterSamplerInfo
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .mipLodBias = 0.0F,
        .anisotropyEnable = VK_FALSE,
        .maxAnisotropy = 1.0F,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0F,
        .maxLod = MAX_PREFILTER_LOD,
        .borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };

    if ( !_prefilterSampler.Init ( device, prefilterSamplerInfo ) )
        return false;

    VkDescriptorImageInfo const images[] =
    {
        {
            .sampler = VK_NULL_HANDLE,
            .imageView = gBuffer.GetAlbedo ().GetImageView (),
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },
        {
            .sampler = VK_NULL_HANDLE,
            .imageView = gBuffer.GetNormal ().GetImageView (),
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },
        {
            .sampler = VK_NULL_HANDLE,
            .imageView = gBuffer.GetParams ().GetImageView (),
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },
        {
            .sampler = VK_NULL_HANDLE,
            .imageView = gBuffer.GetReadOnlyDepthImageView (),
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
        },
        {
            .sampler = _brdfLUTSampler.GetSampler (),
            .imageView = _brdfLUT.GetImageView (),
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },
        {
            .sampler = _prefilterSampler.GetSampler (),
            .imageView = VK_NULL_HANDLE,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        }
    };

    std::vector<VkWriteDescriptorSet> write ( 7U * static_cast<size_t> ( swapchainImages ) );
    size_t idx = 0U;

    for ( uint32_t i = 0U; i < swapchainImages; ++i )
    {
        VkDescriptorSet set = _sets[ i ];

        VkWriteDescriptorSet& albedo = write[ idx++ ];
        albedo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        albedo.pNext = nullptr;
        albedo.dstSet = set;
        albedo.dstBinding = 0U;
        albedo.dstArrayElement = 0U;
        albedo.descriptorCount = 1U;
        albedo.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        albedo.pImageInfo = images;
        albedo.pBufferInfo = nullptr;
        albedo.pTexelBufferView = nullptr;

        VkWriteDescriptorSet& normal = write[ idx++ ];
        normal.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        normal.pNext = nullptr;
        normal.dstSet = set;
        normal.dstBinding = 1U;
        normal.dstArrayElement = 0U;
        normal.descriptorCount = 1U;
        normal.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        normal.pImageInfo = images + 1U;
        normal.pBufferInfo = nullptr;
        normal.pTexelBufferView = nullptr;

        VkWriteDescriptorSet& params = write[ idx++ ];
        params.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        params.pNext = nullptr;
        params.dstSet = set;
        params.dstBinding = 2U;
        params.dstArrayElement = 0U;
        params.descriptorCount = 1U;
        params.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        params.pImageInfo = images + 2U;
        params.pBufferInfo = nullptr;
        params.pTexelBufferView = nullptr;

        VkWriteDescriptorSet& depthStencil = write[ idx++ ];
        depthStencil.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        depthStencil.pNext = nullptr;
        depthStencil.dstSet = set;
        depthStencil.dstBinding = 3U;
        depthStencil.dstArrayElement = 0U;
        depthStencil.descriptorCount = 1U;
        depthStencil.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        depthStencil.pImageInfo = images + 3U;
        depthStencil.pBufferInfo = nullptr;
        depthStencil.pTexelBufferView = nullptr;

        VkWriteDescriptorSet& brdfImage = write[ idx++ ];
        brdfImage.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        brdfImage.pNext = nullptr;
        brdfImage.dstSet = set;
        brdfImage.dstBinding = 4U;
        brdfImage.dstArrayElement = 0U;
        brdfImage.descriptorCount = 1U;
        brdfImage.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        brdfImage.pImageInfo = images + 4U;
        brdfImage.pBufferInfo = nullptr;
        brdfImage.pTexelBufferView = nullptr;

        VkWriteDescriptorSet& brdfSampler = write[ idx++ ];
        brdfSampler.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        brdfSampler.pNext = nullptr;
        brdfSampler.dstSet = set;
        brdfSampler.dstBinding = 5U;
        brdfSampler.dstArrayElement = 0U;
        brdfSampler.descriptorCount = 1U;
        brdfSampler.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        brdfSampler.pImageInfo = images + 4U;
        brdfSampler.pBufferInfo = nullptr;
        brdfSampler.pTexelBufferView = nullptr;

        VkWriteDescriptorSet& prefilterSampler = write[ idx++ ];
        prefilterSampler.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        prefilterSampler.pNext = nullptr;
        prefilterSampler.dstSet = set;
        prefilterSampler.dstBinding = 6U;
        prefilterSampler.dstArrayElement = 0U;
        prefilterSampler.descriptorCount = 1U;
        prefilterSampler.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        prefilterSampler.pImageInfo = images + 5U;
        prefilterSampler.pBufferInfo = nullptr;
        prefilterSampler.pTexelBufferView = nullptr;
    }

    vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( write.size () ), write.data (), 0U, nullptr );

    VkPipelineLayoutCreateInfo const layoutInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .setLayoutCount = 1U,
        .pSetLayouts = &layout,
        .pushConstantRangeCount = 0U,
        .pPushConstantRanges = nullptr
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreatePipelineLayout ( device, &layoutInfo, nullptr, &_pipelineLayout ),
        "pbr::LightupCommonDescriptorSet::Init",
        "Can't create pipeline layout"
    );

    if ( !result )
        return false;

    AV_REGISTER_PIPELINE_LAYOUT ( "pbr::LightupCommonDescriptorSet::_pipelineLayout" )

    _bufferInfo.offset = 0U;
    _bufferInfo.range = static_cast<VkDeviceSize> ( sizeof ( LightLightupBaseProgram::ViewData ) );

    _writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    _writeInfo.pNext = nullptr;
    _writeInfo.dstBinding = 7U;
    _writeInfo.dstArrayElement = 0U;
    _writeInfo.descriptorCount = 1U;
    _writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    _writeInfo.pImageInfo = nullptr;
    _writeInfo.pBufferInfo = &_bufferInfo;
    _writeInfo.pTexelBufferView = nullptr;

    _barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    _barrier.pNext = nullptr;
    _barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    _barrier.dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT;
    _barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    _barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    _barrier.offset = 0U;
    _barrier.size = static_cast<VkDeviceSize> ( sizeof ( LightLightupBaseProgram::ViewData ) );

    return true;
}

void LightupCommonDescriptorSet::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();
    _prefilterSampler.Destroy ( device );
    _brdfLUTSampler.Destroy ( device );
    _brdfLUT.FreeResources ( renderer );
    _uniforms.Destroy ( renderer );

    if ( _pipelineLayout != VK_NULL_HANDLE )
    {
        vkDestroyPipelineLayout ( device, _pipelineLayout, nullptr );
        _pipelineLayout = VK_NULL_HANDLE;
        AV_UNREGISTER_PIPELINE_LAYOUT ( "pbr::LightupCommonDescriptorSet::_pipelineLayout" )
    }

    _layout.Destroy ( device );

    if ( _descriptorPool == VK_NULL_HANDLE )
        return;

    vkDestroyDescriptorPool ( device, _descriptorPool, nullptr );
    _descriptorPool = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_POOL ( "pbr::LightupCommonDescriptorSet::_descriptorPool" )
}

void LightupCommonDescriptorSet::OnFreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept
{
    _brdfLUT.FreeTransferResources ( renderer );
}

void LightupCommonDescriptorSet::Update ( VkDevice device,
    VkCommandBuffer commandBuffer,
    size_t swapchainImageIndex,
    VkExtent2D const &resolution,
    GXMat4 const &viewerLocal,
    GXMat4 const &cvvToView
) noexcept
{
    LightLightupBaseProgram::ViewData const viewData
    {
        ._cvvToView = cvvToView,
        ._viewToWorld = viewerLocal,

        ._invResolutionFactor
        {
            2.0F / static_cast<float> ( resolution.width ),
            2.0F / static_cast<float> ( resolution.height )
        },

        ._padding0_0 {}
    };

    if ( _uniforms.GetAvailableItemCount () < 1U )
        _uniforms.Reset ();

    VkBuffer buffer = _uniforms.Push ( commandBuffer, &viewData, sizeof ( viewData ) );

    _bufferInfo.buffer = buffer;
    _writeInfo.dstSet = _sets[ swapchainImageIndex ];
    vkUpdateDescriptorSets ( device, 1U, &_writeInfo, 0U, nullptr );

    _barrier.buffer = buffer;

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0U,
        0U,
        nullptr,
        1U,
        &_barrier,
        0U,
        nullptr
    );
}

} // namespace pbr
