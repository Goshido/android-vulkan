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
    _fence ( VK_NULL_HANDLE ),
    _interacts {},
    _lightDescriptorPool ( VK_NULL_HANDLE ),
    _lightDescriptorSets {},
    _lightSubmitInfoTransfer {},
    _lightTransferCommandBuffer ( VK_NULL_HANDLE ),
    _lightUniformInfo {},
    _lightUniformPool ( eUniformPoolSize::Tiny_4M ),
    _lightWriteSets {},
    _lightPassNotifier ( nullptr ),
    _lightup {},
    _shadowmapDescriptorPool ( VK_NULL_HANDLE ),
    _shadowmapDescriptorSets {},
    _shadowmapProgram {},
    _shadowmapRenderCommandBuffer ( VK_NULL_HANDLE ),
    _shadowmapRenderPass ( VK_NULL_HANDLE ),
    _shadowmapRenderPassInfo {},
    _shadowmapSubmitInfoRender {},
    _shadowmapSubmitInfoTransfer {},
    _shadowmapTransferCommandBuffer ( VK_NULL_HANDLE ),
    _shadowmapUniformInfo {},
    _shadowmapUniformPool ( eUniformPoolSize::Huge_64M ),
    _shadowmapWriteSets {},
    _shadowmaps {},
    _usedShadowmaps ( 0U )
{
    // NOTHING
}

bool PointLightPass::ExecuteLightupPhase ( android_vulkan::Renderer &renderer,
    LightVolume &lightVolume,
    android_vulkan::MeshGeometry &unitCube,
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

    if ( !UpdateLightGPUData ( renderer, viewProjection ) )
        return false;

    constexpr VkDeviceSize const offset = 0U;

    uint32_t const vertexCount = unitCube.GetVertexCount ();
    vkCmdBindVertexBuffers ( commandBuffer, 0U, 1U, &unitCube.GetVertexBuffer (), &offset );
    vkCmdBindIndexBuffer ( commandBuffer, unitCube.GetIndexBuffer (), 0U, VK_INDEX_TYPE_UINT32 );

    size_t const limit = _interacts.size ();

    for ( size_t i = 0U; i < limit; ++i )
    {
        _lightPassNotifier->OnBeginLightWithVolume ( commandBuffer );

        lightVolume.Execute ( vertexCount, _lightDescriptorSets[ i ], commandBuffer );
        vkCmdNextSubpass ( commandBuffer, VK_SUBPASS_CONTENTS_INLINE );

        _lightup.Lightup ( commandBuffer, unitCube, i );
        _lightPassNotifier->OnEndLightWithVolume ( commandBuffer );
    }

    return true;
}

bool PointLightPass::ExecuteShadowPhase ( android_vulkan::Renderer &renderer,
    SceneData const &sceneData,
    size_t opaqueMeshCount
)
{
    if ( _interacts.empty () )
        return true;

    if ( !UpdateShadowmapGPUData ( renderer, sceneData, opaqueMeshCount ) )
        return false;

    return GenerateShadowmaps ( renderer );
}

bool PointLightPass::Init ( android_vulkan::Renderer &renderer,
    LightPassNotifier &notifier,
    VkCommandPool commandPool,
    VkExtent2D const &resolution,
    VkRenderPass lightupRenderPass
)
{
    _lightPassNotifier = &notifier;
    VkDevice device = renderer.GetDevice ();

    if ( !CreateShadowmapRenderPass ( device ) )
    {
        Destroy ( device );
        return false;
    }

    constexpr VkExtent2D const shadowmapResolution
    {
        .width = SHADOWMAP_RESOLUTION,
        .height = SHADOWMAP_RESOLUTION
    };

    if ( !_shadowmapProgram.Init ( renderer, _shadowmapRenderPass, 0U, shadowmapResolution ) )
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

    if ( !_shadowmapUniformPool.Init ( renderer, sizeof ( PointLightShadowmapGeneratorProgram::InstanceData ) ) )
    {
        Destroy ( device );
        return false;
    }

    if ( !_lightUniformPool.Init ( renderer, sizeof ( PointLightLightupProgram::VolumeData ) ) )
    {
        Destroy ( device );
        return false;
    }

    _commandPool = commandPool;
    VkCommandBuffer commandBuffers[ 3U ];

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

    _shadowmapRenderCommandBuffer = commandBuffers[ 0U ];
    _shadowmapTransferCommandBuffer = commandBuffers[ 1U ];
    _lightTransferCommandBuffer = commandBuffers[ 2U ];

    _lightSubmitInfoTransfer.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    _lightSubmitInfoTransfer.pNext = nullptr;
    _lightSubmitInfoTransfer.waitSemaphoreCount = 0U;
    _lightSubmitInfoTransfer.pWaitSemaphores = nullptr;
    _lightSubmitInfoTransfer.pWaitDstStageMask = nullptr;
    _lightSubmitInfoTransfer.commandBufferCount = 1U;
    _lightSubmitInfoTransfer.pCommandBuffers = &_lightTransferCommandBuffer;
    _lightSubmitInfoTransfer.signalSemaphoreCount = 0U;
    _lightSubmitInfoTransfer.pSignalSemaphores = nullptr;

    result = _lightup.Init ( renderer,
        commandPool,
        lightupRenderPass,
        LightVolume::GetLightupSubpass (),
        resolution
    );

    if ( !result )
    {
        Destroy ( device );
        return false;
    }

    constexpr static VkPipelineStageFlags const waitStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;

    _shadowmapSubmitInfoRender.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    _shadowmapSubmitInfoRender.pNext = nullptr;
    _shadowmapSubmitInfoRender.waitSemaphoreCount = 0U;
    _shadowmapSubmitInfoRender.pWaitSemaphores = nullptr;
    _shadowmapSubmitInfoRender.pWaitDstStageMask = &waitStage;
    _shadowmapSubmitInfoRender.commandBufferCount = 1U;
    _shadowmapSubmitInfoRender.pCommandBuffers = &_shadowmapRenderCommandBuffer;
    _shadowmapSubmitInfoRender.signalSemaphoreCount = 0U;
    _shadowmapSubmitInfoRender.pSignalSemaphores = nullptr;

    _shadowmapSubmitInfoTransfer.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    _shadowmapSubmitInfoTransfer.pNext = nullptr;
    _shadowmapSubmitInfoTransfer.waitSemaphoreCount = 0U;
    _shadowmapSubmitInfoTransfer.pWaitSemaphores = nullptr;
    _shadowmapSubmitInfoTransfer.pWaitDstStageMask = nullptr;
    _shadowmapSubmitInfoTransfer.commandBufferCount = 1U;
    _shadowmapSubmitInfoTransfer.pCommandBuffers = &_shadowmapTransferCommandBuffer;
    _shadowmapSubmitInfoTransfer.signalSemaphoreCount = 0U;
    _shadowmapSubmitInfoTransfer.pSignalSemaphores = nullptr;

    return true;
}

void PointLightPass::Destroy ( VkDevice device )
{
    _lightup.Destroy ( device );

    if ( !_shadowmaps.empty () )
    {
        for ( auto& [image, framebuffer] : _shadowmaps )
        {
            if ( framebuffer == VK_NULL_HANDLE )
                continue;

            image->FreeResources ( device );

            vkDestroyFramebuffer ( device, framebuffer, nullptr );
            AV_UNREGISTER_FRAMEBUFFER ( "PointLightPass::_shadowmaps" )
        }

        _shadowmaps.clear ();
    }

    DestroyLightDescriptorPool ( device );
    _lightDescriptorSets.clear ();
    _lightDescriptorSets.shrink_to_fit ();

    _lightUniformInfo.clear ();
    _lightUniformInfo.shrink_to_fit ();

    _lightWriteSets.clear ();
    _lightWriteSets.shrink_to_fit ();

    DestroyShadowmapDescriptorPool ( device );
    _shadowmapDescriptorSets.clear ();
    _shadowmapDescriptorSets.shrink_to_fit ();

    _shadowmapUniformInfo.clear ();
    _shadowmapUniformInfo.shrink_to_fit ();

    _shadowmapWriteSets.clear ();
    _shadowmapWriteSets.shrink_to_fit ();

    if ( _commandPool != VK_NULL_HANDLE )
    {
        VkCommandBuffer const commandBuffers[] =
        {
            _shadowmapRenderCommandBuffer,
            _shadowmapTransferCommandBuffer,
            _lightTransferCommandBuffer
        };

        vkFreeCommandBuffers ( device,
            _commandPool,
            static_cast<uint32_t> ( std::size ( commandBuffers ) ),
            commandBuffers
        );

        _shadowmapRenderCommandBuffer = VK_NULL_HANDLE;
        _shadowmapTransferCommandBuffer = VK_NULL_HANDLE;
        _lightTransferCommandBuffer = VK_NULL_HANDLE;
        _commandPool = VK_NULL_HANDLE;
    }

    _lightUniformPool.Destroy ( device );
    _shadowmapUniformPool.Destroy ( device );

    if ( _fence != VK_NULL_HANDLE )
    {
        vkDestroyFence ( device, _fence, nullptr );
        _fence = VK_NULL_HANDLE;
        AV_UNREGISTER_FENCE ( "PointLightPass::_fence" )
    }

    _shadowmapProgram.Destroy ( device );
    _lightPassNotifier = nullptr;

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

bool PointLightPass::AllocateLightDescriptorSets ( android_vulkan::Renderer &renderer, size_t neededSets )
{
    assert ( neededSets );

    if ( _lightDescriptorSets.size () >= neededSets )
        return true;

    VkDevice device = renderer.GetDevice ();
    DestroyLightDescriptorPool ( device );
    auto const size = static_cast<uint32_t> ( neededSets );

    VkDescriptorPoolSize const poolSizes[] =
    {
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = size
        }
    };

    VkDescriptorPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .maxSets = size,
        .poolSizeCount = static_cast<uint32_t> ( std::size ( poolSizes ) ),
        .pPoolSizes = poolSizes
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_lightDescriptorPool ),
        "PointLightPass::AllocateLightDescriptorSets",
        "Can't create descriptor pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_POOL ( "PointLightPass::_lightDescriptorPool" )

    constexpr VkDescriptorBufferInfo const uniform
    {
        .buffer = VK_NULL_HANDLE,
        .offset = 0U,
        .range = static_cast<VkDeviceSize> ( sizeof ( PointLightLightupProgram::VolumeData ) )
    };

    _lightUniformInfo.resize ( neededSets, uniform );

    constexpr VkWriteDescriptorSet const writeSet
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = VK_NULL_HANDLE,
        .dstBinding = 0U,
        .dstArrayElement = 0U,
        .descriptorCount = 1U,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pImageInfo = nullptr,
        .pBufferInfo = nullptr,
        .pTexelBufferView = nullptr
    };

    _lightWriteSets.resize ( neededSets, writeSet );
    _lightDescriptorSets.resize ( neededSets, VK_NULL_HANDLE );

    LightVolumeDescriptorSetLayout const layout;
    std::vector<VkDescriptorSetLayout> layouts ( neededSets, layout.GetLayout () );

    VkDescriptorSetAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _lightDescriptorPool,
        .descriptorSetCount = poolInfo.maxSets,
        .pSetLayouts = layouts.data ()
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &allocateInfo, _lightDescriptorSets.data () ),
        "PointLightPass::AllocateLightDescriptorSets",
        "Can't allocate descriptor sets"
    );

    if ( !result )
        return false;

    // Initialize all immutable constant fields.

    for ( size_t i = 0U; i < neededSets; ++i )
    {
        VkWriteDescriptorSet& write = _lightWriteSets[ i ];
        write.dstSet = _lightDescriptorSets[ i ];
        write.pBufferInfo = &_lightUniformInfo[ i ];
    }

    // Now all what is needed to do is to init "_lightUniformInfo::buffer". Then to invoke vkUpdateDescriptorSets.
    return true;
}

bool PointLightPass::AllocateShadowmapDescriptorSets ( android_vulkan::Renderer &renderer, size_t neededSets )
{
    assert ( neededSets );

    if ( _shadowmapDescriptorSets.size () >= neededSets )
        return true;

    VkDevice device = renderer.GetDevice ();
    DestroyShadowmapDescriptorPool ( device );
    auto const size = static_cast<uint32_t> ( neededSets );

    VkDescriptorPoolSize const poolSizes[] =
    {
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = size
        }
    };

    VkDescriptorPoolCreateInfo const poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .maxSets = size,
        .poolSizeCount = static_cast<uint32_t> ( std::size ( poolSizes ) ),
        .pPoolSizes = poolSizes
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateDescriptorPool ( device, &poolInfo, nullptr, &_shadowmapDescriptorPool ),
        "PointLightPass::AllocateShadowmapDescriptorSets",
        "Can't create descriptor pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_DESCRIPTOR_POOL ( "PointLightPass::_shadowmapDescriptorPool" )

    _shadowmapDescriptorSets.resize ( neededSets, VK_NULL_HANDLE );

    OpaqueInstanceDescriptorSetLayout const layout;
    std::vector<VkDescriptorSetLayout> const layouts ( neededSets, layout.GetLayout () );

    VkDescriptorSetAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = _shadowmapDescriptorPool,
        .descriptorSetCount = poolInfo.maxSets,
        .pSetLayouts = layouts.data ()
    };

    result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateDescriptorSets ( device, &allocateInfo, _shadowmapDescriptorSets.data () ),
        "PointLightPass::AllocateShadowmapDescriptorSets",
        "Can't allocate descriptor sets"
    );

    if ( !result )
        return false;

    // Initialize all immutable constant fields.

    constexpr VkDescriptorBufferInfo const uniform
    {
        .buffer = VK_NULL_HANDLE,
        .offset = 0U,
        .range = static_cast<VkDeviceSize> ( sizeof ( PointLightShadowmapGeneratorProgram::InstanceData ) )
    };

    _shadowmapUniformInfo.resize ( neededSets, uniform );

    constexpr VkWriteDescriptorSet const writeSet
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = VK_NULL_HANDLE,
        .dstBinding = 0U,
        .dstArrayElement = 0U,
        .descriptorCount = 1U,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pImageInfo = nullptr,
        .pBufferInfo = nullptr,
        .pTexelBufferView = nullptr
    };

    _shadowmapWriteSets.resize ( neededSets, writeSet );

    for ( size_t i = 0U; i < neededSets; ++i )
    {
        VkWriteDescriptorSet& uniformWriteSet = _shadowmapWriteSets[ i ];
        uniformWriteSet.dstSet = _shadowmapDescriptorSets[ i ];
        uniformWriteSet.pBufferInfo = &_shadowmapUniformInfo[ i ];
    }

    // Now all what is needed to do is to init "_shadowmapUniformInfo::buffer". Then to invoke vkUpdateDescriptorSets.
    return true;
}

PointLightPass::PointLightShadowmapInfo* PointLightPass::AcquirePointLightShadowmap (
    android_vulkan::Renderer &renderer
)
{
    if ( !_shadowmaps.empty () && _usedShadowmaps < _shadowmaps.size () )
        return &_shadowmaps[ _usedShadowmaps++ ];

    PointLightShadowmapInfo info;
    auto& [shadowmap, framebuffer] = info;
    shadowmap = std::make_shared<android_vulkan::TextureCube> ();

    constexpr VkExtent2D const resolution
    {
        .width = SHADOWMAP_RESOLUTION,
        .height = SHADOWMAP_RESOLUTION
    };

    constexpr VkImageUsageFlags const flags = AV_VK_FLAG ( VK_IMAGE_USAGE_SAMPLED_BIT ) |
        AV_VK_FLAG ( VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT );

    if ( !shadowmap->CreateRenderTarget ( renderer, resolution, VK_FORMAT_D32_SFLOAT, flags ) )
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

    constexpr static uint32_t const viewMasks[] =
    {
        0b00000000'00000000'00000000'00111111U
    };

    constexpr static uint32_t const correlationMasks[] =
    {
        0b00000000'00000000'00000000'00111111U
    };

    constexpr static VkRenderPassMultiviewCreateInfo const multiviewInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO,
        .pNext = nullptr,
        .subpassCount = static_cast<uint32_t> ( subpassCount ),
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
        .subpassCount = static_cast<uint32_t> ( subpassCount ),
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

    constexpr static VkClearValue const clearValues[] =
    {
        {
            .depthStencil
            {
                .depth = 1.0F,
                .stencil = 0U
            }
        }
    };

    constexpr static VkRect2D const renderArea
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

    _shadowmapRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    _shadowmapRenderPassInfo.pNext = nullptr;
    _shadowmapRenderPassInfo.renderPass = _shadowmapRenderPass;
    _shadowmapRenderPassInfo.renderArea = renderArea;
    _shadowmapRenderPassInfo.clearValueCount = std::size ( clearValues );
    _shadowmapRenderPassInfo.pClearValues = clearValues;

    return true;
}

void PointLightPass::DestroyLightDescriptorPool ( VkDevice device )
{
    if ( _lightDescriptorPool == VK_NULL_HANDLE )
        return;

    vkDestroyDescriptorPool ( device, _lightDescriptorPool, nullptr );
    _lightDescriptorPool = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_POOL ( "PointLightPass::_lightDescriptorPool" )
}

void PointLightPass::DestroyShadowmapDescriptorPool ( VkDevice device )
{
    if ( _shadowmapDescriptorPool == VK_NULL_HANDLE )
        return;

    vkDestroyDescriptorPool ( device, _shadowmapDescriptorPool, nullptr );
    _shadowmapDescriptorPool = VK_NULL_HANDLE;
    AV_UNREGISTER_DESCRIPTOR_POOL ( "PointLightPass::_shadowmapDescriptorPool" )
}

bool PointLightPass::GenerateShadowmaps ( android_vulkan::Renderer &renderer )
{
    constexpr VkCommandBufferBeginInfo const beginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    vkBeginCommandBuffer ( _shadowmapRenderCommandBuffer, &beginInfo );

    size_t setIndex = 0U;
    constexpr VkDeviceSize const offset = 0U;

    _shadowmapProgram.Bind ( _shadowmapRenderCommandBuffer );

    for ( auto const& [light, casters] : _interacts )
    {
        PointLightShadowmapInfo const* shadowmapInfo = AcquirePointLightShadowmap ( renderer );

        if ( !shadowmapInfo )
            return false;

        _shadowmapRenderPassInfo.framebuffer = shadowmapInfo->second;
        vkCmdBeginRenderPass ( _shadowmapRenderCommandBuffer, &_shadowmapRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

        for ( auto const& unique : casters._uniques )
        {
            vkCmdBindVertexBuffers ( _shadowmapRenderCommandBuffer, 0U, 1U, &unique->GetVertexBuffer (), &offset );

            vkCmdBindIndexBuffer ( _shadowmapRenderCommandBuffer,
                unique->GetIndexBuffer (),
                offset,
                VK_INDEX_TYPE_UINT32
            );

            _shadowmapProgram.SetDescriptorSet ( _shadowmapRenderCommandBuffer, _shadowmapDescriptorSets[ setIndex++ ] );
            vkCmdDrawIndexed ( _shadowmapRenderCommandBuffer, unique->GetVertexCount (), 1U, 0U, 0, 0U );
        }

        for ( auto const& [name, caster] : casters._batches )
        {
            auto const &[mesh, transforms] = caster;
            vkCmdBindVertexBuffers ( _shadowmapRenderCommandBuffer, 0U, 1U, &mesh->GetVertexBuffer (), &offset );

            vkCmdBindIndexBuffer ( _shadowmapRenderCommandBuffer,
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

                _shadowmapProgram.SetDescriptorSet ( _shadowmapRenderCommandBuffer,
                    _shadowmapDescriptorSets[ setIndex++ ] );

                vkCmdDrawIndexed ( _shadowmapRenderCommandBuffer,
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

        vkCmdEndRenderPass ( _shadowmapRenderCommandBuffer );
    }

    bool const result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( _shadowmapRenderCommandBuffer ),
        "PointLightPass::GenerateShadowmaps",
        "Can't end command buffer"
    );

    if ( !result )
        return false;

    return android_vulkan::Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &_shadowmapSubmitInfoRender, _fence ),
        "PointLightPass::GenerateShadowmaps",
        "Can't submit command buffer"
    );
}

bool PointLightPass::UpdateShadowmapGPUData ( android_vulkan::Renderer &renderer,
    SceneData const &sceneData,
    size_t opaqueMeshCount
)
{
    _shadowmapUniformPool.Reset ();
    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkWaitForFences ( device, 1U, &_fence, VK_TRUE, UINT64_MAX ),
        "PointLightPass::UpdateShadowmapGPUData",
        "Can't wait for fence"
    );

    if ( !result )
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( vkResetFences ( device, 1U, &_fence ),
        "PointLightPass::UpdateShadowmapGPUData",
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
        vkBeginCommandBuffer ( _shadowmapTransferCommandBuffer, &beginInfo ),
        "PointLightPass::UpdateShadowmapGPUData",
        "Can't begin command buffer"
    );

    if ( !result )
        return false;

    // Estimating from the top of the maximum required uniform buffers to be uploaded.
    size_t const maxUniformBuffers = _interacts.size () * opaqueMeshCount;

    if ( !AllocateShadowmapDescriptorSets ( renderer, maxUniformBuffers ) )
        return false;

    PointLightShadowmapGeneratorProgram::InstanceData instanceData {};

    auto append = [ & ] ( PointLight::Matrices const &matrices, size_t instance, GXMat4 const &local ) {
        PointLightShadowmapGeneratorProgram::ObjectData& objectData = instanceData._instanceData[ instance ];

        for ( size_t i = 0U; i < PBR_POINT_LIGHT_SHADOW_CASTER_PROJECTION_COUNT; ++i )
        {
            objectData._transform[ i ].Multiply ( local, matrices[ i ] );
        }
    };

    size_t bufferIndex = 0U;

    auto commit = [ & ] () noexcept {
        _shadowmapUniformInfo[ bufferIndex++ ].buffer = _shadowmapUniformPool.Acquire ( renderer,
            _shadowmapTransferCommandBuffer,
            &instanceData,
            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
        );

        return true;
    };

    for ( auto& [light, casters] : _interacts )
    {
        // Note it's safe cast like that here. "NOLINT" is a clang-tidy control comment.
        auto& pointLight = static_cast<PointLight &> ( *light.get () ); // NOLINT
        GXAABB const& lightBounds = pointLight.GetBounds ();
        PointLight::Matrices const& matrices = pointLight.GetMatrices ();

        for ( auto const& [material, opaque] : sceneData )
        {
            std::vector<MeshRef>* uniques = nullptr;

            for ( auto const& [mesh, opaqueData] : opaque.GetUniqueList () )
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

            for ( auto const& [name, meshGroup] : opaque.GetBatchList () )
            {
                for ( auto const& opaqueData : meshGroup._opaqueData )
                {
                    if ( !lightBounds.IsOverlaped ( opaqueData._worldBounds ) )
                        continue;

                    auto& [mesh, locals] = casters._batches[ name ];

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
        for ( auto const& [name, caster] : casters._batches )
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
        static_cast<uint32_t> ( bufferIndex ),
        _shadowmapWriteSets.data (),
        0U,
        nullptr
    );

    result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( _shadowmapTransferCommandBuffer ),
        "PointLightPass::UpdateShadowmapGPUData",
        "Can't end command buffer"
    );

    if ( !result )
        return false;

    return android_vulkan::Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &_shadowmapSubmitInfoTransfer, VK_NULL_HANDLE ),
        "PointLightPass::UpdateShadowmapGPUData",
        "Can't submit command buffer"
    );
}

bool PointLightPass::UpdateLightGPUData ( android_vulkan::Renderer &renderer, GXMat4 const &viewProjection )
{
    _lightUniformPool.Reset ();
    size_t const lightCount = _interacts.size ();

    if ( !AllocateLightDescriptorSets ( renderer, lightCount ) )
        return false;

    constexpr VkCommandBufferBeginInfo const beginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkBeginCommandBuffer ( _lightTransferCommandBuffer, &beginInfo ),
        "PointLightPass::UpdateLightGPUData",
        "Can't begin command buffer"
    );

    if ( !result )
        return false;

    PointLightLightupProgram::VolumeData volumeData {};
    GXMat4& transform = volumeData._transform;
    GXMat4 local {};
    GXVec3 alpha {};

    for ( size_t i = 0U; i < lightCount; ++i )
    {
        auto const& [light, casters] = _interacts[ i ];

        // Note it's safe cast like that here. "NOLINT" is a clang-tidy control comment.
        auto& pointLight = static_cast<PointLight &> ( *light.get () ); // NOLINT
        GXAABB const bounds = pointLight.GetBounds ();

        local.Scale ( bounds.GetWidth (), bounds.GetHeight (), bounds.GetDepth () );
        bounds.GetCenter ( alpha );
        local.SetW ( alpha );
        transform.Multiply ( local, viewProjection );

        _lightUniformInfo[ i ].buffer = _lightUniformPool.Acquire ( renderer,
            _lightTransferCommandBuffer,
            &volumeData,
            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
        );
    }

    result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( _lightTransferCommandBuffer ),
        "PointLightPass::UpdateLightGPUData",
        "Can't end command buffer"
    );

    if ( !result )
        return false;

    result = android_vulkan::Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &_lightSubmitInfoTransfer, VK_NULL_HANDLE ),
        "PointLightPass::UpdateLightGPUData",
        "Can't submit command buffer"
    );

    if ( !result )
        return false;

    vkUpdateDescriptorSets ( renderer.GetDevice (),
        static_cast<uint32_t> ( _lightWriteSets.size () ),
        _lightWriteSets.data (),
        0U,
        nullptr
    );

    return true;
}

} // namespace pbr
