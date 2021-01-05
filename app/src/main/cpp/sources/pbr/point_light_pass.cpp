#include <pbr/point_light_pass.h>
#include <pbr/point_light.h>


namespace pbr {

constexpr static uint32_t SHADOWMAP_RESOLUTION = 512U;

//----------------------------------------------------------------------------------------------------------------------

PointLightPass::PointLightPass () noexcept:
    _commandPool ( VK_NULL_HANDLE ),
    _descriptorPool ( VK_NULL_HANDLE ),
    _fence ( VK_NULL_HANDLE ),
    _interacts {},
    _program {},
    _renderCommandBuffer ( VK_NULL_HANDLE ),
    _renderPass ( VK_NULL_HANDLE ),
    _submitInfoRender {},
    _submitInfoTransfer {},
    _transferCommandBuffer ( VK_NULL_HANDLE ),
    _uniformPool {},
    _usedShadowmaps ( 0U )
{
    // NOTHING
}

bool PointLightPass::Execute ( SceneData const &sceneData,
    size_t opaqueMeshCount,
    android_vulkan::Renderer &renderer
)
{
    std::vector<VkDescriptorSet> descriptorSetStorage;

    if ( !UpdateGPUData ( descriptorSetStorage, sceneData, opaqueMeshCount, renderer ) )
        return false;

    return GenerateShadowmaps ( descriptorSetStorage.data (), renderer );
}

bool PointLightPass::Init ( android_vulkan::Renderer &renderer )
{
    if ( !CreateRenderPass ( renderer ) )
    {
        Destroy ( renderer );
        return false;
    }

    constexpr VkExtent2D const resolution
    {
        .width = SHADOWMAP_RESOLUTION,
        .height = SHADOWMAP_RESOLUTION
    };

    if ( !_program.Init ( renderer, _renderPass, 0U, resolution ) )
    {
        Destroy ( renderer );
        return false;
    }

    constexpr VkFenceCreateInfo const fenceInfo
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateFence ( device, &fenceInfo, nullptr, &_fence ),
        "PointLightPass::Init",
        "Can't create fence"
    );

    if ( !result )
    {
        Destroy ( renderer );
        return false;
    }

    AV_REGISTER_FENCE ( "PointLightPass::_fence" )

    if ( !_uniformPool.Init ( sizeof ( PointLightShadowmapGeneratorProgram::InstanceData ), renderer ) )
    {
        Destroy ( renderer );
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
        Destroy ( renderer );
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
        Destroy ( renderer );
        return false;
    }

    _renderCommandBuffer = commandBuffers[ 0U ];
    _transferCommandBuffer = commandBuffers[ 1U ];

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

void PointLightPass::Destroy ( android_vulkan::Renderer &renderer )
{
    VkDevice device = renderer.GetDevice ();

    if ( !_shadowmaps.empty () )
    {
        for ( auto &[image, framebuffer] : _shadowmaps )
        {
            if ( framebuffer == VK_NULL_HANDLE )
                continue;

            image->FreeResources ( renderer );

            vkDestroyFramebuffer ( device, framebuffer, nullptr );
            AV_UNREGISTER_FRAMEBUFFER ( "PointLightPass::_shadowmaps" )
        }

        _shadowmaps.clear ();
    }

    DestroyDescriptorPool ( renderer );

    if ( _commandPool != VK_NULL_HANDLE )
    {
        vkDestroyCommandPool ( device, _commandPool, nullptr );
        _commandPool = VK_NULL_HANDLE;
        AV_UNREGISTER_COMMAND_POOL ( "PointLightPass::_commandPool" )
    }

    _uniformPool.Destroy ( renderer );

    if ( _fence != VK_NULL_HANDLE )
    {
        vkDestroyFence ( device, _fence, nullptr );
        _fence = VK_NULL_HANDLE;
        AV_UNREGISTER_FENCE ( "PointLightPass::_fence" )
    }

    _program.Destroy ( renderer );

    if ( _renderPass == VK_NULL_HANDLE )
        return;

    vkDestroyRenderPass ( device, _renderPass, nullptr );
    _renderPass = VK_NULL_HANDLE;
    AV_UNREGISTER_RENDER_PASS ( "PointLightPass::_renderPass" )
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

    if ( !shadowmap->CreateRenderTarget ( resolution, renderer.GetDefaultDepthStencilFormat (), flags, renderer ) )
        return nullptr;

    VkImageView const attachments[] = { shadowmap->GetImageView () };

    VkFramebufferCreateInfo const framebufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .renderPass = _renderPass,
        .attachmentCount = static_cast<uint32_t> ( std::size ( attachments ) ),
        .pAttachments = attachments,
        .width = resolution.width,
        .height = resolution.height,
        .layers = PBR_POINT_LIGHT_SHADOW_CASTER_PROJECTION_COUNT
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateFramebuffer ( renderer.GetDevice (), &framebufferInfo, nullptr, &framebuffer ),
        "RenderSession::AcquirePointLightShadowmap",
        "Can't create framebuffer"
    );

    if ( !result )
    {
        shadowmap->FreeResources ( renderer );
        return nullptr;
    }

    AV_REGISTER_FRAMEBUFFER ( "PointLightPass::_shadowmaps" )

    ++_usedShadowmaps;
    return &_shadowmaps.emplace_back ( std::move ( info ) );
}

bool PointLightPass::CreateRenderPass ( android_vulkan::Renderer &renderer )
{
    VkAttachmentDescription const depthAttachment[] =
    {
        {
            .flags = 0U,
            .format = renderer.GetDefaultDepthStencilFormat (),
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
        vkCreateRenderPass ( renderer.GetDevice (), &renderPassInfo, nullptr, &_renderPass ),
        "PointLightPass::CreateRenderPass",
        "Can't create render pass"
    );

    if ( !result )
        return false;

    AV_REGISTER_RENDER_PASS ( "PointLightPass::_renderPass" )
    return true;
}

void PointLightPass::DestroyDescriptorPool ( android_vulkan::Renderer &renderer )
{
    if ( _descriptorPool == VK_NULL_HANDLE )
        return;

    vkDestroyDescriptorPool ( renderer.GetDevice (), _descriptorPool, nullptr );
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
    renderPassInfo.renderPass = _renderPass;
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

    DestroyDescriptorPool ( renderer );

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
        bufferInfo.buffer = _uniformPool.Acquire ( _transferCommandBuffer,
            &instanceData,
            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
            renderer
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
