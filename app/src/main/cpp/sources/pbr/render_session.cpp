#include <pbr/render_session.h>

GX_DISABLE_COMMON_WARNINGS

#include <algorithm>
#include <cassert>

GX_RESTORE_WARNING_STATE

#include <vulkan_utils.h>
#include <pbr/point_light.h>
#include <pbr/reflection_probe_global.h>
#include <pbr/reflection_probe_local.h>


namespace pbr {

void RenderSession::Begin ( GXMat4 const &viewerLocal, GXMat4 const &projection ) noexcept
{
    _opaqueMeshCount = 0U;
    _cvvToView.Inverse ( projection );
    _viewerLocal = viewerLocal;
    _view.Inverse ( _viewerLocal );
    _viewProjection.Multiply ( _view, projection );
    _frustum.From ( _viewProjection );
    _lightPass.Reset ();
    _geometryPass.Reset ();
}

bool RenderSession::End ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept
{
    if ( !_presentPass.AcquirePresentTarget ( renderer ) )
        return false;

    bool result = _lightPass.OnPreGeometryPass ( renderer,
        _gBuffer.GetResolution (),
        _geometryPass.GetOpaqueSubpass ().GetSceneData (),
        _opaqueMeshCount,
        _viewerLocal,
        _view,
        _viewProjection,
        _cvvToView
    );

    if ( !result )
        return false;

    VkCommandBuffer commandBuffer = _geometryPass.Execute ( renderer,
        _frustum,
        _view,
        _viewProjection,
        _defaultTextureManager,
        _samplerManager,
        _renderSessionStats
    );

    if ( commandBuffer == VK_NULL_HANDLE )
        return false;

    result = _lightPass.OnPostGeometryPass ( renderer,
        commandBuffer,
        _viewerLocal,
        _view,
        _viewProjection
    );

    if ( !result )
        return false;

    _renderSessionStats.RenderPointLights ( _lightPass.GetPointLightCount () );
    _renderSessionStats.RenderReflectionLocal ( _lightPass.GetReflectionLocalCount () );

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0U,
        0U,
        nullptr,
        0U,
        nullptr,
        1U,
        &_gBufferImageBarrier
    );

    if ( !_presentPass.Execute ( commandBuffer, _gBufferSlotMapper, _geometryPass.GetFence (), renderer ) )
        return false;

    _renderSessionStats.PrintStats ( deltaTime );
    return true;
}

void RenderSession::FreeTransferResources ( VkDevice device, VkCommandPool commandPool ) noexcept
{
    _defaultTextureManager.FreeTransferResources ( device, commandPool );
}

bool RenderSession::OnInitDevice ( android_vulkan::Renderer &renderer, VkCommandPool commandPool ) noexcept
{
    _lightHandlers[ static_cast<size_t> ( eLightType::PointLight ) ] = &RenderSession::SubmitPointLight;
    _lightHandlers[ static_cast<size_t> ( eLightType::ReflectionGlobal ) ] = &RenderSession::SubmitReflectionGlobal;
    _lightHandlers[ static_cast<size_t> ( eLightType::ReflectionLocal ) ] = &RenderSession::SubmitReflectionLocal;

    _meshHandlers[ static_cast<size_t> ( eMaterialType::Opaque ) ] = &RenderSession::SubmitOpaqueCall;
    _meshHandlers[ static_cast<size_t> ( eMaterialType::Stipple ) ] = &RenderSession::SubmitStippleCall;

    if ( !_texturePresentDescriptorSetLayout.Init ( renderer ) )
        return false;

    if ( !_defaultTextureManager.Init ( renderer, commandPool ) )
        return false;

    return _samplerManager.Init ( renderer );
}

void RenderSession::OnDestroyDevice ( VkDevice device ) noexcept
{
    std::memset ( _lightHandlers, 0, sizeof ( _lightHandlers ) );
    std::memset ( _meshHandlers, 0, sizeof ( _meshHandlers ) );

    _texturePresentDescriptorSetLayout.Destroy ( device );
    _samplerManager.Destroy ( device );
    _defaultTextureManager.Destroy ( device );
    DestroyGBufferResources ( device );
}

bool RenderSession::OnSwapchainCreated ( android_vulkan::Renderer &renderer,
    VkExtent2D const &resolution,
    VkCommandPool commandPool
) noexcept
{
    VkExtent2D const& currentResolution = _gBuffer.GetResolution ();

    if ( ( currentResolution.width != resolution.width ) | ( currentResolution.height != resolution.height ) )
    {
        DestroyGBufferResources ( renderer.GetDevice () );

        if ( !CreateGBufferResources ( renderer, resolution, commandPool ) )
        {
            OnSwapchainDestroyed ( renderer.GetDevice () );
            return false;
        }

        android_vulkan::LogInfo ( "pbr::RenderSession::OnSwapchainCreated - G-buffer resolution is %u x %u.",
            resolution.width,
            resolution.height
        );
    }

    if ( _presentPass.Init ( renderer ) )
        return true;

    VkDevice device = renderer.GetDevice ();
    DestroyGBufferResources ( device );
    OnSwapchainDestroyed ( device );

    return false;
}

void RenderSession::OnSwapchainDestroyed ( VkDevice device ) noexcept
{
    _presentPass.Destroy ( device );
}

void RenderSession::SubmitLight ( LightRef &light ) noexcept
{
    auto const idx = static_cast<size_t> ( light->GetType () );
    assert ( idx < std::size ( _lightHandlers ) );

    LightHandler const handler = _lightHandlers[ idx ];

    // C++ calling method by pointer syntax.
    ( this->*handler ) ( light );
}

void RenderSession::SubmitMesh ( MeshRef &mesh,
    MaterialRef const &material,
    GXMat4 const &local,
    GXAABB const &worldBounds,
    GXColorRGB const &color0,
    GXColorRGB const &color1,
    GXColorRGB const &color2,
    GXColorRGB const &emission
) noexcept
{
    auto const idx = static_cast<size_t> ( material->GetMaterialType () );
    assert ( idx < std::size ( _meshHandlers ) );

    MeshHandler const handler = _meshHandlers[ idx ];

    // Calling method by pointer C++ syntax.
    ( this->*handler ) ( mesh, material, local, worldBounds, color0, color1, color2, emission );
}

bool RenderSession::CreateGBufferFramebuffer ( android_vulkan::Renderer &renderer ) noexcept
{
    VkExtent2D const& resolution = _gBuffer.GetResolution ();

    VkImageView const attachments[] =
    {
        _gBuffer.GetAlbedo ().GetImageView (),
        _gBuffer.GetHDRAccumulator ().GetImageView (),
        _gBuffer.GetNormal ().GetImageView (),
        _gBuffer.GetParams ().GetImageView (),
        _gBuffer.GetDepthStencil ().GetImageView ()
    };

    VkFramebufferCreateInfo const framebufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .renderPass = _gBufferRenderPass,
        .attachmentCount = std::size ( attachments ),
        .pAttachments = attachments,
        .width = resolution.width,
        .height = resolution.height,
        .layers = 1U
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateFramebuffer ( renderer.GetDevice (), &framebufferInfo, nullptr, &_gBufferFramebuffer ),
        "pbr::RenderSession::CreateGBufferFramebuffer",
        "Can't create GBuffer framebuffer"
    );

    if ( !result )
        return false;

    AV_REGISTER_FRAMEBUFFER ( "pbr::RenderSession::_gBufferFramebuffer" )
    return true;
}

bool RenderSession::CreateGBufferRenderPass ( android_vulkan::Renderer &renderer ) noexcept
{
    VkAttachmentDescription const attachments[]
    {
        // #0: albedo
        {
            .flags = 0U,
            .format = _gBuffer.GetAlbedo ().GetFormat (),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        },

        // #1: emission
        {
            .flags = 0U,
            .format = _gBuffer.GetHDRAccumulator ().GetFormat (),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        },

        // #2: normal
        {
            .flags = 0U,
            .format = _gBuffer.GetNormal ().GetFormat (),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        },

        // #3: params
        {
            .flags = 0U,
            .format = _gBuffer.GetParams ().GetFormat (),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        },

        // #4: depth|stencil
        {
            .flags = 0U,
            .format = _gBuffer.GetDepthStencil ().GetFormat (),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        }
    };

    static VkSubpassDescription const subpasses[] =
    {
        GeometryPass::GetSubpassDescription ()
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
        .dependencyCount = 0U,
        .pDependencies = nullptr
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateRenderPass ( renderer.GetDevice (), &renderPassInfo, nullptr, &_gBufferRenderPass ),
        "pbr::RenderSession::CreateGBufferRenderPass",
        "Can't create render pass"
    );

    if ( !result )
        return false;

    AV_REGISTER_RENDER_PASS ( "pbr::RenderSession::_gBufferRenderPass" )
    return true;
}

bool RenderSession::CreateGBufferResources ( android_vulkan::Renderer &renderer,
    VkExtent2D const &resolution,
    VkCommandPool commandPool
) noexcept
{
    VkDevice device = renderer.GetDevice ();

    if ( !_gBuffer.Init ( renderer, resolution ) )
    {
        DestroyGBufferResources ( device );
        return false;
    }

    if ( !CreateGBufferRenderPass ( renderer ) || !CreateGBufferFramebuffer ( renderer ) )
    {
        DestroyGBufferResources ( device );
        return false;
    }

    bool result = _geometryPass.Init ( renderer,
        commandPool,
        _gBuffer.GetResolution (),
        _gBufferRenderPass,
        _gBufferFramebuffer
    );

    if ( !result )
    {
        DestroyGBufferResources ( device );
        return false;
    }

    if ( !_lightPass.Init ( renderer, commandPool, _gBuffer ) )
    {
        DestroyGBufferResources ( device );
        return false;
    }

    if ( !CreateGBufferSlotMapper ( renderer ) )
    {
        DestroyGBufferResources ( device );
        return false;
    }

    android_vulkan::Texture2D const& texture = _gBuffer.GetHDRAccumulator();

    _gBufferImageBarrier =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = texture.GetImage (),

        .subresourceRange =
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0U,
            .levelCount = static_cast<uint32_t> ( texture.GetMipLevelCount () ),
            .baseArrayLayer = 0U,
            .layerCount = 1U
        }
    };

    result = android_vulkan::Renderer::CheckVkResult ( vkDeviceWaitIdle ( device ),
        "pbr::RenderSession::CreateGBufferResources",
        "Can't wait device idle"
    );

    if ( !result )
    {
        DestroyGBufferResources ( device );
        return false;
    }

    _lightPass.OnFreeTransferResources ( device );
    return true;
}

bool RenderSession::CreateGBufferSlotMapper ( android_vulkan::Renderer &renderer ) noexcept
{
    constexpr static VkDescriptorPoolSize const poolSizes[] =
    {
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = 1U
        },
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = 1U
        }
    };

    constexpr VkDescriptorPoolCreateInfo poolInfo
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
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_gBufferDescriptorPool ),
        "pbr::RenderSession::CreateGBufferSlotMapper",
        "Can't create descriptor pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_POOL ( "pbr::RenderSession::_gBufferDescriptorPool" )

    TexturePresentDescriptorSetLayout const layout;
    VkDescriptorSetLayout nativeLayout = layout.GetLayout ();

    VkDescriptorSetAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _gBufferDescriptorPool,
        .descriptorSetCount = 1U,
        .pSetLayouts = &nativeLayout
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &allocateInfo, &_gBufferSlotMapper ),
        "pbr::RenderSession::CreateGBufferSlotMapper",
        "Can't allocate descriptor sets"
    );

    if ( !result )
        return false;

    VkDescriptorImageInfo const imageInfo[] =
    {
        {
            .sampler = _samplerManager.GetPointSampler ()->GetSampler (),
            .imageView = _gBuffer.GetHDRAccumulator ().GetImageView (),
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        }
    };

    VkWriteDescriptorSet const writeSets[] =
    {
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _gBufferSlotMapper,
            .dstBinding = 0U,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .pImageInfo = imageInfo,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _gBufferSlotMapper,
            .dstBinding = 1U,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .pImageInfo = imageInfo,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        }
    };

    vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( std::size ( writeSets ) ), writeSets, 0U, nullptr );
    return true;
}

void RenderSession::DestroyGBufferResources ( VkDevice device ) noexcept
{
    if ( _gBufferFramebuffer != VK_NULL_HANDLE )
    {
        vkDestroyFramebuffer ( device, _gBufferFramebuffer, nullptr );
        _gBufferFramebuffer = VK_NULL_HANDLE;
        AV_UNREGISTER_FRAMEBUFFER ( "pbr::RenderSession::_gBufferFramebuffer" )
    }

    if ( _gBufferRenderPass != VK_NULL_HANDLE )
    {
        vkDestroyRenderPass ( device, _gBufferRenderPass, nullptr );
        _gBufferRenderPass = VK_NULL_HANDLE;
        AV_UNREGISTER_RENDER_PASS ( "pbr::RenderSession::_gBufferRenderPass" )
    }

    _lightPass.Destroy ( device );

    if ( _gBufferDescriptorPool != VK_NULL_HANDLE )
    {
        vkDestroyDescriptorPool ( device, _gBufferDescriptorPool, nullptr );
        _gBufferDescriptorPool = VK_NULL_HANDLE;
        AV_UNREGISTER_DESCRIPTOR_POOL ( "pbr::RenderSession::_gBufferDescriptorPool" )
    }

    _geometryPass.Destroy ( device );

    if ( _gBufferFramebuffer != VK_NULL_HANDLE )
    {
        vkDestroyFramebuffer ( device, _gBufferFramebuffer, nullptr );
        _gBufferFramebuffer = VK_NULL_HANDLE;
        AV_UNREGISTER_FRAMEBUFFER ( "pbr::RenderSession::_gBufferFramebuffer" )
    }

    if ( _gBufferRenderPass != VK_NULL_HANDLE )
    {
        vkDestroyRenderPass ( device, _gBufferRenderPass, nullptr );
        _gBufferRenderPass = VK_NULL_HANDLE;
        AV_UNREGISTER_RENDER_PASS ( "pbr::RenderSession::_gBufferRenderPass" )
    }

    _gBuffer.Destroy ( device );
}

void RenderSession::SubmitOpaqueCall ( MeshRef &mesh,
    MaterialRef const &material,
    GXMat4 const &local,
    GXAABB const &worldBounds,
    GXColorRGB const &color0,
    GXColorRGB const &color1,
    GXColorRGB const &color2,
    GXColorRGB const &emission
) noexcept
{
    ++_opaqueMeshCount;
    _renderSessionStats.SubmitOpaque ( mesh->GetVertexCount () );
    _geometryPass.GetOpaqueSubpass ().Submit ( mesh, material, local, worldBounds, color0, color1, color2, emission );
}

void RenderSession::SubmitStippleCall ( MeshRef &mesh,
    MaterialRef const &material,
    GXMat4 const &local,
    GXAABB const &worldBounds,
    GXColorRGB const &color0,
    GXColorRGB const &color1,
    GXColorRGB const &color2,
    GXColorRGB const &emission
) noexcept
{
    _renderSessionStats.SubmitStipple ( mesh->GetVertexCount () );

    if ( !_frustum.IsVisible ( worldBounds ) )
        return;

    _geometryPass.GetStippleSubpass ().Submit ( mesh, material, local, worldBounds, color0, color1, color2, emission );
}

void RenderSession::SubmitPointLight ( LightRef &light ) noexcept
{
    _renderSessionStats.SubmitPointLight ();

    // NOLINTNEXTLINE - downcast
    auto const& pointLight = static_cast<PointLight const&> ( *light );

    if ( !_frustum.IsVisible ( pointLight.GetBounds () ) )
        return;

    _lightPass.SubmitPointLight ( light );
}

void RenderSession::SubmitReflectionGlobal ( LightRef &light ) noexcept
{
    _renderSessionStats.RenderReflectionGlobal ();

    // NOLINTNEXTLINE - downcast
    auto& probe = static_cast<ReflectionProbeGlobal&> ( *light );

    _lightPass.SubmitReflectionGlobal ( probe.GetPrefilter () );
}

void RenderSession::SubmitReflectionLocal ( LightRef &light ) noexcept
{
    _renderSessionStats.SubmitReflectionLocal ();

    // NOLINTNEXTLINE - downcast
    auto& probe = static_cast<ReflectionProbeLocal&> ( *light );

    if ( !_frustum.IsVisible ( probe.GetBounds () ) )
        return;

    _lightPass.SubmitReflectionLocal ( probe.GetPrefilter (), probe.GetLocation (), probe.GetSize () );
}

} // namespace pbr
