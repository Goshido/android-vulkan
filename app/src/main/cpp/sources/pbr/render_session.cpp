#include <pbr/render_session.h>

GX_DISABLE_COMMON_WARNINGS

#include <algorithm>
#include <cassert>

GX_RESTORE_WARNING_STATE

#include <half_types.h>
#include <vulkan_utils.h>
#include <pbr/point_light.h>


namespace pbr {

RenderSession::RenderSession () noexcept:
    _frustum {},
    _gBuffer {},
    _gBufferDescriptorPool ( VK_NULL_HANDLE ),
    _gBufferFramebuffer ( VK_NULL_HANDLE ),
    _gBufferImageBarrier {},
    _gBufferRenderPass ( VK_NULL_HANDLE ),
    _gBufferSlotMapper {},
    _geometryPass {},
    _opaqueMeshCount ( 0U ),
    _texturePresentProgram {},
    _presentInfo {},
    _presentBeginInfo {},
    _presentClearValue {},
    _presentFramebuffers {},
    _presentRenderPass ( VK_NULL_HANDLE ),
    _presentRenderPassEndSemaphore ( VK_NULL_HANDLE ),
    _presentRenderTargetAcquiredSemaphore ( VK_NULL_HANDLE ),
    _renderSessionStats {},
    _samplerManager {},
    _submitInfoMain {},
    _view {},
    _viewProjection {}
{
    // NOTHING
}

void RenderSession::Begin ( GXMat4 const &view, GXMat4 const &projection )
{
    _opaqueMeshCount = 0U;
    _view = view;
    _viewProjection.Multiply ( view, projection );
    _frustum.From ( _viewProjection );
    _pointLightPass.Reset ();
    _geometryPass.Reset ();
}

bool RenderSession::End ( ePresentTarget target, double deltaTime, android_vulkan::Renderer &renderer )
{
    uint32_t framebufferIndex = UINT32_MAX;

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkAcquireNextImageKHR ( renderer.GetDevice (),
            renderer.GetSwapchain (),
            UINT64_MAX,
            _presentRenderTargetAcquiredSemaphore,
            VK_NULL_HANDLE,
            &framebufferIndex
        ),

        "RenderSession::End",
        "Can't get presentation image index"
    );

    if ( !result || !_pointLightPass.Execute ( _geometryPass.GetSceneData (), _opaqueMeshCount, renderer ) )
        return false;

    VkCommandBuffer commandBuffer = _geometryPass.Execute ( _frustum,
        _view,
        _viewProjection,
        _samplerManager,
        _renderSessionStats,
        renderer
    );

    if ( commandBuffer == VK_NULL_HANDLE )
        return false;

    android_vulkan::Texture2D* targetTexture;

    if ( target == ePresentTarget::Albedo )
    {
        targetTexture = &_gBuffer.GetAlbedo ();
    }
    else if ( target == ePresentTarget::Normal )
    {
        targetTexture = &_gBuffer.GetNormal ();
    }
    else if ( target == ePresentTarget::Param )
    {
        targetTexture = &_gBuffer.GetParams ();
    }
    else if ( target == ePresentTarget::Emission )
    {
        targetTexture = &_gBuffer.GetEmission ();
    }
    else
    {
        android_vulkan::LogWarning ( "RenderSession::End - Unknown target [%zu]. Defaulting to albedo target.",
            static_cast<int> ( target )
        );

        targetTexture = &_gBuffer.GetAlbedo ();
    }

    _gBufferImageBarrier.image = targetTexture->GetImage ();
    _gBufferImageBarrier.subresourceRange.levelCount = static_cast<uint32_t> ( targetTexture->GetMipLevelCount () );

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

    _presentBeginInfo.framebuffer = _presentFramebuffers[ framebufferIndex ];
    _presentBeginInfo.renderArea.extent = renderer.GetSurfaceSize ();

    vkCmdBeginRenderPass ( commandBuffer, &_presentBeginInfo, VK_SUBPASS_CONTENTS_INLINE );
    _texturePresentProgram.Bind ( commandBuffer );

    _texturePresentProgram.SetData ( commandBuffer,
        _gBufferSlotMapper[ static_cast<size_t> ( target ) ],
        renderer.GetPresentationEngineTransform ()
    );

    vkCmdDraw ( commandBuffer, 4U, 1U, 0U, 0U );
    vkCmdEndRenderPass ( commandBuffer );

    result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( commandBuffer ),
        "RenderSession::End",
        "Can't end command buffer"
    );

    if ( !result )
        return false;

    _submitInfoMain.pCommandBuffers = &commandBuffer;

    result = android_vulkan::Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &_submitInfoMain, _geometryPass.GetFence () ),
        "RenderSession::End",
        "Can't submit geometry render command buffer"
    );

    if ( !result )
        return false;

    VkResult presentResult = VK_ERROR_DEVICE_LOST;

    _presentInfo.pResults = &presentResult;
    _presentInfo.pSwapchains = &renderer.GetSwapchain ();
    _presentInfo.pImageIndices = &framebufferIndex;

    result = android_vulkan::Renderer::CheckVkResult ( vkQueuePresentKHR ( renderer.GetQueue (), &_presentInfo ),
        "RenderSession::EndFrame",
        "Can't present frame"
    );

    if ( !result )
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( presentResult,
        "RenderSession::EndFrame",
        "Present queue has been failed"
    );

    if ( !result )
        return false;

    _renderSessionStats.PrintStats ( deltaTime );
    return true;
}

bool RenderSession::Init ( android_vulkan::Renderer &renderer, VkExtent2D const &resolution )
{
    if ( !_gBuffer.Init ( resolution, renderer ) )
        return false;

    if ( !CreateSyncPrimitives ( renderer ) )
    {
        Destroy ( renderer );
        return false;
    }

    if ( !CreateGBufferRenderPass ( renderer ) )
    {
        Destroy ( renderer );
        return false;
    }

    if ( !CreateGBufferFramebuffer ( renderer ) )
    {
        Destroy ( renderer );
        return false;
    }

    if ( !_geometryPass.Init ( _gBuffer.GetResolution (), _gBufferRenderPass, _gBufferFramebuffer, renderer ) )
    {
        Destroy ( renderer );
        return false;
    }

    if ( !_pointLightPass.Init ( renderer ) )
    {
        Destroy ( renderer );
        return false;
    }

    if ( !CreatePresentRenderPass ( renderer ) )
    {
        Destroy ( renderer );
        return false;
    }

    if ( !CreatePresentFramebuffers ( renderer ) )
    {
        Destroy ( renderer );
        return false;
    }

    if ( !_texturePresentProgram.Init ( renderer, _presentRenderPass, renderer.GetSurfaceSize () ) )
    {
        Destroy ( renderer );
        return false;
    }

    if ( !CreateGBufferSlotMapper ( renderer ) )
    {
        Destroy ( renderer );
        return false;
    }

    InitCommonStructures ();
    return true;
}

void RenderSession::Destroy ( android_vulkan::Renderer &renderer )
{
    _samplerManager.FreeResources ( renderer );
    _pointLightPass.Destroy ( renderer );

    _texturePresentProgram.Destroy ( renderer );
    VkDevice device = renderer.GetDevice ();

    if ( _gBufferDescriptorPool != VK_NULL_HANDLE )
    {
        vkDestroyDescriptorPool ( device, _gBufferDescriptorPool, nullptr );
        _gBufferDescriptorPool = VK_NULL_HANDLE;
        AV_UNREGISTER_DESCRIPTOR_POOL ( "RenderSession::_gBufferDescriptorPool" )
    }

    DestroyPresentFramebuffers ( renderer );

    if ( _presentRenderPass != VK_NULL_HANDLE )
    {
        vkDestroyRenderPass ( device, _presentRenderPass, nullptr );
        _presentRenderPass = VK_NULL_HANDLE;
        AV_UNREGISTER_RENDER_PASS ( "RenderSession::_presentRenderPass" )
    }

    _geometryPass.Destroy ( renderer );

    if ( _gBufferFramebuffer != VK_NULL_HANDLE )
    {
        vkDestroyFramebuffer ( device, _gBufferFramebuffer, nullptr );
        _gBufferFramebuffer = VK_NULL_HANDLE;
        AV_UNREGISTER_FRAMEBUFFER ( "RenderSession::_gBufferFramebuffer" )
    }

    if ( _gBufferRenderPass != VK_NULL_HANDLE )
    {
        vkDestroyRenderPass ( renderer.GetDevice (), _gBufferRenderPass, nullptr );
        _gBufferRenderPass = VK_NULL_HANDLE;
        AV_UNREGISTER_RENDER_PASS ( "RenderSession::_gBufferRenderPass" )
    }

    DestroySyncPrimitives ( renderer );
    _gBuffer.Destroy ( renderer );
}

void RenderSession::SubmitMesh ( MeshRef &mesh,
    MaterialRef const &material,
    GXMat4 const &local,
    GXAABB const &worldBounds,
    android_vulkan::Half4 const &color0,
    android_vulkan::Half4 const &color1,
    android_vulkan::Half4 const &color2,
    android_vulkan::Half4 const &color3
)
{
    ++_opaqueMeshCount;

    if ( material->GetMaterialType() == eMaterialType::Opaque )
    {
        SubmitOpaqueCall ( mesh, material, local, worldBounds, color0, color1, color2, color3 );
    }
}

void RenderSession::SubmitLight ( LightRef const &light )
{
    if ( light->GetType () == eLightType::PointLight )
    {
        SubmitPointLight ( light );
    }
}

bool RenderSession::CreateGBufferFramebuffer ( android_vulkan::Renderer &renderer )
{
    VkExtent2D const& resolution = _gBuffer.GetResolution ();

    VkImageView const attachments[] =
    {
        _gBuffer.GetAlbedo ().GetImageView (),
        _gBuffer.GetEmission ().GetImageView (),
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

    const bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateFramebuffer ( renderer.GetDevice (), &framebufferInfo, nullptr, &_gBufferFramebuffer ),
        "RenderSession::CreateGBufferFramebuffer",
        "Can't create GBuffer framebuffer"
    );

    if ( !result )
        return false;

    AV_REGISTER_FRAMEBUFFER ( "RenderSession::_gBufferFramebuffer" )
    return true;
}

bool RenderSession::CreateGBufferRenderPass ( android_vulkan::Renderer &renderer )
{
    constexpr static VkAttachmentReference const colorReferences[] =
    {
        // albedo
        {
            .attachment = 0U,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        },

        // emission
        {
            .attachment = 1U,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        },

        // normal
        {
            .attachment = 2U,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        },

        // params
        {
            .attachment = 3U,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        }
    };

    // depth|stencil
    constexpr static VkAttachmentReference const depthStencilReference
    {
        .attachment = 4U,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    constexpr static VkSubpassDescription const subpasses[] =
    {
        {
            .flags = 0U,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = 0U,
            .pInputAttachments = nullptr,
            .colorAttachmentCount = static_cast<uint32_t> ( std::size ( colorReferences ) ),
            .pColorAttachments = colorReferences,
            .pResolveAttachments = nullptr,
            .pDepthStencilAttachment = &depthStencilReference,
            .preserveAttachmentCount = 0U,
            .pPreserveAttachments = nullptr
        }
    };

    VkAttachmentDescription const attachments[]
    {
        // albedo
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

        // emission
        {
            .flags = 0U,
            .format = _gBuffer.GetEmission ().GetFormat (),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        },

        // normal
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

        // params
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

        // depth|stencil
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
        "RenderSession::CreateGBufferRenderPass",
        "Can't create render pass"
    );

    if ( !result )
        return false;

    AV_REGISTER_RENDER_PASS ( "RenderSession::_gBufferRenderPass" )
    return true;
}

bool RenderSession::CreateGBufferSlotMapper ( android_vulkan::Renderer &renderer )
{
    constexpr static VkDescriptorPoolSize const poolSizes[] =
    {
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = static_cast<uint32_t> ( ePresentTarget::TargetCount )
        },
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = static_cast<uint32_t> ( ePresentTarget::TargetCount )
        }
    };

    constexpr static VkDescriptorPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .maxSets = static_cast<uint32_t> ( ePresentTarget::TargetCount ),
        .poolSizeCount = static_cast<uint32_t> ( std::size ( poolSizes ) ),
        .pPoolSizes = poolSizes
    };

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_gBufferDescriptorPool ),
        "RenderSession::CreateGBufferSlotMapper",
        "Can't create descriptor pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_POOL ( "RenderSession::_gBufferDescriptorPool" )

    TexturePresentDescriptorSetLayout const layout;
    VkDescriptorSetLayout nativeLayout = layout.GetLayout ();

    VkDescriptorSetLayout const layouts[ static_cast<size_t> ( ePresentTarget::TargetCount ) ] =
    {
        nativeLayout,
        nativeLayout,
        nativeLayout,
        nativeLayout
    };

    VkDescriptorSetAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _gBufferDescriptorPool,
        .descriptorSetCount = static_cast<uint32_t> ( std::size ( layouts ) ),
        .pSetLayouts = layouts
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &allocateInfo, _gBufferSlotMapper ),
        "RenderSession::CreateGBufferSlotMapper",
        "Can't allocate descriptor sets"
    );

    if ( !result )
        return false;

    SamplerRef pointSampler = _samplerManager.GetPointSampler ( renderer );
    VkSampler nativeSampler = pointSampler->GetSampler ();

    VkDescriptorImageInfo imageInfo[ static_cast<size_t> ( ePresentTarget::TargetCount ) ];
    VkDescriptorImageInfo& albedo = imageInfo[ static_cast<size_t> ( ePresentTarget::Albedo ) ];
    albedo.sampler = nativeSampler;
    albedo.imageView = _gBuffer.GetAlbedo ().GetImageView ();
    albedo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDescriptorImageInfo& emission = imageInfo[ static_cast<size_t> ( ePresentTarget::Emission ) ];
    emission.sampler = nativeSampler;
    emission.imageView = _gBuffer.GetEmission ().GetImageView ();
    emission.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDescriptorImageInfo& normal = imageInfo[ static_cast<size_t> ( ePresentTarget::Normal ) ];
    normal.sampler = nativeSampler;
    normal.imageView = _gBuffer.GetNormal ().GetImageView ();
    normal.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDescriptorImageInfo& param = imageInfo[ static_cast<size_t> ( ePresentTarget::Param ) ];
    param.sampler = nativeSampler;
    param.imageView = _gBuffer.GetParams ().GetImageView ();
    param.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet const writeSets[] =
    {
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _gBufferSlotMapper[ static_cast<size_t> ( ePresentTarget::Albedo ) ],
            .dstBinding = 0U,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .pImageInfo = &albedo,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _gBufferSlotMapper[ static_cast<size_t> ( ePresentTarget::Albedo ) ],
            .dstBinding = 1U,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .pImageInfo = &albedo,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _gBufferSlotMapper[ static_cast<size_t> ( ePresentTarget::Emission ) ],
            .dstBinding = 0U,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .pImageInfo = &emission,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _gBufferSlotMapper[ static_cast<size_t> ( ePresentTarget::Emission ) ],
            .dstBinding = 1U,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .pImageInfo = &emission,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _gBufferSlotMapper[ static_cast<size_t> ( ePresentTarget::Normal ) ],
            .dstBinding = 0U,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .pImageInfo = &normal,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _gBufferSlotMapper[ static_cast<size_t> ( ePresentTarget::Normal ) ],
            .dstBinding = 1U,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .pImageInfo = &normal,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _gBufferSlotMapper[ static_cast<size_t> ( ePresentTarget::Param ) ],
            .dstBinding = 0U,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .pImageInfo = &param,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = _gBufferSlotMapper[ static_cast<size_t> ( ePresentTarget::Param ) ],
            .dstBinding = 1U,
            .dstArrayElement = 0U,
            .descriptorCount = 1U,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .pImageInfo = &param,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        }
    };

    vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( std::size ( writeSets ) ), writeSets, 0U, nullptr );
    return true;
}

bool RenderSession::CreatePresentFramebuffers ( android_vulkan::Renderer &renderer )
{
    size_t const framebufferCount = renderer.GetPresentImageCount ();
    _presentFramebuffers.reserve ( framebufferCount );
    VkFramebuffer framebuffer = VK_NULL_HANDLE;

    VkExtent2D const& resolution = renderer.GetSurfaceSize ();

    VkFramebufferCreateInfo framebufferInfo;
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.pNext = nullptr;
    framebufferInfo.flags = 0U;
    framebufferInfo.renderPass = _presentRenderPass;
    framebufferInfo.attachmentCount = 1U;
    framebufferInfo.width = resolution.width;
    framebufferInfo.height = resolution.height;
    framebufferInfo.layers = 1U;

    VkDevice device = renderer.GetDevice ();

    for ( size_t i = 0U; i < framebufferCount; ++i )
    {
        framebufferInfo.pAttachments = &renderer.GetPresentImageView ( i );

        bool const result = android_vulkan::Renderer::CheckVkResult (
            vkCreateFramebuffer ( device, &framebufferInfo, nullptr, &framebuffer ),
            "RenderSession::CreatePresentFramebuffers",
            "Can't create a framebuffer"
        );

        if ( !result )
            return false;

        _presentFramebuffers.push_back ( framebuffer );
        AV_REGISTER_FRAMEBUFFER ( "RenderSession::_presentFramebuffers" )
    }

    return true;
}

void RenderSession::DestroyPresentFramebuffers ( android_vulkan::Renderer &renderer )
{
    VkDevice device = renderer.GetDevice ();

    for ( auto framebuffer : _presentFramebuffers )
    {
        vkDestroyFramebuffer ( device, framebuffer, nullptr );
        AV_UNREGISTER_FRAMEBUFFER ( "RenderSession::_presentFramebuffers" )
    }

    _presentFramebuffers.clear ();
}

bool RenderSession::CreatePresentRenderPass ( android_vulkan::Renderer &renderer )
{
    VkAttachmentDescription const attachment[] =
    {
        {
            .flags = 0U,
            .format = renderer.GetSurfaceFormat (),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        }
    };

    constexpr static VkAttachmentReference const references[] =
    {
        {
            .attachment = 0U,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        }
    };

    constexpr static VkSubpassDescription const subpasses[] =
    {
        {
            .flags = 0U,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = 0U,
            .pInputAttachments = nullptr,
            .colorAttachmentCount = static_cast<uint32_t> ( std::size ( references ) ),
            .pColorAttachments = references,
            .pResolveAttachments = nullptr,
            .pDepthStencilAttachment = nullptr,
            .preserveAttachmentCount = 0U,
            .pPreserveAttachments = nullptr,
        }
    };

    VkRenderPassCreateInfo const info
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .attachmentCount = static_cast<uint32_t> ( std::size ( attachment ) ),
        .pAttachments = attachment,
        .subpassCount = static_cast<uint32_t> ( std::size ( subpasses ) ),
        .pSubpasses = subpasses,
        .dependencyCount = 0U,
        .pDependencies = nullptr
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateRenderPass ( renderer.GetDevice (), &info, nullptr, &_presentRenderPass ),
        "RenderSession::CreatePresentRenderPass",
        "Can't create render pass"
    );

    if ( !result )
        return false;

    AV_REGISTER_RENDER_PASS ( "RenderSession::_presentRenderPass" )
    return true;
}

bool RenderSession::CreateSyncPrimitives ( android_vulkan::Renderer &renderer )
{
    constexpr VkSemaphoreCreateInfo const semaphoreInfo
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U
    };

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &_presentRenderPassEndSemaphore ),
        "RenderSession::CreateSyncPrimitives",
        "Can't create render pass end semaphore"
    );

    if ( !result )
        return false;

    AV_REGISTER_SEMAPHORE ( "RenderSession::_presentRenderPassEndSemaphore" )

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateSemaphore ( device, &semaphoreInfo, nullptr, &_presentRenderTargetAcquiredSemaphore ),
        "RenderSession::CreateSyncPrimitives",
        "Can't create render target acquired semaphore"
    );

    if ( !result )
        return false;

    AV_REGISTER_SEMAPHORE ( "RenderSession::_presentRenderTargetAcquiredSemaphore" )
    return true;
}

void RenderSession::DestroySyncPrimitives ( android_vulkan::Renderer &renderer )
{
    VkDevice device = renderer.GetDevice ();

    if ( _presentRenderTargetAcquiredSemaphore != VK_NULL_HANDLE )
    {
        vkDestroySemaphore ( device, _presentRenderTargetAcquiredSemaphore, nullptr );
        _presentRenderTargetAcquiredSemaphore = VK_NULL_HANDLE;
        AV_UNREGISTER_SEMAPHORE ( "RenderSession::_presentRenderTargetAcquiredSemaphore" )
    }

    if ( _presentRenderPassEndSemaphore == VK_NULL_HANDLE )
        return;

    vkDestroySemaphore ( device, _presentRenderPassEndSemaphore, nullptr );
    _presentRenderPassEndSemaphore = VK_NULL_HANDLE;
    AV_UNREGISTER_SEMAPHORE ( "RenderSession::_presentRenderPassEndSemaphore" )
}

void RenderSession::InitCommonStructures ()
{
    _gBufferImageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    _gBufferImageBarrier.pNext = nullptr;
    _gBufferImageBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    _gBufferImageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    _gBufferImageBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    _gBufferImageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    _gBufferImageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    _gBufferImageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    _gBufferImageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    _gBufferImageBarrier.subresourceRange.baseMipLevel = 0U;
    _gBufferImageBarrier.subresourceRange.baseArrayLayer = 0U;
    _gBufferImageBarrier.subresourceRange.layerCount = 1U;

    _presentClearValue.color.float32[ 0U ] = 0.0F;
    _presentClearValue.color.float32[ 1U ] = 0.0F;
    _presentClearValue.color.float32[ 2U ] = 0.0F;
    _presentClearValue.color.float32[ 3U ] = 0.0F;

    _presentBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    _presentBeginInfo.pNext = nullptr;
    _presentBeginInfo.renderPass = _presentRenderPass;
    _presentBeginInfo.clearValueCount = 1U;
    _presentBeginInfo.pClearValues = &_presentClearValue;
    _presentBeginInfo.renderArea.offset.x = 0;
    _presentBeginInfo.renderArea.offset.y = 0;

    constexpr static VkPipelineStageFlags const waitStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    _submitInfoMain.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    _submitInfoMain.pNext = nullptr;
    _submitInfoMain.waitSemaphoreCount = 1U;
    _submitInfoMain.pWaitSemaphores = &_presentRenderTargetAcquiredSemaphore;
    _submitInfoMain.pWaitDstStageMask = &waitStage;
    _submitInfoMain.commandBufferCount = 1U;
    _submitInfoMain.signalSemaphoreCount = 1U;
    _submitInfoMain.pSignalSemaphores = &_presentRenderPassEndSemaphore;

    _presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    _presentInfo.pNext = nullptr;
    _presentInfo.waitSemaphoreCount = 1U;
    _presentInfo.pWaitSemaphores = &_presentRenderPassEndSemaphore;
    _presentInfo.swapchainCount = 1U;
}

void RenderSession::SubmitOpaqueCall ( MeshRef &mesh,
    MaterialRef const &material,
    GXMat4 const &local,
    GXAABB const &worldBounds,
    android_vulkan::Half4 const &color0,
    android_vulkan::Half4 const &color1,
    android_vulkan::Half4 const &color2,
    android_vulkan::Half4 const &color3
)
{
    _renderSessionStats.SubmitOpaque ( mesh->GetVertexCount () );
    _geometryPass.Submit ( mesh, material, local, worldBounds, color0, color1, color2, color3 );
}

void RenderSession::SubmitPointLight ( LightRef const &light )
{
    _renderSessionStats.SubmitPointLight ();

    // Note it's safe cast like that here. "NOLINT" is a clang-tidy control comment.
    auto const& pointLight = static_cast<PointLight const&> ( *light.get () ); // NOLINT

    if ( _frustum.IsVisible ( pointLight.GetBounds () ) )
    {
        _pointLightPass.Submit ( light );
    }
}

} // namespace pbr
