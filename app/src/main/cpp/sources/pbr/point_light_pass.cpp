#include <pbr/point_light_pass.h>
#include <pbr/point_light.h>


GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace pbr {

constexpr static uint32_t SHADOWMAP_RESOLUTION = 512U;

//----------------------------------------------------------------------------------------------------------------------

PointLightPass::PointLightPass () noexcept:
    _commandPool ( VK_NULL_HANDLE ),
    _descriptorPool ( VK_NULL_HANDLE ),
    _fence ( VK_NULL_HANDLE ),
    _interacts {},
    _lightVolume {},
    _lightup {},
    _lightupCommonDescriptorSet {},
    _lightupRenderPassInfo {},
    _program {},
    _renderCommandBuffer ( VK_NULL_HANDLE ),
    _shadowmapRenderPass ( VK_NULL_HANDLE ),
    _submitInfoRender {},
    _submitInfoTransfer {},
    _transferCommandBuffer ( VK_NULL_HANDLE ),
    _uniformPool ( eUniformPoolSize::Huge_64M ),
    _usedShadowmaps ( 0U )
{
    // NOTHING
}

bool PointLightPass::ExecuteLightupPhase ( android_vulkan::Renderer &renderer,
    bool &isCommonSetBind,
    VkCommandBuffer commandBuffer,
    GXMat4 const &viewerLocal,
    GXMat4 const &view,
    GXMat4 const &viewProjection
)
{
    if ( _interacts.empty () )
        return true;

    if ( !_lightup.UpdateGPUData ( renderer, *this, viewerLocal, view ) )
        return false;

    GXMat4 local;
    local.Identity ();

    GXMat4 transform;
    GXVec3 alpha;
    size_t lightIndex = 0U;

    constexpr VkDeviceSize const offset = 0U;

    android_vulkan::MeshGeometry const& mesh = _lightup.GetLightVolume ();
    uint32_t const vertexCount = mesh.GetVertexCount ();
    vkCmdBindVertexBuffers ( commandBuffer, 0U, 1U, &mesh.GetVertexBuffer (), &offset );
    vkCmdBindIndexBuffer ( commandBuffer, mesh.GetIndexBuffer (), 0U, VK_INDEX_TYPE_UINT32 );

    for ( auto const &[light, casters] : _interacts )
    {
        // Note it's safe cast like that here. "NOLINT" is a clang-tidy control comment.
        auto& pointLight = *static_cast<PointLight*> ( light.get () ); // NOLINT

        GXAABB const& bounds = pointLight.GetBounds ();
        local._m[ 0U ][ 0U ] = bounds.GetWidth ();
        local._m[ 1U ][ 1U ] = bounds.GetHeight ();
        local._m[ 2U ][ 2U ] = bounds.GetDepth ();
        bounds.GetCenter ( alpha );
        local.SetW ( alpha );

        transform.Multiply ( local, viewProjection );
        vkCmdBeginRenderPass ( commandBuffer, &_lightupRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

        _lightVolume.Execute ( vertexCount, transform, commandBuffer );
        vkCmdNextSubpass ( commandBuffer, VK_SUBPASS_CONTENTS_INLINE );

        if ( !isCommonSetBind )
        {
            _lightup.SetCommonDescriptorSet ( commandBuffer, _lightupCommonDescriptorSet.GetSet () );
            isCommonSetBind = true;
        }

        _lightup.Lightup ( commandBuffer, lightIndex, transform );
        vkCmdEndRenderPass ( commandBuffer );

        ++lightIndex;
    }

    return true;
}

bool PointLightPass::ExecuteShadowPhase ( android_vulkan::Renderer &renderer,
    SceneData const &sceneData,
    size_t opaqueMeshCount
)
{
    std::vector<VkDescriptorSet> descriptorSetStorage;

    if ( !UpdateGPUData ( descriptorSetStorage, sceneData, opaqueMeshCount, renderer ) )
        return false;

    return GenerateShadowmaps ( descriptorSetStorage.data (), renderer );
}

bool PointLightPass::Init ( android_vulkan::Renderer &renderer, GBuffer &gBuffer )
{
    _lightupRenderPassInfo.framebuffer = VK_NULL_HANDLE;
    _lightupRenderPassInfo.renderPass = VK_NULL_HANDLE;

    VkDevice device = renderer.GetDevice ();

    if ( !CreateShadowmapRenderPass ( device ) )
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

    constexpr VkExtent2D const shadowmapResolution
    {
        .width = SHADOWMAP_RESOLUTION,
        .height = SHADOWMAP_RESOLUTION
    };

    if ( !_program.Init ( renderer, _shadowmapRenderPass, 0U, shadowmapResolution ) )
    {
        Destroy ( device );
        return false;
    }

    constexpr VkFenceCreateInfo const fenceInfo
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateFence ( device, &fenceInfo, nullptr, &_fence ),
        "PointLightPass::Init",
        "Can't create fence"
    );

    if ( !result )
    {
        Destroy ( device );
        return false;
    }

    AV_REGISTER_FENCE ( "PointLightPass::_fence" )

    if ( !_uniformPool.Init ( renderer, sizeof ( PointLightShadowmapGeneratorProgram::InstanceData ) ) )
    {
        Destroy ( device );
        return false;
    }

    VkCommandPoolCreateInfo const commandPoolInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( device, &commandPoolInfo, nullptr, &_commandPool ),
        "PointLightPass::Init",
        "Can't create command pool"
    );

    if ( !result )
    {
        Destroy ( device );
        return false;
    }

    AV_REGISTER_COMMAND_POOL ( "PointLightPass::_commandPool" )

    VkCommandBuffer commandBuffers[ 2U ];

    VkCommandBufferAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = _commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t> ( std::size ( commandBuffers ) )
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &allocateInfo, commandBuffers ),
        "PointLightPass::Init",
        "Can't allocate command buffers"
    );

    if ( !result )
    {
        Destroy ( device );
        return false;
    }

    _renderCommandBuffer = commandBuffers[ 0U ];
    _transferCommandBuffer = commandBuffers[ 1U ];

    result = _lightup.Init ( renderer,
        _transferCommandBuffer,
        _lightupRenderPassInfo.renderPass,
        LightVolume::GetLightupSubpass (),
        gBuffer.GetResolution ()
    );

    if ( !result )
    {
        Destroy ( device );
        return false;
    }

    constexpr static VkPipelineStageFlags const waitStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;

    _submitInfoRender.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    _submitInfoRender.pNext = nullptr;
    _submitInfoRender.waitSemaphoreCount = 0U;
    _submitInfoRender.pWaitSemaphores = nullptr;
    _submitInfoRender.pWaitDstStageMask = &waitStage;
    _submitInfoRender.commandBufferCount = 1U;
    _submitInfoRender.pCommandBuffers = &_renderCommandBuffer;
    _submitInfoRender.signalSemaphoreCount = 0U;
    _submitInfoRender.pSignalSemaphores = nullptr;

    _submitInfoTransfer.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    _submitInfoTransfer.pNext = nullptr;
    _submitInfoTransfer.waitSemaphoreCount = 0U;
    _submitInfoTransfer.pWaitSemaphores = nullptr;
    _submitInfoTransfer.pWaitDstStageMask = nullptr;
    _submitInfoTransfer.commandBufferCount = 1U;
    _submitInfoTransfer.pCommandBuffers = &_transferCommandBuffer;
    _submitInfoTransfer.signalSemaphoreCount = 0U;
    _submitInfoTransfer.pSignalSemaphores = nullptr;

    return true;
}

void PointLightPass::Destroy ( VkDevice device )
{
    _lightup.Destroy ( device );

    if ( !_shadowmaps.empty () )
    {
        for ( auto &[image, framebuffer] : _shadowmaps )
        {
            if ( framebuffer == VK_NULL_HANDLE )
                continue;

            image->FreeResources ( device );

            vkDestroyFramebuffer ( device, framebuffer, nullptr );
            AV_UNREGISTER_FRAMEBUFFER ( "PointLightPass::_shadowmaps" )
        }

        _shadowmaps.clear ();
    }

    DestroyDescriptorPool ( device );

    if ( _commandPool != VK_NULL_HANDLE )
    {
        vkDestroyCommandPool ( device, _commandPool, nullptr );
        _commandPool = VK_NULL_HANDLE;
        AV_UNREGISTER_COMMAND_POOL ( "PointLightPass::_commandPool" )
    }

    _uniformPool.Destroy ( device );

    if ( _fence != VK_NULL_HANDLE )
    {
        vkDestroyFence ( device, _fence, nullptr );
        _fence = VK_NULL_HANDLE;
        AV_UNREGISTER_FENCE ( "PointLightPass::_fence" )
    }

    _program.Destroy ( device );
    _lightupCommonDescriptorSet.Destroy ( device );
    _lightVolume.Destroy ( device );

    if ( _lightupRenderPassInfo.framebuffer != VK_NULL_HANDLE )
    {
        vkDestroyFramebuffer ( device, _lightupRenderPassInfo.framebuffer, nullptr );
        _lightupRenderPassInfo.framebuffer = VK_NULL_HANDLE;
        AV_UNREGISTER_FRAMEBUFFER ( "PointLightPass::_lightupFramebuffer" )
    }

    if ( _lightupRenderPassInfo.renderPass != VK_NULL_HANDLE )
    {
        vkDestroyRenderPass ( device, _lightupRenderPassInfo.renderPass, nullptr );
        _lightupRenderPassInfo.renderPass = VK_NULL_HANDLE;
        AV_UNREGISTER_RENDER_PASS ( "PointLightPass::_lightupRenderPass" )
    }

    if ( _shadowmapRenderPass == VK_NULL_HANDLE )
        return;

    vkDestroyRenderPass ( device, _shadowmapRenderPass, nullptr );
    _shadowmapRenderPass = VK_NULL_HANDLE;
    AV_UNREGISTER_RENDER_PASS ( "PointLightPass::_shadowmapRenderPass" )
}

size_t PointLightPass::GetPointLightCount () const
{
    return _interacts.size ();
}

PointLightPass::PointLightInfo PointLightPass::GetPointLightInfo ( size_t lightIndex ) const
{
    assert ( lightIndex < _interacts.size () );

    // Note it's safe cast like that here. "NOLINT" is a clang-tidy control comment.
    auto* pointLight = static_cast<PointLight*> ( _interacts[ lightIndex ].first.get () ); // NOLINT

    return std::make_pair ( pointLight, _shadowmaps[ lightIndex ].first.get () );
}

void PointLightPass::Reset ()
{
    _interacts.clear ();
    _usedShadowmaps = 0U;
}

void PointLightPass::Submit ( LightRef const &light )
{
    _interacts.emplace_back ( std::make_pair ( light, ShadowCasters () ) );
}

bool PointLightPass::Update ( android_vulkan::Renderer &renderer,
    VkExtent2D const &resolution,
    GXMat4 const &cvvToView
)
{
    return _lightupCommonDescriptorSet.Update ( renderer, resolution, cvvToView );
}

PointLightPass::PointLightShadowmapInfo* PointLightPass::AcquirePointLightShadowmap (
    android_vulkan::Renderer &renderer
)
{
    if ( !_shadowmaps.empty () && _usedShadowmaps <= _shadowmaps.size () )
        return &_shadowmaps[ _usedShadowmaps++ ];

    PointLightShadowmapInfo info;
    auto &[shadowmap, framebuffer] = info;
    shadowmap = std::make_shared<android_vulkan::TextureCube> ();

    constexpr VkExtent2D const resolution
    {
        .width = SHADOWMAP_RESOLUTION,
        .height = SHADOWMAP_RESOLUTION
    };

    constexpr VkImageUsageFlags const flags = AV_VK_FLAG ( VK_IMAGE_USAGE_SAMPLED_BIT ) |
        AV_VK_FLAG ( VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT );

    if ( !shadowmap->CreateRenderTarget ( resolution, VK_FORMAT_D32_SFLOAT, flags, renderer ) )
        return nullptr;

    VkImageView const attachments[] = { shadowmap->GetImageView () };

    VkFramebufferCreateInfo const framebufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .renderPass = _shadowmapRenderPass,
        .attachmentCount = static_cast<uint32_t> ( std::size ( attachments ) ),
        .pAttachments = attachments,
        .width = resolution.width,
        .height = resolution.height,
        .layers = PBR_POINT_LIGHT_SHADOW_CASTER_PROJECTION_COUNT
    };

    VkDevice device = renderer.GetDevice ();

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateFramebuffer ( device, &framebufferInfo, nullptr, &framebuffer ),
        "RenderSession::AcquirePointLightShadowmap",
        "Can't create framebuffer"
    );

    if ( !result )
    {
        shadowmap->FreeResources ( device );
        return nullptr;
    }

    AV_REGISTER_FRAMEBUFFER ( "PointLightPass::_shadowmaps" )

    ++_usedShadowmaps;
    return &_shadowmaps.emplace_back ( std::move ( info ) );
}

bool PointLightPass::CreateLightupFramebuffer ( VkDevice device, GBuffer &gBuffer )
{
    VkImageView const attachments[] =
    {
        gBuffer.GetAlbedo ().GetImageView (),
        gBuffer.GetHDRAccumulator ().GetImageView (),
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

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateFramebuffer ( device, &info, nullptr, &_lightupRenderPassInfo.framebuffer ),
        "PointLightPass::CreateLightupFramebuffer",
        "Can't create framebuffer"
    );

    if ( !result )
        return false;

    AV_REGISTER_FRAMEBUFFER ( "PointLightPass::_lightupFramebuffer" )
    return true;
}

bool PointLightPass::CreateLightupRenderPass ( VkDevice device, GBuffer &gBuffer )
{
    VkAttachmentDescription const attachments[]
    {
        // #0: albedo
        {
            .flags = 0U,
            .format = gBuffer.GetAlbedo ().GetFormat (),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        },

        // #1: HDR accumulator
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

        // #2: normal
        {
            .flags = 0U,
            .format = gBuffer.GetNormal ().GetFormat (),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
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
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
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
        // #1: HDR accumulator
        {
            .attachment = 1U,
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
            .attachment = 0U,
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
            .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
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
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
        },
        {
            .srcSubpass = 0U,
            .dstSubpass = 1U,
            .srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
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

bool PointLightPass::CreateShadowmapRenderPass ( VkDevice device )
{
    VkAttachmentDescription const depthAttachment[] =
    {
        {
            .flags = 0U,
            .format = VK_FORMAT_D32_SFLOAT,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        }
    };

    constexpr static VkAttachmentReference const depthAttachmentReference
    {
        .attachment = 0U,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    constexpr static VkSubpassDescription const subpasses[]
    {
        {
            .flags = 0U,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = 0U,
            .pInputAttachments = nullptr,
            .colorAttachmentCount = 0U,
            .pColorAttachments = nullptr,
            .pResolveAttachments = nullptr,
            .pDepthStencilAttachment = &depthAttachmentReference,
            .preserveAttachmentCount = 0U,
            .pPreserveAttachments = nullptr
        }
    };

    constexpr size_t const subpassCount = std::size ( subpasses );

    constexpr static uint32_t const viewMasks[ subpassCount ] =
    {
        0b00000000'00000000'00000000'00111111U
    };

    constexpr static uint32_t const correlationMasks[ subpassCount ] =
    {
        0b00000000'00000000'00000000'00111111U
    };

    constexpr static VkRenderPassMultiviewCreateInfo const multiviewInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO,
        .pNext = nullptr,
        .subpassCount = static_cast<size_t> ( subpassCount ),
        .pViewMasks = viewMasks,
        .dependencyCount = 0U,
        .pViewOffsets = nullptr,
        .correlationMaskCount = static_cast<size_t> ( std::size ( correlationMasks ) ),
        .pCorrelationMasks = correlationMasks
    };

    VkRenderPassCreateInfo const renderPassInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = &multiviewInfo,
        .flags = 0U,
        .attachmentCount = static_cast<uint32_t> ( std::size ( depthAttachment ) ),
        .pAttachments = depthAttachment,
        .subpassCount = subpassCount,
        .pSubpasses = subpasses,
        .dependencyCount = 0U,
        .pDependencies = nullptr
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateRenderPass ( device, &renderPassInfo, nullptr, &_shadowmapRenderPass ),
        "PointLightPass::CreateShadowmapRenderPass",
        "Can't create render pass"
    );

    if ( !result )
        return false;

    AV_REGISTER_RENDER_PASS ( "PointLightPass::_shadowmapRenderPass" )
    return true;
}

void PointLightPass::DestroyDescriptorPool ( VkDevice device )
{
    if ( _descriptorPool == VK_NULL_HANDLE )
        return;

    vkDestroyDescriptorPool ( device, _descriptorPool, nullptr );
    _descriptorPool = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_POOL ( "PointLightPass::_descriptorPool" )
}

bool PointLightPass::GenerateShadowmaps ( VkDescriptorSet const* descriptorSets, android_vulkan::Renderer &renderer )
{
    constexpr VkCommandBufferBeginInfo const beginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    vkBeginCommandBuffer ( _renderCommandBuffer, &beginInfo );

    constexpr VkClearValue const clearValues[] =
    {
        {
            .depthStencil
            {
                .depth = 1.0F,
                .stencil = 0U
            }
        }
    };

    constexpr VkRect2D const renderArea
    {
        .offset
        {
            .x = 0,
            .y = 0
        },

        .extent
        {
            .width = SHADOWMAP_RESOLUTION,
            .height = SHADOWMAP_RESOLUTION
        }
    };

    VkRenderPassBeginInfo renderPassInfo;
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.renderPass = _shadowmapRenderPass;
    renderPassInfo.renderArea = renderArea;
    renderPassInfo.clearValueCount = std::size ( clearValues );
    renderPassInfo.pClearValues = clearValues;

    size_t setIndex = 0U;
    constexpr VkDeviceSize const offset = 0U;

    _program.Bind ( _renderCommandBuffer );

    for ( auto const &[light, casters] : _interacts )
    {
        PointLightShadowmapInfo const* shadowmapInfo = AcquirePointLightShadowmap ( renderer );

        if ( !shadowmapInfo )
            return false;

        renderPassInfo.framebuffer = shadowmapInfo->second;
        vkCmdBeginRenderPass ( _renderCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

        for ( auto const &unique : casters._uniques )
        {
            vkCmdBindVertexBuffers ( _renderCommandBuffer, 0U, 1U, &unique->GetVertexBuffer (), &offset );

            vkCmdBindIndexBuffer ( _renderCommandBuffer,
                unique->GetIndexBuffer (),
                offset,
                VK_INDEX_TYPE_UINT32
            );

            _program.SetDescriptorSet ( _renderCommandBuffer, descriptorSets[ setIndex ] );
            ++setIndex;

            vkCmdDrawIndexed ( _renderCommandBuffer, unique->GetVertexCount (), 1U, 0U, 0, 0U );
        }

        for ( auto const &[name, caster] : casters._batches )
        {
            auto const &[mesh, transforms] = caster;
            vkCmdBindVertexBuffers ( _renderCommandBuffer, 0U, 1U, &mesh->GetVertexBuffer (), &offset );

            vkCmdBindIndexBuffer ( _renderCommandBuffer,
                mesh->GetIndexBuffer (),
                offset,
                VK_INDEX_TYPE_UINT32
            );

            size_t remain = transforms.size ();
            uint32_t const vertexCount = mesh->GetVertexCount ();

            do
            {
                size_t const instances = std::min ( remain,
                    static_cast<size_t> ( PBR_POINT_LIGHT_MAX_SHADOW_CASTER_INSTANCE_COUNT )
                );

                _program.SetDescriptorSet ( _renderCommandBuffer, descriptorSets[ setIndex ] );
                ++setIndex;

                vkCmdDrawIndexed ( _renderCommandBuffer,
                    vertexCount,
                    static_cast<uint32_t> ( instances ),
                    0U,
                    0,
                    0U
                );

                remain -= instances;
            }
            while ( remain );
        }

        vkCmdEndRenderPass ( _renderCommandBuffer );
    }

    bool const result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( _renderCommandBuffer ),
        "PointLightPass::GenerateShadowmaps",
        "Can't end command buffer"
    );

    if ( !result )
        return false;

    return android_vulkan::Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &_submitInfoRender, _fence ),
        "PointLightPass::GenerateShadowmaps",
        "Can't submit command buffer"
    );
}

bool PointLightPass::UpdateGPUData ( std::vector<VkDescriptorSet> &descriptorSetStorage,
    SceneData const &sceneData,
    size_t opaqueMeshCount,
    android_vulkan::Renderer &renderer
)
{
    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkWaitForFences ( device, 1U, &_fence, VK_TRUE, UINT64_MAX ),
        "PointLightPass::UpdateGPUData",
        "Can't wait for fence"
    );

    if ( !result )
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( vkResetFences ( device, 1U, &_fence ),
        "PointLightPass::UpdateGPUData",
        "Can't reset fence"
    );

    if ( !result )
        return false;

    constexpr VkCommandBufferBeginInfo const beginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkBeginCommandBuffer ( _transferCommandBuffer, &beginInfo ),
        "PointLightPass::UpdateGPUData",
        "Can't begin command buffer"
    );

    if ( !result )
        return false;

    // Estimating from the top of the maximum required uniform buffers to be uploaded.
    size_t const maxUniformBuffers = _interacts.size () * opaqueMeshCount;

    VkDescriptorPoolSize const poolSize[] =
    {
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = static_cast<uint32_t> ( maxUniformBuffers )
        }
    };

    VkDescriptorPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .maxSets = static_cast<uint32_t> ( maxUniformBuffers ),
        .poolSizeCount = std::size ( poolSize ),
        .pPoolSizes = poolSize
    };

    DestroyDescriptorPool ( device );

    result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_descriptorPool ),
        "PointLightPass::UpdateGPUData",
        "Can't create descriptor pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_POOL ( "PointLightPass::_descriptorPool" )

    std::vector<VkDescriptorSetLayout> layouts;
    layouts.reserve ( maxUniformBuffers );

    OpaqueInstanceDescriptorSetLayout const instanceLayout;
    VkDescriptorSetLayout nativeLayout = instanceLayout.GetLayout ();

    for ( size_t i = 0U; i < maxUniformBuffers; ++i )
        layouts.push_back ( nativeLayout );

    VkDescriptorSetAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _descriptorPool,
        .descriptorSetCount = poolInfo.maxSets,
        .pSetLayouts = layouts.data ()
    };

    descriptorSetStorage.resize ( maxUniformBuffers );

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &allocateInfo, descriptorSetStorage.data () ),
        "PointLightPass::UpdateGPUData",
        "Can't allocate descriptor sets"
    );

    if ( !result )
        return false;

    VkDescriptorBufferInfo bufferInfo;
    bufferInfo.offset = 0U;
    bufferInfo.range = static_cast<VkDeviceSize> ( sizeof ( PointLightShadowmapGeneratorProgram::InstanceData ) );

    VkWriteDescriptorSet writeInfo;
    writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.pNext = nullptr;
    writeInfo.dstBinding = 0U;
    writeInfo.dstArrayElement = 0U;
    writeInfo.descriptorCount = 1U;
    writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeInfo.pImageInfo = nullptr;
    writeInfo.pTexelBufferView = nullptr;

    std::vector<VkWriteDescriptorSet> writeStorage;
    std::vector<VkDescriptorBufferInfo> uniformStorage;
    writeStorage.reserve ( maxUniformBuffers );
    uniformStorage.reserve ( maxUniformBuffers );

    PointLightShadowmapGeneratorProgram::InstanceData instanceData;
    _uniformPool.Reset ();

    auto append = [ & ] ( PointLight::Matrices const &matrices, size_t instance, GXMat4 const &local ) {
        PointLightShadowmapGeneratorProgram::ObjectData& objectData = instanceData._instanceData[ instance ];

        for ( size_t i = 0U; i < PBR_POINT_LIGHT_SHADOW_CASTER_PROJECTION_COUNT; ++i )
        {
            objectData._transform[ i ].Multiply ( local, matrices[ i ] );
        }
    };

    auto commit = [ & ] () {
        bufferInfo.buffer = _uniformPool.Acquire ( renderer,
            _transferCommandBuffer,
            &instanceData,
            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
        );

        uniformStorage.push_back ( bufferInfo );

        writeInfo.pBufferInfo = &uniformStorage.back ();
        writeInfo.dstSet = descriptorSetStorage[ writeStorage.size () ];
        writeStorage.push_back ( writeInfo );
    };

    for ( auto &[light, casters] : _interacts )
    {
        // Note it's safe cast like that here. "NOLINT" is a clang-tidy control comment.
        auto& pointLight = static_cast<PointLight &> ( *light.get () ); // NOLINT
        GXAABB const& lightBounds = pointLight.GetBounds ();
        PointLight::Matrices const& matrices = pointLight.GetMatrices ();

        for ( auto const &[material, opaque] : sceneData )
        {
            std::vector<MeshRef>* uniques = nullptr;

            for ( auto const &[mesh, opaqueData] : opaque.GetUniqueList () )
            {
                if ( !lightBounds.IsOverlaped ( opaqueData._worldBounds ) )
                    continue;

                if ( !uniques )
                {
                    uniques = &casters._uniques;

                    // Estimating from top. Worst case: all unique meshes are interacting with point light.
                    uniques->reserve ( opaqueMeshCount );
                }

                append ( matrices, 0U, opaqueData._local );
                commit ();
                uniques->push_back ( mesh );
            }

            // Note: the original mesh submit layout is stored by material groups. So there is a probability
            // that there are exact same meshes with different materials. Shadow casters do not care about
            // material only geometry matters. So to make shadowmap calls most efficiently it's need to collect
            // all instances first and only then fill uniform buffers.

            for ( auto const &[name, meshGroup] : opaque.GetBatchList () )
            {
                for ( auto const& opaqueData : meshGroup._opaqueData )
                {
                    if ( !lightBounds.IsOverlaped ( opaqueData._worldBounds ) )
                        continue;

                    auto &[mesh, locals] = casters._batches[ name ];

                    if ( !mesh )
                    {
                        mesh = meshGroup._mesh;

                        // Heap relocation optimization.
                        locals.reserve ( PBR_POINT_LIGHT_MAX_SHADOW_CASTER_INSTANCE_COUNT );
                    }

                    locals.push_back ( opaqueData._local );
                }
            }
        }

        // Commit uniform buffers for batch meshes.
        for ( auto const &[name, caster] : casters._batches )
        {
            std::vector<GXMat4> const& locals = caster.second;
            size_t remain = locals.size ();
            size_t instance = 0U;

            do
            {
                size_t const batches = std::min ( remain,
                    static_cast<size_t> ( PBR_POINT_LIGHT_MAX_SHADOW_CASTER_INSTANCE_COUNT )
                );

                for ( size_t i = 0U; i < batches; ++i )
                {
                    append ( matrices, i, locals[ instance ] );
                    ++instance;
                }

                commit ();
                remain -= batches;
            }
            while ( remain );
        }
    }

    vkUpdateDescriptorSets ( device,
        static_cast<uint32_t> ( writeStorage.size () ),
        writeStorage.data (),
        0U,
        nullptr
    );

    result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( _transferCommandBuffer ),
        "PointLightPass::UpdateGPUData",
        "Can't end command buffer"
    );

    if ( !result )
        return false;

    return android_vulkan::Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &_submitInfoTransfer, VK_NULL_HANDLE ),
        "PointLightPass::UpdateGPUData",
        "Can't submit command buffer"
    );
}

} // namespace pbr
