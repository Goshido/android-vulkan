#include <pbr/light_pass.h>
#include <vulkan_utils.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace pbr {

bool LightPass::Init ( android_vulkan::Renderer &renderer, VkCommandPool commandPool, GBuffer &gBuffer ) noexcept
{
    _lightupRenderPassInfo.framebuffer = VK_NULL_HANDLE;
    _lightupRenderPassInfo.renderPass = VK_NULL_HANDLE;

    VkDevice device = renderer.GetDevice ();

    if ( !CreateLightupRenderPass ( device, gBuffer ) || !CreateLightupFramebuffer ( device, gBuffer ) )
    {
        Destroy ( device );
        return false;
    }

    if ( !_lightVolume.Init ( renderer, gBuffer, _lightupRenderPassInfo.renderPass ) )
    {
        Destroy ( renderer.GetDevice () );
        return false;
    }

    if ( !_lightupCommonDescriptorSet.Init ( renderer, commandPool, gBuffer ) )
    {
        Destroy ( renderer.GetDevice () );
        return false;
    }

    VkExtent2D const& resolution = gBuffer.GetResolution ();

    if ( !_pointLightPass.Init ( renderer, *this, commandPool, resolution, _lightupRenderPassInfo.renderPass ) )
    {
        Destroy ( renderer.GetDevice () );
        return false;
    }

    if ( !_reflectionGlobalPass.Init ( renderer, _lightupRenderPassInfo.renderPass, 1U, resolution ) )
    {
        Destroy ( renderer.GetDevice () );
        return false;
    }

    bool const result = _reflectionLocalPass.Init ( renderer,
        *this,
        commandPool,
        _lightupRenderPassInfo.renderPass,
        1U,
        resolution
    );

    if ( result && CreateUnitCube ( renderer, commandPool ) )
        return true;

    Destroy ( renderer.GetDevice () );
    return false;
}

void LightPass::Destroy ( VkDevice device ) noexcept
{
    _unitCube.FreeResources ( device );

    _reflectionLocalPass.Destroy ( device );
    _reflectionGlobalPass.Destroy ( device );
    _pointLightPass.Destroy ( device );
    _lightupCommonDescriptorSet.Destroy ( device );
    _lightVolume.Destroy ( device );

    if ( _lightupRenderPassInfo.framebuffer != VK_NULL_HANDLE )
    {
        vkDestroyFramebuffer ( device, _lightupRenderPassInfo.framebuffer, nullptr );
        _lightupRenderPassInfo.framebuffer = VK_NULL_HANDLE;
        AV_UNREGISTER_FRAMEBUFFER ( "pbr::LightPass::_lightupFramebuffer" )
    }

    if ( _lightupRenderPassInfo.renderPass != VK_NULL_HANDLE )
    {
        vkDestroyRenderPass ( device, _lightupRenderPassInfo.renderPass, nullptr );
        _lightupRenderPassInfo.renderPass = VK_NULL_HANDLE;
        AV_UNREGISTER_RENDER_PASS ( "pbr::LightPass::_lightupRenderPass" )
    }

    if ( _transfer == VK_NULL_HANDLE )
        return;

    vkFreeCommandBuffers ( device, _commandPool, 1U, &_transfer );
    _transfer = VK_NULL_HANDLE;
    _commandPool = VK_NULL_HANDLE;
}

size_t LightPass::GetPointLightCount () const noexcept
{
    return _pointLightPass.GetPointLightCount ();
}

size_t LightPass::GetReflectionLocalCount () const noexcept
{
    return _reflectionLocalPass.GetReflectionLocalCount ();
}

void LightPass::OnFreeTransferResources ( VkDevice device ) noexcept
{
    _unitCube.FreeTransferResources ( device );
    _lightupCommonDescriptorSet.OnFreeTransferResources ( device );

    if ( _transfer == VK_NULL_HANDLE )
        return;

    vkFreeCommandBuffers ( device, _commandPool, 1U, &_transfer );
    _transfer = VK_NULL_HANDLE;
    _commandPool = VK_NULL_HANDLE;
}

bool LightPass::OnPreGeometryPass ( android_vulkan::Renderer &renderer,
    VkExtent2D const &resolution,
    SceneData const &sceneData,
    size_t opaqueMeshCount,
    GXMat4 const &viewerLocal,
    GXMat4 const &view,
    GXMat4 const &viewProjection,
    GXMat4 const &cvvToView
) noexcept
{
    if ( !_lightupCommonDescriptorSet.Update ( renderer, resolution, viewerLocal, cvvToView ) )
        return false;

    if ( !_pointLightPass.ExecuteShadowPhase ( renderer, sceneData, opaqueMeshCount ) )
        return false;

    return _reflectionLocalPass.UploadGPUData ( renderer, view, viewProjection );
}

bool LightPass::OnPostGeometryPass ( android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer,
    GXMat4 const &viewerLocal,
    GXMat4 const &view,
    GXMat4 const &viewProjection
) noexcept
{
    size_t const pointLights = _pointLightPass.GetPointLightCount ();
    size_t const localReflections = _reflectionLocalPass.GetReflectionLocalCount ();
    size_t const globalReflections = _reflectionGlobalPass.GetReflectionCount ();
    size_t const lightVolumes = pointLights + localReflections;

    if ( lightVolumes + globalReflections == 0U )
        return true;

    _lightupCommonDescriptorSet.Bind ( commandBuffer );

    if ( lightVolumes )
    {
        _lightupRenderPassCounter = globalReflections ? lightVolumes - 1U : lightVolumes;
    }
    else
    {
        vkCmdBeginRenderPass ( commandBuffer, &_lightupRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
        vkCmdNextSubpass ( commandBuffer, VK_SUBPASS_CONTENTS_INLINE );
    }

    if ( pointLights )
    {
        bool const result = _pointLightPass.ExecuteLightupPhase ( renderer,
            _lightVolume,
            _unitCube,
            commandBuffer,
            viewerLocal,
            view,
            viewProjection
        );

        if ( !result )
        {
            return false;
        }
    }

    if ( localReflections )
    {
        if ( !_reflectionLocalPass.Execute ( renderer, _lightVolume, _unitCube, commandBuffer ) )
        {
            return false;
        }
    }

    if ( !globalReflections )
        return true;

    bool const result = _reflectionGlobalPass.Execute ( renderer, commandBuffer, viewerLocal );

    vkCmdEndRenderPass ( commandBuffer );
    return result;
}

void LightPass::Reset () noexcept
{
    _lightupRenderPassCounter = 0U;
    _pointLightPass.Reset ();
    _reflectionGlobalPass.Reset ();
    _reflectionLocalPass.Reset ();
}

void LightPass::SubmitPointLight ( LightRef const &light ) noexcept
{
    _pointLightPass.Submit ( light );
}

void LightPass::SubmitReflectionGlobal ( TextureCubeRef &prefilter ) noexcept
{
    _reflectionGlobalPass.Append ( prefilter );
}

void LightPass::SubmitReflectionLocal ( TextureCubeRef &prefilter, GXVec3 const &location, float size ) noexcept
{
    _reflectionLocalPass.Append ( prefilter, location, size );
}

void LightPass::OnBeginLightWithVolume ( VkCommandBuffer commandBuffer ) noexcept
{
    vkCmdBeginRenderPass ( commandBuffer, &_lightupRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
}

void LightPass::OnEndLightWithVolume ( VkCommandBuffer commandBuffer ) noexcept
{
    if ( _lightupRenderPassCounter == 0U )
        return;

    vkCmdEndRenderPass ( commandBuffer );
    --_lightupRenderPassCounter;
}

bool LightPass::CreateLightupFramebuffer ( VkDevice device, GBuffer &gBuffer ) noexcept
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
        "pbr::LightPass::CreateLightupFramebuffer",
        "Can't create framebuffer"
    );

    if ( !result )
        return false;

    AV_REGISTER_FRAMEBUFFER ( "pbr::LightPass::_lightupFramebuffer" )
    return true;
}

bool LightPass::CreateLightupRenderPass ( VkDevice device, GBuffer &gBuffer ) noexcept
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
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },

        // #2: normal
        {
            .flags = 0U,
            .format = gBuffer.GetNormal ().GetFormat (),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },

        // #3: params
        {
            .flags = 0U,
            .format = gBuffer.GetParams ().GetFormat (),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },

        // #4: depth|stencil
        {
            .flags = 0U,
            .format = gBuffer.GetDepthStencil ().GetFormat (),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        }
    };

    constexpr static VkAttachmentReference depthStencilReference =
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

    constexpr static VkAttachmentReference readOnlyDepth
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

    constexpr static uint32_t const preserved[] = { 1U, 2U, 3U };

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
            .preserveAttachmentCount = static_cast<uint32_t> ( std::size ( preserved ) ),
            .pPreserveAttachments = preserved
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
            .dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
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
        "pbr::PointLightPass::CreateLightupRenderPass",
        "Can't create light-up render pass"
    );

    if ( !result )
        return false;

    AV_REGISTER_RENDER_PASS ( "pbr::LightPass::_lightupRenderPass" )

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
                .depth = 0.0F,
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

bool LightPass::CreateUnitCube ( android_vulkan::Renderer &renderer, VkCommandPool commandPool ) noexcept
{
    _commandPool = commandPool;

    VkCommandBufferAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = _commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1U
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( renderer.GetDevice (), &allocateInfo, &_transfer ),
        "pbr::LightPass::CreateUnitCube",
        "Can't allocate command buffer"
    );

    if ( !result )
    {
        Destroy ( renderer.GetDevice () );
        return false;
    }

    constexpr GXVec3 const vertices[] =
    {
        GXVec3 ( -0.5F, -0.5F, -0.5F ),
        GXVec3 ( 0.5F, -0.5F, -0.5F ),
        GXVec3 ( -0.5F, 0.5F, -0.5F ),
        GXVec3 ( 0.5F, 0.5F, -0.5F ),
        GXVec3 ( 0.5F, -0.5F, 0.5F ),
        GXVec3 ( -0.5F, -0.5F, 0.5F ),
        GXVec3 ( 0.5F, 0.5F, 0.5F ),
        GXVec3 ( -0.5F, 0.5F, 0.5F )
    };

    constexpr uint32_t const indices[] =
    {
        0U, 2U, 1U,
        1U, 2U, 3U,
        1U, 3U, 4U,
        4U, 3U, 6U,
        4U, 6U, 5U,
        5U, 6U, 7U,
        5U, 7U, 0U,
        0U, 7U, 2U,
        2U, 7U, 3U,
        3U, 7U, 6U,
        5U, 0U, 4U,
        4U, 0U, 1U
    };

    GXAABB bounds;
    bounds.AddVertex ( vertices[ 0U ] );
    bounds.AddVertex ( vertices[ 7U ] );

    return _unitCube.LoadMesh ( reinterpret_cast<uint8_t const*> ( vertices ),
        sizeof ( vertices ),
        indices,
        static_cast<uint32_t> ( std::size ( indices ) ),
        bounds,
        renderer,
        _transfer
    );
}

} // namespace pbr
