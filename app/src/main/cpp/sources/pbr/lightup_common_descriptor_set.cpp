#include <vulkan_utils.h>
#include <pbr/lightup_common_descriptor_set.h>
#include <pbr/light_lightup_base_program.h>


namespace pbr {

constexpr static char const BRDF_LUT[] = "pbr/system/brdf-lut.png";

// 256 128 64 32 16 8 4 2 1
// counting from 0.0F
constexpr static float MAX_PREFILTER_LOD = 8.0F;

//----------------------------------------------------------------------------------------------------------------------

void LightupCommonDescriptorSet::Bind ( VkCommandBuffer commandBuffer ) noexcept
{
    vkCmdBindDescriptorSets ( commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        _pipelineLayout,
        0U,
        1U,
        &_set,
        0U,
        nullptr
    );
}

bool LightupCommonDescriptorSet::Init ( android_vulkan::Renderer &renderer,
    VkCommandPool commandPool,
    GBuffer &gBuffer
) noexcept
{
    constexpr static VkDescriptorPoolSize const poolSizes[] =
    {
        {
            .type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            .descriptorCount = 4U
        },
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = 1U
        },
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = 2U
        },
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1U
        }
    };

    constexpr static VkDescriptorPoolCreateInfo poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .maxSets = 1U,
        .poolSizeCount = static_cast<uint32_t> ( std::size ( poolSizes ) ),
        .pPoolSizes = poolSizes
    };

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "pbr::LightupCommonDescriptorSet::Init",
        "Can't create descriptor pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_POOL ( "pbr::LightupCommonDescriptorSet::_descriptorPool" )

    if ( !_layout.Init ( device ) )
    {
        Destroy ( device );
        return false;
    }

    VkDescriptorSetLayout nativeLayout = _layout.GetLayout ();

    VkDescriptorSetLayout const layouts[] =
    {
        nativeLayout
    };

    VkPipelineLayoutCreateInfo const layoutInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .setLayoutCount = static_cast<uint32_t> ( std::size ( layouts ) ),
        .pSetLayouts = layouts,
        .pushConstantRangeCount = 0U,
        .pPushConstantRanges = nullptr
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreatePipelineLayout ( renderer.GetDevice (), &layoutInfo, nullptr, &_pipelineLayout ),
        "pbr::LightupCommonDescriptorSet::Init",
        "Can't create pipeline layout"
    );

    if ( !result )
    {
        Destroy ( device );
        return false;
    }

    AV_REGISTER_PIPELINE_LAYOUT ( "pbr::LightupCommonDescriptorSet::_pipelineLayout" )

    VkDescriptorSetAllocateInfo const descriptorSetAllocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _descriptorPool,
        .descriptorSetCount = 1U,
        .pSetLayouts = &nativeLayout
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &descriptorSetAllocateInfo, &_set ),
        "pbr::LightupCommonDescriptorSet::Init",
        "Can't allocate descriptor set"
    );

    if ( !result )
    {
        Destroy ( device );
        return false;
    }

    _commandPool = commandPool;

    if ( !_uniformBuffer.Init ( renderer, _commandPool, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT ) )
    {
        Destroy ( device );
        return false;
    }

    LightLightupBaseProgram::ViewData const dummy {};

    if ( !_uniformBuffer.Update ( renderer, reinterpret_cast<uint8_t const*> ( &dummy ), sizeof ( dummy ) ) )
    {
        Destroy ( device );
        return false;
    }

    VkCommandBufferAllocateInfo const commandBufferAllocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = _commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1U
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &commandBufferAllocateInfo, &_brdfTransfer ),
        "pbr::LightupCommonDescriptorSet::Init",
        "Can't allocate command buffer"
    );

    if ( !result )
    {
        Destroy ( device );
        return false;
    }

    if ( !_brdfLUT.UploadData ( renderer, BRDF_LUT, android_vulkan::eFormat::Unorm, false, _brdfTransfer ) )
    {
        Destroy ( device );
        return false;
    }

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

    if ( !_brdfLUTSampler.Init ( renderer, brdfSamplerInfo ) )
    {
        Destroy ( device );
        return false;
    }

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

    if ( !_prefilterSampler.Init ( renderer, prefilterSamplerInfo ) )
    {
        Destroy ( device );
        return false;
    }

    VkDescriptorBufferInfo const buffer
    {
        .buffer = _uniformBuffer.GetBuffer (),
        .offset = 0U,
        .range = static_cast<VkDeviceSize> ( sizeof ( LightLightupBaseProgram::ViewData ) )
    };

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
        },
    };

    VkWriteDescriptorSet const writeInfo[]
    {
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _set,
            .dstBinding = 0U,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            .pImageInfo = images,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _set,
            .dstBinding = 1U,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            .pImageInfo = images + 1U,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _set,
            .dstBinding = 2U,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            .pImageInfo = images + 2U,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _set,
            .dstBinding = 3U,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            .pImageInfo = images + 3U,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _set,
            .dstBinding = 4U,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .pImageInfo = images + 4U,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _set,
            .dstBinding = 5U,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .pImageInfo = images + 4U,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _set,
            .dstBinding = 6U,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .pImageInfo = images + 5U,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _set,
            .dstBinding = 7U,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pImageInfo = nullptr,
            .pBufferInfo = &buffer,
            .pTexelBufferView = nullptr
        }
    };

    vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( std::size ( writeInfo ) ), writeInfo, 0U, nullptr );
    return true;
}

void LightupCommonDescriptorSet::Destroy ( VkDevice device ) noexcept
{
    _prefilterSampler.Destroy ( device );
    _brdfLUTSampler.Destroy ( device );
    _brdfLUT.FreeResources ( device );

    _uniformBuffer.FreeResources ( device );

    if ( _commandPool != VK_NULL_HANDLE )
    {
        vkFreeCommandBuffers ( device, _commandPool, 1U, &_brdfTransfer );
        _brdfTransfer = VK_NULL_HANDLE;
        _commandPool = VK_NULL_HANDLE;
    }

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

void LightupCommonDescriptorSet::OnFreeTransferResources ( VkDevice device ) noexcept
{
    _brdfLUT.FreeTransferResources ( device );

    if ( _brdfTransfer == VK_NULL_HANDLE )
        return;

    vkFreeCommandBuffers ( device, _commandPool, 1U, &_brdfTransfer );
    _brdfTransfer = VK_NULL_HANDLE;
    _commandPool = VK_NULL_HANDLE;
}

bool LightupCommonDescriptorSet::Update ( android_vulkan::Renderer &renderer,
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

    return _uniformBuffer.Update ( renderer, reinterpret_cast<uint8_t const*> ( &viewData ), sizeof ( viewData ) );
}

} // namespace pbr
