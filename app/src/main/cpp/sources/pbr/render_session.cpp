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

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkWaitForFences ( device, 1U, &_fence, VK_TRUE, std::numeric_limits<uint64_t>::max () ),
        "pbr::RenderSession::End",
        "Can't wait fence"
    );

    if ( !result )
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( vkResetFences ( device, 1U, &_fence ),
        "pbr::RenderSession::End",
        "Can't reset fence"
    );

    if ( !result )
        return false;

    result = android_vulkan::Renderer::CheckVkResult (
        vkResetCommandPool ( device, _commandPool, 0U ),
        "pbr::RenderSession::End",
        "Can't reset command pool"
    );

    if ( !result )
        return false;

    result = _lightPass.OnPreGeometryPass ( renderer,
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

    constexpr VkCommandBufferBeginInfo beginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    result = android_vulkan::Renderer::CheckVkResult ( vkBeginCommandBuffer ( _commandBuffer, &beginInfo ),
        "pbr::RenderSession::End",
        "Can't begin main render pass"
    );

    if ( !result )
        return false;

    vkCmdBeginRenderPass ( _commandBuffer, &_renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

    result = _geometryPass.Execute ( renderer,
        _commandBuffer,
        _frustum,
        _view,
        _viewProjection,
        _defaultTextureManager,
        _renderSessionStats
    );

    if ( !result )
        return false;

    vkCmdNextSubpass ( _commandBuffer, VK_SUBPASS_CONTENTS_INLINE );

    result = _lightPass.OnPostGeometryPass ( renderer,
        _commandBuffer,
        _viewerLocal,
        _view,
        _viewProjection
    );

    if ( !result )
        return false;

    vkCmdEndRenderPass ( _commandBuffer );

    _renderSessionStats.RenderPointLights ( _lightPass.GetPointLightCount () );
    _renderSessionStats.RenderReflectionLocal ( _lightPass.GetReflectionLocalCount () );

    if ( !_presentPass.Execute ( renderer, _commandBuffer, _gBufferSlotMapper, _fence ) )
        return false;

    _renderSessionStats.PrintStats ( deltaTime );
    return true;
}

void RenderSession::FreeTransferResources ( VkDevice device ) noexcept
{
    _defaultTextureManager.FreeTransferResources ( device, _commandPool );
}

bool RenderSession::OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept
{
    _lightHandlers[ static_cast<size_t> ( eLightType::PointLight ) ] = &RenderSession::SubmitPointLight;
    _lightHandlers[ static_cast<size_t> ( eLightType::ReflectionGlobal ) ] = &RenderSession::SubmitReflectionGlobal;
    _lightHandlers[ static_cast<size_t> ( eLightType::ReflectionLocal ) ] = &RenderSession::SubmitReflectionLocal;

    _meshHandlers[ static_cast<size_t> ( eMaterialType::Opaque ) ] = &RenderSession::SubmitOpaqueCall;
    _meshHandlers[ static_cast<size_t> ( eMaterialType::Stipple ) ] = &RenderSession::SubmitStippleCall;

    VkCommandPoolCreateInfo const createInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    VkDevice device = renderer.GetDevice ();

    constexpr VkFenceCreateInfo fenceInfo
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    bool result = android_vulkan::Renderer::CheckVkResult ( vkCreateFence ( device, &fenceInfo, nullptr, &_fence ),
        "pbr::RenderSession::OnInitDevice",
        "Can't create fence"
    );

    if ( !result )
        return false;

    AV_REGISTER_FENCE ( "pbr::RenderSession::_fence" )

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( device, &createInfo, nullptr, &_commandPool ),
        "pbr::RenderSession::OnInitDevice",
        "Can't create lead command pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_COMMAND_POOL ( "pbr::RenderSession::_commandPool" )

    VkCommandBufferAllocateInfo const bufferAllocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = _commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1U
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &bufferAllocateInfo, &_commandBuffer ),
        "pbr::RenderSession::OnInitDevice",
        "Can't allocate command buffer"
    );

    if ( !result )
        return false;

    if ( !_texturePresentDescriptorSetLayout.Init ( renderer.GetDevice () ) )
        return false;

    if ( !_defaultTextureManager.Init ( renderer, _commandPool ) )
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

    if ( _renderPass != VK_NULL_HANDLE )
    {
        vkDestroyRenderPass ( device, _renderPass, nullptr );
        _renderPass = VK_NULL_HANDLE;
        AV_UNREGISTER_RENDER_PASS ( "pbr::RenderSession::_renderPass" )
    }

    if ( _commandPool != VK_NULL_HANDLE )
    {
        vkDestroyCommandPool ( device, _commandPool, nullptr );
        _commandPool = VK_NULL_HANDLE;
        AV_UNREGISTER_COMMAND_POOL ( "pbr::RenderSession::_commandPool" )
    }

    if ( _fence == VK_NULL_HANDLE )
        return;

    vkDestroyFence ( device, _fence, nullptr );
    _fence = VK_NULL_HANDLE;
    AV_UNREGISTER_FENCE ( "pbr::RenderSession::_fence" )
}

bool RenderSession::OnSwapchainCreated ( android_vulkan::Renderer &renderer,
    VkExtent2D const &resolution
) noexcept
{
    VkExtent2D const& currentResolution = _gBuffer.GetResolution ();

    if ( ( currentResolution.width != resolution.width ) | ( currentResolution.height != resolution.height ) )
    {
        DestroyGBufferResources ( renderer.GetDevice () );

        if ( !CreateGBufferResources ( renderer, resolution ) )
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

bool RenderSession::CreateFramebuffer ( android_vulkan::Renderer &renderer ) noexcept
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

    constexpr static VkSubpassDescription const subpasses[] =
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
        },
        {
            .srcSubpass = 1U,
            .dstSubpass = VK_SUBPASS_EXTERNAL,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
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
                .float32 = { 0.0F, 0.0F, 0.0F, 0.0F }
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

    if ( !_gBuffer.Init ( renderer, resolution ) || !CreateRenderPass ( renderer ) || !CreateFramebuffer ( renderer ) )
        return false;

    bool result = _geometryPass.Init ( renderer,
        _commandPool,
        _gBuffer.GetResolution (),
        _renderPass,
        _samplerManager
    );

    if ( !result )
        return false;

    if ( !_lightPass.Init ( renderer, _commandPool, _renderPass, _gBuffer ) )
        return false;

    if ( !CreateGBufferSlotMapper ( renderer ) )
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( vkDeviceWaitIdle ( device ),
        "pbr::RenderSession::CreateGBufferResources",
        "Can't wait device idle"
    );

    if ( !result )
        return false;

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
    if ( _framebuffer != VK_NULL_HANDLE )
    {
        vkDestroyFramebuffer ( device, _framebuffer, nullptr );
        _framebuffer = VK_NULL_HANDLE;
        AV_UNREGISTER_FRAMEBUFFER ( "pbr::RenderSession::_framebuffer" )
    }

    _lightPass.Destroy ( device );

    if ( _gBufferDescriptorPool != VK_NULL_HANDLE )
    {
        vkDestroyDescriptorPool ( device, _gBufferDescriptorPool, nullptr );
        _gBufferDescriptorPool = VK_NULL_HANDLE;
        AV_UNREGISTER_DESCRIPTOR_POOL ( "pbr::RenderSession::_gBufferDescriptorPool" )
    }

    _geometryPass.Destroy ( device );
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
