#include <pbr/render_session.hpp>
#include <pbr/point_light.hpp>
#include <pbr/reflection_probe_global.hpp>
#include <pbr/reflection_probe_local.hpp>
#include <av_assert.hpp>
#include <trace.hpp>
#include <vulkan_utils.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <algorithm>

GX_RESTORE_WARNING_STATE


namespace pbr {

void RenderSession::Begin ( GXMat4 const &viewerLocal, GXMat4 const &projection ) noexcept
{
    AV_TRACE ( "Begin render session" )

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
    AV_TRACE ( "End render session" )

    size_t swapchainImageIndex;

    if ( !_presentRenderPass.AcquirePresentTarget ( renderer, swapchainImageIndex ) )
        return false;

    CommandInfo &commandInfo = _commandInfo[ swapchainImageIndex ];
    VkFence &fence = commandInfo._fence;
    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkWaitForFences ( device, 1U, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max () ),
        "pbr::RenderSession::End",
        "Can't wait fence"
    );

    if ( !result )
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( vkResetFences ( device, 1U, &fence ),
        "pbr::RenderSession::End",
        "Can't reset fence"
    );

    if ( !result )
        return false;

    result = android_vulkan::Renderer::CheckVkResult (
        vkResetCommandPool ( device, commandInfo._pool, 0U ),
        "pbr::RenderSession::End",
        "Can't reset command pool"
    );

    if ( !result )
        return false;

    VkCommandBuffer commandBuffer = commandInfo._buffer;

    constexpr VkCommandBufferBeginInfo beginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    result = android_vulkan::Renderer::CheckVkResult ( vkBeginCommandBuffer ( commandBuffer, &beginInfo ),
        "pbr::RenderSession::End",
        "Can't begin main render pass"
    );

    if ( !result || ( _brightnessChanged && !UpdateBrightness ( renderer ) ) )
        return false;

    if ( !_uiPass.UploadGPUData ( renderer, commandBuffer, swapchainImageIndex ) )
        return false;

    _toneMapperPass.UploadGPUData ( renderer, commandBuffer );

    result = _lightPass.OnPreGeometryPass ( renderer,
        commandBuffer,
        swapchainImageIndex,
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

    _geometryPass.UploadGPUData ( device, commandBuffer, _frustum, _view, _viewProjection );

    vkCmdBeginRenderPass ( commandBuffer, &_renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

    _geometryPass.Execute ( commandBuffer, _renderSessionStats );

    vkCmdNextSubpass ( commandBuffer, VK_SUBPASS_CONTENTS_INLINE );

    _lightPass.OnPostGeometryPass ( device, commandBuffer, swapchainImageIndex );

    vkCmdEndRenderPass ( commandBuffer );

    _exposurePass.Execute ( commandBuffer, static_cast<float> ( deltaTime ) );

    _presentRenderPass.Begin ( commandBuffer );

    _toneMapperPass.Execute ( commandBuffer );

    result = _uiPass.Execute ( commandBuffer, swapchainImageIndex ) &&
        _presentRenderPass.End ( renderer, commandBuffer, fence );

    if ( !result )
        return false;

    _renderSessionStats.RenderPointLights ( _lightPass.GetPointLightCount () );
    _renderSessionStats.RenderReflectionLocal ( _lightPass.GetReflectionLocalCount () );
    _renderSessionStats.SubmitUIVertices ( _uiPass.GetUsedVertexCount () );

    _renderSessionStats.PrintStats ( deltaTime );
    return true;
}

void RenderSession::FreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept
{
    VkCommandPool pool = _commandInfo[ 0U ]._pool;
    _defaultTextureManager.FreeTransferResources ( renderer, pool );
    _exposurePass.FreeTransferResources ( renderer, pool );
}

UIPass &RenderSession::GetUIPass () noexcept
{
    return _uiPass;
}

bool RenderSession::OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    _lightHandlers[ static_cast<size_t> ( eLightType::PointLight ) ] = &RenderSession::SubmitPointLight;
    _lightHandlers[ static_cast<size_t> ( eLightType::ReflectionGlobal ) ] = &RenderSession::SubmitReflectionGlobal;
    _lightHandlers[ static_cast<size_t> ( eLightType::ReflectionLocal ) ] = &RenderSession::SubmitReflectionLocal;

    _meshHandlers[ static_cast<size_t> ( eMaterialType::Opaque ) ] = &RenderSession::SubmitOpaqueCall;
    _meshHandlers[ static_cast<size_t> ( eMaterialType::Stipple ) ] = &RenderSession::SubmitStippleCall;

    _commandInfo.resize ( 1U );
    CommandInfo &commandInfo = _commandInfo[ 0U ];

    VkDevice device = renderer.GetDevice ();

    if ( !AllocateCommandInfo ( commandInfo, device, renderer.GetQueueFamilyIndex () ) )
        return false;

    return _exposurePass.Init ( renderer, commandInfo._pool ) &&
        _toneMapperPass.Init ( renderer ) &&
        _defaultTextureManager.Init ( renderer, commandInfo._pool ) &&
        _samplerManager.Init ( device ) &&
        _presentRenderPass.OnInitDevice ( renderer ) &&
        _uiPass.OnInitDevice ( renderer, _samplerManager, _defaultTextureManager.GetTransparent ()->GetImageView () );
}

void RenderSession::OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    std::memset ( _lightHandlers, 0, sizeof ( _lightHandlers ) );
    std::memset ( _meshHandlers, 0, sizeof ( _meshHandlers ) );

    VkDevice device = renderer.GetDevice ();

    _toneMapperPass.Destroy ( renderer );
    _exposurePass.Destroy ( renderer );
    _samplerManager.Destroy ( device );
    _defaultTextureManager.Destroy ( renderer );
    DestroyGBufferResources ( renderer );

    if ( _renderPass != VK_NULL_HANDLE )
    {
        vkDestroyRenderPass ( device, _renderPass, nullptr );
        _renderPass = VK_NULL_HANDLE;
        AV_UNREGISTER_RENDER_PASS ( "pbr::RenderSession::_renderPass" )
    }

    for ( auto &commandInfo : _commandInfo )
    {
        if ( commandInfo._pool != VK_NULL_HANDLE )
        {
            vkDestroyCommandPool ( device, commandInfo._pool, nullptr );
            AV_UNREGISTER_COMMAND_POOL ( "pbr::RenderSession::_commandInfo::_pool" )
        }

        if ( commandInfo._fence == VK_NULL_HANDLE )
            continue;

        vkDestroyFence ( device, commandInfo._fence, nullptr );
        AV_UNREGISTER_FENCE ( "pbr::RenderSession::_fence" )
    }

    _commandInfo.clear ();
    _commandInfo.shrink_to_fit ();

    _uiPass.OnDestroyDevice ( renderer );
    _presentRenderPass.OnDestroyDevice ( device );
}

bool RenderSession::OnSwapchainCreated ( android_vulkan::Renderer &renderer,
    VkExtent2D const &resolution
) noexcept
{
    VkExtent2D const newResolution = ExposurePass::AdjustResolution ( resolution );
    VkExtent2D const &currentResolution = _gBuffer.GetResolution ();

    bool const hasChanges = ( currentResolution.width != newResolution.width ) |
        ( currentResolution.height != newResolution.height );

    if ( hasChanges )
    {
        DestroyGBufferResources ( renderer );

        if ( !CreateGBufferResources ( renderer, newResolution ) )
            return false;

        android_vulkan::LogInfo ( "pbr::RenderSession::OnSwapchainCreated - G-buffer resolution is %u x %u.",
            newResolution.width,
            newResolution.height
        );
    }

    size_t const imageCount = renderer.GetPresentImageCount ();
    size_t const commandPoolCount = _commandInfo.size ();
    VkDevice device = renderer.GetDevice ();

    if ( commandPoolCount < imageCount )
    {
        _commandInfo.resize ( imageCount );
        uint32_t const queueIndex = renderer.GetQueueFamilyIndex ();

        for ( size_t i = commandPoolCount; i < imageCount; ++i )
        {
            if ( !AllocateCommandInfo ( _commandInfo[ i ], device, queueIndex ) )
            {
                return false;
            }
        }
    }

    if ( !_presentRenderPass.OnSwapchainCreated ( renderer ) )
        return false;

    constexpr uint32_t subpass = PresentRenderPass::GetSubpass ();
    VkImageView hdrView = _gBuffer.GetHDRAccumulator ().GetImageView ();
    VkRenderPass renderPass = _presentRenderPass.GetRenderPass ();

    if ( !_uiPass.OnSwapchainCreated ( renderer, renderPass, subpass, hdrView ) )
        return false;

    if ( !hasChanges )
        return true;

    return _toneMapperPass.SetTarget ( renderer,
        renderPass,
        subpass,
        hdrView,
        _exposurePass.GetExposure (),
        _samplerManager.GetClampToEdgeSampler ()
    );
}

void RenderSession::OnSwapchainDestroyed ( VkDevice device ) noexcept
{
    _presentRenderPass.OnSwapchainDestroyed ( device );
    _uiPass.OnSwapchainDestroyed ();
}

void RenderSession::SetBrightness ( float brightnessBalance ) noexcept
{
    _brightnessChanged = true;
    _brightnessBalance = brightnessBalance;
}

void RenderSession::SubmitLight ( LightRef &light ) noexcept
{
    auto const idx = static_cast<size_t> ( light->GetType () );
    AV_ASSERT ( idx < std::size ( _lightHandlers ) )

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
    AV_ASSERT ( idx < std::size ( _meshHandlers ) )

    MeshHandler const handler = _meshHandlers[ idx ];

    // Calling method by pointer C++ syntax.
    ( this->*handler ) ( mesh, material, local, worldBounds, color0, color1, color2, emission );
}

bool RenderSession::CreateFramebuffer ( android_vulkan::Renderer &renderer ) noexcept
{
    VkExtent2D const &resolution = _gBuffer.GetResolution ();

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
        .renderPass = _renderPass,
        .attachmentCount = std::size ( attachments ),
        .pAttachments = attachments,
        .width = resolution.width,
        .height = resolution.height,
        .layers = 1U
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateFramebuffer ( renderer.GetDevice (), &framebufferInfo, nullptr, &_framebuffer ),
        "pbr::RenderSession::CreateFramebuffer",
        "Can't create GBuffer framebuffer"
    );

    if ( !result )
        return false;

    _renderPassInfo.framebuffer = _framebuffer;
    _renderPassInfo.renderArea.extent = resolution;

    AV_REGISTER_FRAMEBUFFER ( "pbr::RenderSession::_framebuffer" )
    return true;
}

bool RenderSession::CreateRenderPass ( android_vulkan::Renderer &renderer ) noexcept
{
    VkAttachmentDescription const attachments[]
    {
        // #0: albedo
        {
            .flags = 0U,
            .format = _gBuffer.GetAlbedo ().GetFormat (),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
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
            .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },

        // #2: normal
        {
            .flags = 0U,
            .format = _gBuffer.GetNormal ().GetFormat (),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },

        // #3: params
        {
            .flags = 0U,
            .format = _gBuffer.GetParams ().GetFormat (),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },

        // #4: depth|stencil
        {
            .flags = 0U,
            .format = _gBuffer.GetDepthStencil ().GetFormat (),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
        }
    };

    constexpr static VkAttachmentReference const geometryColorReferences[] =
    {
        // #0: albedo
        {
            .attachment = 0U,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        },

        // #1: emission
        {
            .attachment = 1U,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        },

        // #2: normal
        {
            .attachment = 2U,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        },

        // #3: params
        {
            .attachment = 3U,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        }
    };

    // depth|stencil
    constexpr static VkAttachmentReference geometryDepthStencilReference
    {
        .attachment = 4U,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    constexpr static VkAttachmentReference const lightColorReferences[] =
    {
        // #1: HDR accumulator
        {
            .attachment = 1U,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        }
    };

    constexpr static VkAttachmentReference const lightInputAttachments[] =
    {
        // #0: albedo
        {
            .attachment = 0U,
            .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },

        // #2: normal
        {
            .attachment = 2U,
            .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },

        // #3: params
        {
            .attachment = 3U,
            .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },

        // #4: depth
        {
            .attachment = 4U,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
        }
    };

    constexpr VkSubpassDescription const subpasses[] =
    {
        {
            .flags = 0U,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = 0U,
            .pInputAttachments = nullptr,
            .colorAttachmentCount = static_cast<uint32_t> ( std::size ( geometryColorReferences ) ),
            .pColorAttachments = geometryColorReferences,
            .pResolveAttachments = nullptr,
            .pDepthStencilAttachment = &geometryDepthStencilReference,
            .preserveAttachmentCount = 0U,
            .pPreserveAttachments = nullptr
        },
        {
            .flags = 0U,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = static_cast<uint32_t> ( std::size ( lightInputAttachments ) ),
            .pInputAttachments = lightInputAttachments,
            .colorAttachmentCount = static_cast<uint32_t> ( std::size ( lightColorReferences ) ),
            .pColorAttachments = lightColorReferences,
            .pResolveAttachments = nullptr,
            .pDepthStencilAttachment = nullptr,
            .preserveAttachmentCount = 0U,
            .pPreserveAttachments = nullptr
        }
    };

    constexpr VkSubpassDependency const dependencies[] =
    {
        {
            .srcSubpass = 0U,
            .dstSubpass = 1U,

            .srcStageMask = AV_VK_FLAG ( VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT ) |
                AV_VK_FLAG ( VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT ),

            .dstStageMask = AV_VK_FLAG ( VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT ) |
                AV_VK_FLAG ( VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT ) |
                AV_VK_FLAG ( VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT ) |
                AV_VK_FLAG ( VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT ),

            .srcAccessMask = AV_VK_FLAG ( VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT ) |
                AV_VK_FLAG ( VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT ),

            .dstAccessMask = AV_VK_FLAG ( VK_ACCESS_INPUT_ATTACHMENT_READ_BIT ) |
                AV_VK_FLAG ( VK_ACCESS_COLOR_ATTACHMENT_READ_BIT ) |
                AV_VK_FLAG ( VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT ) |
                AV_VK_FLAG ( VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT ) |
                AV_VK_FLAG ( VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT ),

            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
        },
        {
            .srcSubpass = 1U,
            .dstSubpass = VK_SUBPASS_EXTERNAL,

            .srcStageMask = AV_VK_FLAG ( VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT ) |
                AV_VK_FLAG ( VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT ),

            .dstStageMask = AV_VK_FLAG ( VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT ) |
                AV_VK_FLAG ( VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT ) |
                AV_VK_FLAG ( VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT ) |
                AV_VK_FLAG ( VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT ),

            .srcAccessMask = AV_VK_FLAG ( VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT ) |
                AV_VK_FLAG ( VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT ),

            .dstAccessMask = AV_VK_FLAG ( VK_ACCESS_SHADER_READ_BIT ) |
                AV_VK_FLAG ( VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT ) |
                AV_VK_FLAG ( VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT ),

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
        vkCreateRenderPass ( renderer.GetDevice (), &renderPassInfo, nullptr, &_renderPass ),
        "pbr::RenderSession::CreateRenderPass",
        "Can't create render pass"
    );

    if ( !result )
        return false;

    AV_REGISTER_RENDER_PASS ( "pbr::RenderSession::_renderPass" )

    constexpr static VkClearValue const clearValues[] =
    {
        // albedo
        {
            .color
            {
                .float32 = { 0.0F, 0.0F, 0.0F, 0.0F }
            }
        },

        // emission
        {
            .color
            {
                .float32 = { 0.0F, 0.0F, 0.0F, 1.0F }
            }
        },

        // normal
        {
            .color
            {
                .float32 = { 0.5F, 0.5F, 0.5F, 0.0F }
            }
        },

        // param
        {
            .color
            {
                .float32 = { 0.5F, 0.5F, 0.5F, 0.0F }
            }
        },

        // depth|stencil
        {
            .depthStencil
            {
                .depth = 0.0F,
                .stencil = 0U
            }
        }
    };

    _renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    _renderPassInfo.pNext = nullptr;
    _renderPassInfo.renderPass = _renderPass;
    _renderPassInfo.clearValueCount = static_cast<uint32_t> ( std::size ( clearValues ) );
    _renderPassInfo.pClearValues = clearValues;
    _renderPassInfo.renderArea.offset.x = 0;
    _renderPassInfo.renderArea.offset.y = 0;

    return true;
}

bool RenderSession::CreateGBufferResources ( android_vulkan::Renderer &renderer,
    VkExtent2D const &resolution
) noexcept
{
    VkDevice device = renderer.GetDevice ();

    bool const result = _gBuffer.Init ( renderer, resolution ) &&
        CreateRenderPass ( renderer ) &&
        CreateFramebuffer ( renderer ) &&

        _geometryPass.Init ( renderer,
            _gBuffer.GetResolution (),
            _renderPass,
            _samplerManager,
            _defaultTextureManager
        ) &&

        _lightPass.Init ( renderer, _renderPass, _gBuffer ) &&
        _exposurePass.SetTarget ( renderer, _gBuffer.GetHDRAccumulator () ) &&

        android_vulkan::Renderer::CheckVkResult ( vkDeviceWaitIdle ( device ),
            "pbr::RenderSession::CreateGBufferResources",
            "Can't wait device idle"
        );

    if ( !result )
        return false;

    _lightPass.OnFreeTransferResources ( renderer );
    return true;
}

void RenderSession::DestroyGBufferResources ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    if ( _framebuffer != VK_NULL_HANDLE )
    {
        vkDestroyFramebuffer ( device, _framebuffer, nullptr );
        _framebuffer = VK_NULL_HANDLE;
        AV_UNREGISTER_FRAMEBUFFER ( "pbr::RenderSession::_framebuffer" )
    }

    _lightPass.Destroy ( renderer );
    _geometryPass.Destroy ( renderer );
    _gBuffer.Destroy ( renderer );
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
    auto const &pointLight = static_cast<PointLight const &> ( *light );

    if ( !_frustum.IsVisible ( pointLight.GetBounds () ) )
        return;

    _lightPass.SubmitPointLight ( light );
}

void RenderSession::SubmitReflectionGlobal ( LightRef &light ) noexcept
{
    _renderSessionStats.RenderReflectionGlobal ();

    // NOLINTNEXTLINE - downcast
    auto &probe = static_cast<ReflectionProbeGlobal &> ( *light );

    _lightPass.SubmitReflectionGlobal ( probe.GetPrefilter () );
}

void RenderSession::SubmitReflectionLocal ( LightRef &light ) noexcept
{
    _renderSessionStats.SubmitReflectionLocal ();

    // NOLINTNEXTLINE - downcast
    auto &probe = static_cast<ReflectionProbeLocal &> ( *light );

    if ( !_frustum.IsVisible ( probe.GetBounds () ) )
        return;

    _lightPass.SubmitReflectionLocal ( probe.GetPrefilter (), probe.GetLocation (), probe.GetSize () );
}

bool RenderSession::UpdateBrightness ( android_vulkan::Renderer &renderer ) noexcept
{
    bool result = android_vulkan::Renderer::CheckVkResult ( vkQueueWaitIdle ( renderer.GetQueue () ),
        "pbr::RenderSession::UpdateBrightness",
        "Can't wait queue idle"
    );

    if ( !result )
        return false;

    VkRenderPass renderPass = _presentRenderPass.GetRenderPass ();
    constexpr uint32_t subpass = PresentRenderPass::GetSubpass ();

    result = _toneMapperPass.SetBrightness ( renderer, renderPass, subpass, _brightnessBalance ) &&
        _uiPass.SetBrightness ( renderer, renderPass, subpass, _brightnessBalance );

    if ( !result )
        return false;

    _brightnessChanged = false;
    return true;
}

bool RenderSession::AllocateCommandInfo ( CommandInfo &info, VkDevice device, uint32_t queueIndex ) noexcept
{
    VkCommandPool &pool = info._pool;

    constexpr VkFenceCreateInfo fenceInfo
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    bool result = android_vulkan::Renderer::CheckVkResult ( vkCreateFence ( device, &fenceInfo, nullptr, &info._fence ),
        "pbr::RenderSession::AllocateCommandInfo",
        "Can't create fence"
    );

    if ( !result )
        return false;

    AV_REGISTER_FENCE ( "pbr::RenderSession::_fence" )

    VkCommandPoolCreateInfo const createInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        .queueFamilyIndex = queueIndex
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( device, &createInfo, nullptr, &pool ),
        "pbr::RenderSession::AllocateCommandInfo",
        "Can't create lead command pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_COMMAND_POOL ( "pbr::RenderSession::_commandInfo::_pool" )

    VkCommandBufferAllocateInfo const bufferAllocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1U
    };

    return android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &bufferAllocateInfo, &info._buffer ),
        "pbr::RenderSession::AllocateCommandInfo",
        "Can't allocate command buffer"
    );
}

} // namespace pbr
