#include <precompiled_headers.hpp>
#include <pbr/animation_graph.hpp>
#include <pbr/render_session.hpp>
#include <pbr/point_light.hpp>
#include <pbr/reflection_probe_global.hpp>
#include <pbr/reflection_probe_local.hpp>
#include <pbr/skeletal_mesh_component.hpp>
#include <av_assert.hpp>
#include <logger.hpp>
#include <trace.hpp>
#include <vulkan_utils.hpp>


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

    size_t const commandBufferIndex = _writingCommandInfo;
    CommandInfo &commandInfo = _commandInfo[ _writingCommandInfo ];
    _writingCommandInfo = ++_writingCommandInfo % DUAL_COMMAND_BUFFER;

    VkFence &fence = commandInfo._fence;
    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkWaitForFences ( device, 1U, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max () ),
        "pbr::RenderSession::End",
        "Can't wait fence"
    );

    if ( !result ) [[unlikely]]
        return false;

    result = android_vulkan::Renderer::CheckVkResult (
        _presentRenderPass.AcquirePresentTarget ( renderer, commandInfo._acquire ),
        "pbr::RenderSession::End",
        "Can't acquire present image"
    );

    if ( !result ) [[unlikely]]
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( vkResetFences ( device, 1U, &fence ),
        "pbr::RenderSession::End",
        "Can't reset fence"
    );

    if ( !result ) [[unlikely]]
        return false;

    result = android_vulkan::Renderer::CheckVkResult (
        vkResetCommandPool ( device, commandInfo._pool, 0U ),
        "pbr::RenderSession::End",
        "Can't reset command pool"
    );

    if ( !result ) [[unlikely]]
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

    if ( !result || ( _brightnessChanged && !UpdateBrightness ( renderer ) ) ) [[unlikely]]
        return false;

    AnimationGraph::UploadGPUData ( commandBuffer, commandBufferIndex );

    if ( !_uiPass.UploadGPUData ( renderer, commandBuffer, commandBufferIndex ) ) [[unlikely]]
        return false;

    if ( !SkeletalMeshComponent::ApplySkin ( commandBuffer, commandBufferIndex ) ) [[unlikely]]
        return false;

    _toneMapperPass.UploadGPUData ( renderer, commandBuffer );

    result = _lightPass.OnPreGeometryPass ( renderer,
        commandBuffer,
        commandBufferIndex,
        _gBuffer.GetResolution (),
        _geometryPass.GetOpaqueSubpass ().GetSceneData (),
        _opaqueMeshCount,
        _viewerLocal,
        _view,
        _viewProjection,
        _cvvToView
    );

    if ( !result ) [[unlikely]]
        return false;

    _geometryPass.UploadGPUData ( device, commandBuffer, _frustum, _view, _viewProjection );

    vkCmdBeginRenderPass ( commandBuffer, &_renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

    _geometryPass.Execute ( commandBuffer, _renderSessionStats );

    vkCmdNextSubpass ( commandBuffer, VK_SUBPASS_CONTENTS_INLINE );

    _lightPass.OnPostGeometryPass ( device, commandBuffer, commandBufferIndex );

    vkCmdEndRenderPass ( commandBuffer );

    _exposurePass.Execute ( commandBuffer, static_cast<float> ( deltaTime ) );

    _presentRenderPass.Begin ( commandBuffer );

    _toneMapperPass.Execute ( commandBuffer );

    if ( !_uiPass.Execute ( commandBuffer, commandBufferIndex ) ) [[unlikely]]
        return false;

    std::optional<VkResult> const presentResult = _presentRenderPass.End ( renderer,
        commandBuffer,
        commandInfo._acquire,
        fence,
        nullptr
    );

    if ( !presentResult ) [[unlikely]]
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( *presentResult,
        "pbr::RenderSession::End",
        "Can't present frame"
    );

    if ( !result ) [[unlikely]]
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
    _exposurePass.FreeTransferResources ( renderer.GetDevice (), pool );
}

UIPass &RenderSession::GetUIPass () noexcept
{
    return _uiPass;
}

size_t RenderSession::GetWritingCommandBufferIndex () const noexcept
{
    return _writingCommandInfo;
}

bool RenderSession::OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    _lightHandlers[ static_cast<size_t> ( eLightType::PointLight ) ] = &RenderSession::SubmitPointLight;
    _lightHandlers[ static_cast<size_t> ( eLightType::ReflectionGlobal ) ] = &RenderSession::SubmitReflectionGlobal;
    _lightHandlers[ static_cast<size_t> ( eLightType::ReflectionLocal ) ] = &RenderSession::SubmitReflectionLocal;

    _meshHandlers[ static_cast<size_t> ( eMaterialType::Opaque ) ] = &RenderSession::SubmitOpaqueCall;
    _meshHandlers[ static_cast<size_t> ( eMaterialType::Stipple ) ] = &RenderSession::SubmitStippleCall;

    constexpr size_t initFrameInFlightIndex = 0U;
    CommandInfo &commandInfo = _commandInfo[ initFrameInFlightIndex ];
    VkDevice device = renderer.GetDevice ();

    if ( !AllocateCommandInfo ( commandInfo, device, renderer.GetQueueFamilyIndex (), initFrameInFlightIndex ) )
    {
        [[unlikely]]
        return false;
    }

    return _exposurePass.Init ( renderer, commandInfo._pool ) &&
        _toneMapperPass.Init ( renderer ) &&
        _defaultTextureManager.Init ( renderer, commandInfo._pool ) &&
        _samplerManager.Init ( device ) &&
        _presentRenderPass.OnInitDevice () &&
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

    if ( _renderPassInfo.renderPass != VK_NULL_HANDLE )
    {
        vkDestroyRenderPass ( device, _renderPassInfo.renderPass, nullptr );
        _renderPassInfo.renderPass = VK_NULL_HANDLE;
    }

    for ( auto &commandInfo : _commandInfo )
    {
        if ( commandInfo._pool != VK_NULL_HANDLE ) [[likely]]
        {
            vkDestroyCommandPool ( device, commandInfo._pool, nullptr );
            commandInfo._pool = VK_NULL_HANDLE;
        }

        if ( commandInfo._acquire != VK_NULL_HANDLE ) [[likely]]
        {
            vkDestroySemaphore ( device, commandInfo._acquire, nullptr );
            commandInfo._acquire = VK_NULL_HANDLE;
        }

        if ( commandInfo._fence == VK_NULL_HANDLE ) [[unlikely]]
            continue;

        vkDestroyFence ( device, commandInfo._fence, nullptr );
        commandInfo._fence = VK_NULL_HANDLE;
    }

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

        if ( !CreateGBufferResources ( renderer, newResolution ) ) [[unlikely]]
            return false;

        android_vulkan::LogInfo ( "pbr::RenderSession::OnSwapchainCreated - G-buffer resolution is %u x %u.",
            newResolution.width,
            newResolution.height
        );
    }

    VkDevice device = renderer.GetDevice ();
    uint32_t const queueIndex = renderer.GetQueueFamilyIndex ();

    for ( size_t i = 0U; i < DUAL_COMMAND_BUFFER; ++i )
    {
        CommandInfo &commandInfo = _commandInfo[ i ];

        if ( commandInfo._pool != VK_NULL_HANDLE )
            continue;

        if ( !AllocateCommandInfo ( commandInfo, device, queueIndex, i ) ) [[unlikely]]
        {
            return false;
        }
    }

    if ( !_presentRenderPass.OnSwapchainCreated ( renderer ) ) [[unlikely]]
        return false;

    constexpr uint32_t subpass = PresentRenderPass::GetSubpass ();
    VkImageView hdrView = _gBuffer.GetHDRAccumulator ().GetImageView ();
    VkRenderPass renderPass = _presentRenderPass.GetRenderPass ();

    if ( !_uiPass.OnSwapchainCreated ( renderer, renderPass, subpass ) ) [[unlikely]]
        return false;

    if ( !hasChanges ) [[likely]]
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

void RenderSession::SetExposureCompensation ( float exposureValue ) noexcept
{
    _exposurePass.SetExposureCompensation ( exposureValue );
}

void RenderSession::SetExposureMaximumBrightness ( float exposureValues ) noexcept
{
    _exposurePass.SetMaximumBrightness ( exposureValues );
}

void RenderSession::SetExposureMinimumBrightness ( float exposureValues ) noexcept
{
    _exposurePass.SetMinimumBrightness ( exposureValues );
}

void RenderSession::SetEyeAdaptationSpeed ( float speed ) noexcept
{
    _exposurePass.SetEyeAdaptationSpeed ( speed );
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
    GeometryPassProgram::ColorData const &colorData
) noexcept
{
    auto const idx = static_cast<size_t> ( material->GetMaterialType () );
    AV_ASSERT ( idx < std::size ( _meshHandlers ) )

    MeshHandler const handler = _meshHandlers[ idx ];

    // Calling method by pointer C++ syntax.
    ( this->*handler ) ( mesh, material, local, worldBounds, colorData );
}

bool RenderSession::CreateFramebuffer ( VkDevice device ) noexcept
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
        .renderPass = _renderPassInfo.renderPass,
        .attachmentCount = std::size ( attachments ),
        .pAttachments = attachments,
        .width = resolution.width,
        .height = resolution.height,
        .layers = 1U
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateFramebuffer ( device, &framebufferInfo, nullptr, &_renderPassInfo.framebuffer ),
        "pbr::RenderSession::CreateFramebuffer",
        "Can't create GBuffer framebuffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    _renderPassInfo.renderArea.extent = resolution;

    AV_SET_VULKAN_OBJECT_NAME ( device, _renderPassInfo.framebuffer, VK_OBJECT_TYPE_FRAMEBUFFER, "G-Buffer" )
    return true;
}

bool RenderSession::CreateRenderPass ( VkDevice device ) noexcept
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

    constexpr static VkAttachmentReference const geometryColorReferences[]
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

    constexpr static VkAttachmentReference const lightColorReferences[]
    {
        // #1: HDR accumulator
        {
            .attachment = 1U,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        }
    };

    constexpr static VkAttachmentReference const lightInputAttachments[]
    {
        // #0: albedo
        {
            .attachment = 0U,
            .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },

        // #1: normal
        {
            .attachment = 2U,
            .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },

        // #2: params
        {
            .attachment = 3U,
            .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        },

        // #3: depth
        {
            .attachment = 4U,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
        }
    };

    constexpr VkSubpassDescription const subpasses[]
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

    constexpr VkSubpassDependency const dependencies[]
    {
        {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0U,

            .srcStageMask = AV_VK_FLAG ( VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT ) |
                AV_VK_FLAG ( VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT ),

            .dstStageMask = AV_VK_FLAG ( VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT ) |
                AV_VK_FLAG ( VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT ),

            .srcAccessMask = AV_VK_FLAG ( VK_ACCESS_SHADER_READ_BIT ) |
                AV_VK_FLAG ( VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT ),

            .dstAccessMask = AV_VK_FLAG ( VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT ) |
                 AV_VK_FLAG ( VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT ),

            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
        },
        {
            .srcSubpass = 0U,
            .dstSubpass = 1U,

            .srcStageMask = AV_VK_FLAG ( VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT ) |
                AV_VK_FLAG ( VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT ),

            .dstStageMask = AV_VK_FLAG ( VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT ) |
                AV_VK_FLAG ( VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT ),

            .srcAccessMask = AV_VK_FLAG ( VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT ) |
                AV_VK_FLAG ( VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT ),

            .dstAccessMask = AV_VK_FLAG ( VK_ACCESS_INPUT_ATTACHMENT_READ_BIT ) |
                AV_VK_FLAG ( VK_ACCESS_COLOR_ATTACHMENT_READ_BIT ) |
                AV_VK_FLAG ( VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT ),

            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
        },
        {
            .srcSubpass = 1U,
            .dstSubpass = VK_SUBPASS_EXTERNAL,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
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
        vkCreateRenderPass ( device, &renderPassInfo, nullptr, &_renderPassInfo.renderPass ),
        "pbr::RenderSession::CreateRenderPass",
        "Can't create render pass"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _renderPassInfo.renderPass, VK_OBJECT_TYPE_RENDER_PASS, "Render session" )

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
        CreateRenderPass ( device ) &&
        CreateFramebuffer ( device ) &&

        _geometryPass.Init ( renderer,
            _gBuffer.GetResolution (),
            _renderPassInfo.renderPass,
            _samplerManager,
            _defaultTextureManager
        ) &&

        _lightPass.Init ( renderer, _renderPassInfo.renderPass, _gBuffer ) &&
        _exposurePass.SetTarget ( renderer, _gBuffer.GetHDRAccumulator () ) &&

        android_vulkan::Renderer::CheckVkResult ( vkDeviceWaitIdle ( device ),
            "pbr::RenderSession::CreateGBufferResources",
            "Can't wait device idle"
        );

    if ( !result ) [[unlikely]]
        return false;

    _lightPass.OnFreeTransferResources ( renderer );
    return true;
}

void RenderSession::DestroyGBufferResources ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    if ( _renderPassInfo.framebuffer != VK_NULL_HANDLE ) [[likely]]
    {
        vkDestroyFramebuffer ( device, _renderPassInfo.framebuffer, nullptr );
        _renderPassInfo.framebuffer = VK_NULL_HANDLE;
    }

    _lightPass.Destroy ( renderer );
    _geometryPass.Destroy ( renderer );
    _gBuffer.Destroy ( renderer );
}

void RenderSession::SubmitOpaqueCall ( MeshRef &mesh,
    MaterialRef const &material,
    GXMat4 const &local,
    GXAABB const &worldBounds,
    GeometryPassProgram::ColorData const &colorData
) noexcept
{
    ++_opaqueMeshCount;
    _renderSessionStats.SubmitOpaque ( mesh->GetVertexCount () );
    _geometryPass.GetOpaqueSubpass ().Submit ( mesh, material, local, worldBounds, colorData );
}

void RenderSession::SubmitStippleCall ( MeshRef &mesh,
    MaterialRef const &material,
    GXMat4 const &local,
    GXAABB const &worldBounds,
    GeometryPassProgram::ColorData const &colorData
) noexcept
{
    _renderSessionStats.SubmitStipple ( mesh->GetVertexCount () );

    if ( !_frustum.IsVisible ( worldBounds ) )
        return;

    _geometryPass.GetStippleSubpass ().Submit ( mesh, material, local, worldBounds, colorData );
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

    if ( !result ) [[unlikely]]
        return false;

    VkRenderPass renderPass = _presentRenderPass.GetRenderPass ();
    constexpr uint32_t subpass = PresentRenderPass::GetSubpass ();

    result = _toneMapperPass.SetBrightness ( renderer, renderPass, subpass, _brightnessBalance ) &&
        _uiPass.SetBrightness ( renderer, renderPass, subpass, _brightnessBalance );

    if ( !result ) [[unlikely]]
        return false;

    _brightnessChanged = false;
    return true;
}

bool RenderSession::AllocateCommandInfo ( CommandInfo &info,
    VkDevice device,
    uint32_t queueIndex,
    [[maybe_unused]] size_t frameInFlightIndex
) noexcept
{
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

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, info._fence, VK_OBJECT_TYPE_FENCE, "Frame in flight #%zu", frameInFlightIndex )

    constexpr VkSemaphoreCreateInfo semaphoreInfo
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &info._acquire ),
        "pbr::RenderSession::AllocateCommandInfo",
        "Can't create render target acquired semaphore"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device,
        info._acquire,
        VK_OBJECT_TYPE_SEMAPHORE,
        "Frame in flight #%zu",
        frameInFlightIndex
    )

    VkCommandPoolCreateInfo const createInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        .queueFamilyIndex = queueIndex
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( device, &createInfo, nullptr, &info._pool ),
        "pbr::RenderSession::AllocateCommandInfo",
        "Can't create lead command pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device,
        info._pool,
        VK_OBJECT_TYPE_COMMAND_POOL,
        "Frame in flight #%zu",
        frameInFlightIndex
    )

    VkCommandBufferAllocateInfo const bufferAllocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = info._pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1U
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &bufferAllocateInfo, &info._buffer ),
        "pbr::RenderSession::AllocateCommandInfo",
        "Can't allocate command buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device,
        info._buffer,
        VK_OBJECT_TYPE_COMMAND_BUFFER,
        "Frame in flight #%zu",
        frameInFlightIndex
    )

    return true;
}

} // namespace pbr
