#include <pbr/light_pass.h>
#include <vulkan_utils.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace pbr {

constexpr static char const BRDF_LUT[] = "pbr/system/brdf-lut.png";

LightPass::LightPass () noexcept:
    _brdfLUT {},
    _brdfLUTSampler {},
    _commandBuffer ( VK_NULL_HANDLE ),
    _commandPool ( VK_NULL_HANDLE ),
    _lightupRenderPassInfo {},
    _lightVolume {},
    _lightupCommonDescriptorSet {},
    _pointLightPass {},
    _reflectionGlobalPass {}
{
    // NOTHING
}

bool LightPass::Init ( android_vulkan::Renderer &renderer, GBuffer &gBuffer )
{
    _lightupRenderPassInfo.framebuffer = VK_NULL_HANDLE;
    _lightupRenderPassInfo.renderPass = VK_NULL_HANDLE;

    VkCommandPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( device, &poolInfo, nullptr, &_commandPool ),
        "LightPass::Init",
        "Can't create command pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_COMMAND_POOL ( "LightPass::_commandPool" )

    VkCommandBufferAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = _commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1U
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &allocateInfo, &_commandBuffer ),
        "LightPass::Init",
        "Can't allocate command buffer"
    );

    if ( !result )
    {
        Destroy ( device );
        return false;
    }

    if ( !_brdfLUT.UploadData ( renderer, BRDF_LUT, android_vulkan::eFormat::Unorm, false, _commandBuffer ) )
    {
        Destroy ( device );
        return false;
    }

    constexpr VkSamplerCreateInfo const samplerInfo
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

    if ( !_brdfLUTSampler.Init ( renderer, samplerInfo ) )
    {
        Destroy ( device );
        return false;
    }

    if ( !CreateLightupRenderPass ( device, gBuffer ) )
    {
        Destroy ( device );
        return false;
    }

    if ( !CreateLightupFramebuffer ( device, gBuffer ) )
    {
        Destroy ( device );
        return false;
    }

    if ( !_lightVolume.Init ( renderer, gBuffer, _lightupRenderPassInfo.renderPass ) )
    {
        Destroy ( renderer.GetDevice () );
        return false;
    }

    if ( !_lightupCommonDescriptorSet.Init ( renderer, gBuffer ) )
    {
        Destroy ( renderer.GetDevice () );
        return false;
    }

    VkExtent2D const& resolution = gBuffer.GetResolution ();

    if ( !_pointLightPass.Init ( renderer, resolution, _lightupRenderPassInfo.renderPass ) )
    {
        Destroy ( renderer.GetDevice () );
        return false;
    }

    result = _reflectionGlobalPass.Init ( renderer,
        _lightupRenderPassInfo.renderPass,
        _brdfLUTSampler.GetSampler (),
        _brdfLUT.GetImageView (),
        1U,
        resolution
    );

    if ( !result )
    {
        Destroy ( renderer.GetDevice () );
        return false;
    }

    return true;
}

void LightPass::Destroy ( VkDevice device )
{
    _reflectionGlobalPass.Destroy ( device );
    _pointLightPass.Destroy ( device );
    _lightupCommonDescriptorSet.Destroy ( device );
    _lightVolume.Destroy ( device );

    if ( _lightupRenderPassInfo.framebuffer != VK_NULL_HANDLE )
    {
        vkDestroyFramebuffer ( device, _lightupRenderPassInfo.framebuffer, nullptr );
        _lightupRenderPassInfo.framebuffer = VK_NULL_HANDLE;
        AV_UNREGISTER_FRAMEBUFFER ( "LightPass::_lightupFramebuffer" )
    }

    if ( _lightupRenderPassInfo.renderPass != VK_NULL_HANDLE )
    {
        vkDestroyRenderPass ( device, _lightupRenderPassInfo.renderPass, nullptr );
        _lightupRenderPassInfo.renderPass = VK_NULL_HANDLE;
        AV_UNREGISTER_RENDER_PASS ( "LightPass::_lightupRenderPass" )
    }

    _brdfLUTSampler.Destroy ( device );
    _brdfLUT.FreeResources ( device );
    DestroyCommandPool ( device );
}

size_t LightPass::GetPointLightCount () const
{
    return _pointLightPass.GetPointLightCount ();
}

void LightPass::OnFreeTransferResources ( VkDevice device )
{
    _brdfLUT.FreeTransferResources ( device );
    DestroyCommandPool ( device );
}

bool LightPass::OnPreGeometryPass ( android_vulkan::Renderer &renderer,
    VkExtent2D const &resolution,
    SceneData const &sceneData,
    size_t opaqueMeshCount,
    GXMat4 const &cvvToView
)
{
    if ( !_lightupCommonDescriptorSet.Update ( renderer, resolution, cvvToView ) )
        return false;

    return _pointLightPass.ExecuteShadowPhase ( renderer, sceneData, opaqueMeshCount );
}

bool LightPass::OnPostGeometryPass ( android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer,
    GXMat4 const &viewerLocal,
    GXMat4 const &view,
    GXMat4 const &viewProjection
)
{
    bool isCommonSetBind = false;

    bool result = _pointLightPass.ExecuteLightupPhase ( renderer,
        isCommonSetBind,
        _lightVolume,
        _lightupCommonDescriptorSet,
        _lightupRenderPassInfo,
        commandBuffer,
        viewerLocal,
        view,
        viewProjection
    );

    if ( !result )
        return false;

    vkCmdBeginRenderPass ( commandBuffer, &_lightupRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
    vkCmdNextSubpass ( commandBuffer, VK_SUBPASS_CONTENTS_INLINE );

    GXMat4 dummy;
    dummy.Identity ();

    result = _reflectionGlobalPass.Execute ( renderer,
        isCommonSetBind,
        _lightupCommonDescriptorSet,
        commandBuffer,
        dummy
    );

    vkCmdEndRenderPass ( commandBuffer );
    return result;
}

void LightPass::Reset ()
{
    _pointLightPass.Reset ();
    _reflectionGlobalPass.Reset ();
}

void LightPass::SubmitPointLight ( LightRef const &light )
{
    _pointLightPass.Submit ( light );
}

void LightPass::SubmitReflectionGlobal ( TextureCubeRef &prefilter )
{
    _reflectionGlobalPass.Append ( prefilter );
}

[[maybe_unused]] void LightPass::SubmitReflectionLocal ( TextureCubeRef &/*prefilter*/,
    GXVec3 const &/*location*/,
    float /*size*/
)
{
    // TODO
    assert ( false );
}

bool LightPass::CreateLightupFramebuffer ( VkDevice device, GBuffer &gBuffer )
{
    VkImageView const attachments[] =
    {
        gBuffer.GetHDRAccumulator ().GetImageView (),
        gBuffer.GetAlbedo ().GetImageView (),
        gBuffer.GetNormal ().GetImageView (),
        gBuffer.GetParams ().GetImageView (),
        gBuffer.GetDepthStencil ().GetImageView ()
    };

    VkExtent2D const& resolution = gBuffer.GetResolution ();

    VkFramebufferCreateInfo const info
    {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .renderPass = _lightupRenderPassInfo.renderPass,
        .attachmentCount = static_cast<uint32_t> ( std::size ( attachments ) ),
        .pAttachments = attachments,
        .width = resolution.width,
        .height = resolution.height,
        .layers = 1U
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateFramebuffer ( device, &info, nullptr, &_lightupRenderPassInfo.framebuffer ),
        "LightPass::CreateLightupFramebuffer",
        "Can't create framebuffer"
    );

    if ( !result )
        return false;

    AV_REGISTER_FRAMEBUFFER ( "LightPass::_lightupFramebuffer" )
    return true;
}

bool LightPass::CreateLightupRenderPass ( VkDevice device, GBuffer &gBuffer )
{
    VkAttachmentDescription const attachments[]
    {
        // #0: HDR accumulator
        {
            .flags = 0U,
            .format = gBuffer.GetHDRAccumulator ().GetFormat (),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        },

        // #1: albedo
        {
            .flags = 0U,
            .format = gBuffer.GetAlbedo ().GetFormat (),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        },

        // #2: normal
        {
            .flags = 0U,
            .format = gBuffer.GetNormal ().GetFormat (),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        },

        // #3: params
        {
            .flags = 0U,
            .format = gBuffer.GetParams ().GetFormat (),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        },

        // #4: depth|stencil
        {
            .flags = 0U,
            .format = gBuffer.GetDepthStencil ().GetFormat (),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE,
            .initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        }
    };

    constexpr static VkAttachmentReference const depthStencilReference =
    {
        .attachment = 4U,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    constexpr static VkAttachmentReference const colorReferences[] =
    {
        // #0: HDR accumulator
        {
            .attachment = 0U,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        }
    };

    constexpr static VkAttachmentReference const readOnlyDepth
    {
        .attachment = 4U,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
    };

    constexpr static VkAttachmentReference const inputAttachments[] =
    {
        {
            .attachment = 1U,
            .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },
        {
            .attachment = 2U,
            .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },
        {
            .attachment = 3U,
            .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },
        {
            .attachment = 4U,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
        }
    };

    constexpr static VkSubpassDescription subpasses[] =
    {
        {
            .flags = 0U,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = 0U,
            .pInputAttachments = nullptr,
            .colorAttachmentCount = 0U,
            .pColorAttachments = nullptr,
            .pResolveAttachments = nullptr,
            .pDepthStencilAttachment = &depthStencilReference,
            .preserveAttachmentCount = 0U,
            .pPreserveAttachments = nullptr
        },
        {
            .flags = 0U,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = static_cast<uint32_t> ( std::size ( inputAttachments ) ),
            .pInputAttachments = inputAttachments,
            .colorAttachmentCount = static_cast<uint32_t> ( std::size ( colorReferences ) ),
            .pColorAttachments = colorReferences,
            .pResolveAttachments = nullptr,
            .pDepthStencilAttachment = &readOnlyDepth,
            .preserveAttachmentCount = 0U,
            .pPreserveAttachments = nullptr
        }
    };

    constexpr static VkSubpassDependency const dependencies[] =
    {
        {
            .srcSubpass = 0U,
            .dstSubpass = 1U,
            .srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
        }
    };

    VkRenderPassCreateInfo const renderPassInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .attachmentCount = static_cast<uint32_t> ( std::size ( attachments ) ),
        .pAttachments = attachments,
        .subpassCount = static_cast<uint32_t> ( std::size ( subpasses ) ),
        .pSubpasses = subpasses,
        .dependencyCount = static_cast<uint32_t> ( std::size ( dependencies ) ),
        .pDependencies = dependencies
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateRenderPass ( device, &renderPassInfo, nullptr, &_lightupRenderPassInfo.renderPass ),
        "PointLightPass::CreateLightupRenderPass",
        "Can't create light-up render pass"
    );

    if ( !result )
        return false;

    AV_REGISTER_RENDER_PASS ( "LightPass::_lightupRenderPass" )

    constexpr static VkClearValue const clearValues[] =
    {
        // albedo - not used actually
        {
            .color
            {
                .float32 = { 0.0F, 0.0F, 0.0F, 0.0F }
            }
        },

        // emission - not used actually
        {
            .color
            {
                .float32 = { 0.0F, 0.0F, 0.0F, 0.0F }
            }
        },

        // normal - not used actually
        {
            .color
            {
                .float32 = { 0.5F, 0.5F, 0.5F, 0.0F }
            }
        },

        // param - not used actually
        {
            .color
            {
                .float32 = { 0.5F, 0.5F, 0.5F, 0.0F }
            }
        },

        // depth|stencil - but this will be used, obviously
        {
            .depthStencil
            {
                .depth = 1.0F,
                .stencil = LightVolumeProgram::GetStencilInitialValue ()
            }
        }
    };

    _lightupRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    _lightupRenderPassInfo.pNext = nullptr;

    _lightupRenderPassInfo.renderArea =
    {
        .offset
        {
            .x = 0,
            .y = 0
        },

        .extent = gBuffer.GetResolution ()
    };

    _lightupRenderPassInfo.clearValueCount = static_cast<uint32_t> ( std::size ( clearValues ) );
    _lightupRenderPassInfo.pClearValues = clearValues;
    return true;
}

void LightPass::DestroyCommandPool ( VkDevice device )
{
    if ( _commandPool == VK_NULL_HANDLE )
        return;

    vkDestroyCommandPool ( device, _commandPool, nullptr );
    _commandPool = VK_NULL_HANDLE;
    _commandBuffer = VK_NULL_HANDLE;
    AV_UNREGISTER_COMMAND_POOL ( "LightPass::_commandPool" )
}

} // namespace pbr
