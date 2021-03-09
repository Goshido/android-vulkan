#include <pbr/light_pass.h>
#include <vulkan_utils.h>


namespace pbr {

LightPass::LightPass () noexcept:
    _lightupRenderPassInfo {},
    _lightVolume {},
    _lightupCommonDescriptorSet {},
    _pointLightPass {}
{
    // NOTHING
}

[[maybe_unused]] bool LightPass::Init ( android_vulkan::Renderer &renderer, GBuffer &gBuffer )
{
    _lightupRenderPassInfo.framebuffer = VK_NULL_HANDLE;
    _lightupRenderPassInfo.renderPass = VK_NULL_HANDLE;

    VkDevice device = renderer.GetDevice ();

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

    if ( !_pointLightPass.Init ( renderer, gBuffer.GetResolution (), _lightupRenderPassInfo.renderPass ) )
    {
        Destroy ( renderer.GetDevice () );
        return false;
    }

    return true;
}

void LightPass::Destroy ( VkDevice device )
{
    _pointLightPass.Destroy ( device );
    _lightupCommonDescriptorSet.Destroy ( device );
    _lightVolume.Destroy ( device );

    if ( _lightupRenderPassInfo.framebuffer != VK_NULL_HANDLE )
    {
        vkDestroyFramebuffer ( device, _lightupRenderPassInfo.framebuffer, nullptr );
        _lightupRenderPassInfo.framebuffer = VK_NULL_HANDLE;
        AV_UNREGISTER_FRAMEBUFFER ( "LightPass::_lightupFramebuffer" )
    }

    if ( _lightupRenderPassInfo.renderPass == VK_NULL_HANDLE )
        return;

    vkDestroyRenderPass ( device, _lightupRenderPassInfo.renderPass, nullptr );
    _lightupRenderPassInfo.renderPass = VK_NULL_HANDLE;
    AV_UNREGISTER_RENDER_PASS ( "LightPass::_lightupRenderPass" )
}

size_t LightPass::GetPointLightCount () const
{
    return _pointLightPass.GetPointLightCount ();
}

[[maybe_unused]] bool LightPass::OnPreGeometryPass ( android_vulkan::Renderer &renderer,
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

[[maybe_unused]] bool LightPass::OnPostGeometryPass ( android_vulkan::Renderer &renderer,
    bool &isCommonSetBind,
    VkCommandBuffer commandBuffer,
    GXMat4 const &viewerLocal,
    GXMat4 const &view,
    GXMat4 const &viewProjection
)
{
    return _pointLightPass.ExecuteLightupPhase ( renderer,
        isCommonSetBind,
        _lightVolume,
        _lightupCommonDescriptorSet,
        _lightupRenderPassInfo,
        commandBuffer,
        viewerLocal,
        view,
        viewProjection
    );
}

void LightPass::Reset ()
{
    _pointLightPass.Reset ();
}

void LightPass::Submit ( LightRef const &light )
{
    _pointLightPass.Submit ( light );
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

    AV_REGISTER_RENDER_PASS ( "PointLightPass::_lightupRenderPass" )

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

} // namespace pbr
