#include <pbr/point_light_pass.h>
#include <trace.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace pbr {

constexpr static uint32_t SHADOWMAP_RESOLUTION = 512U;

//----------------------------------------------------------------------------------------------------------------------

bool PointLightPass::ExecuteLightupPhase ( android_vulkan::MeshGeometry &unitCube,
    VkCommandBuffer commandBuffer
) noexcept
{
    if ( _interacts.empty () )
        return true;

    constexpr VkDeviceSize offset = 0U;

    _lightup.BindProgram ( commandBuffer );
    vkCmdBindVertexBuffers ( commandBuffer, 0U, 1U, &unitCube.GetVertexBuffer (), &offset );
    vkCmdBindIndexBuffer ( commandBuffer, unitCube.GetIndexBuffer (), 0U, VK_INDEX_TYPE_UINT32 );

    size_t const limit = _interacts.size ();

    for ( size_t i = 0U; i < limit; ++i )
        _lightup.Lightup ( commandBuffer, _lightBufferPool.Acquire (), unitCube );

    _lightBufferPool.Commit ();
    _lightup.Commit ();
    return true;
}

bool PointLightPass::ExecuteShadowPhase ( android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer,
    SceneData const &sceneData,
    size_t opaqueMeshCount
) noexcept
{
    if ( _interacts.empty () )
        return true;

    UpdateShadowmapGPUData ( renderer, commandBuffer, sceneData, opaqueMeshCount ) ;
    return GenerateShadowmaps ( renderer, commandBuffer );
}

bool PointLightPass::Init ( android_vulkan::Renderer &renderer,
    VkExtent2D const &resolution,
    VkRenderPass lightupRenderPass
) noexcept
{
    VkDevice device = renderer.GetDevice ();

    if ( !CreateShadowmapRenderPass ( device ) )
        return false;

    constexpr VkExtent2D shadowmapResolution
    {
        .width = SHADOWMAP_RESOLUTION,
        .height = SHADOWMAP_RESOLUTION
    };

    if ( !_shadowmapProgram.Init ( renderer, _shadowmapRenderPass, 0U, shadowmapResolution ) )
        return false;

    bool const result = _shadowmapBufferPool.Init ( renderer,
        GeometryPassInstanceDescriptorSetLayout {},
        sizeof ( PointLightShadowmapGeneratorProgram::InstanceData ),
        "pbr::PointLightPass::_shadowmapBufferPool"
    );

    if ( !result )
        return false;

    if ( !_lightup.Init ( renderer, lightupRenderPass, 1U, resolution ) )
        return false;

    return _lightBufferPool.Init ( renderer,
        LightVolumeDescriptorSetLayout {},
        sizeof ( PointLightLightupProgram::VolumeData ),
        "pbr::PointLightPass::_lightBufferPool"
    );
}

void PointLightPass::Destroy ( VkDevice device ) noexcept
{
    _lightup.Destroy ( device );

    if ( !_shadowmaps.empty () )
    {
        for ( auto& [image, framebuffer ] : _shadowmaps )
        {
            if ( framebuffer == VK_NULL_HANDLE )
                continue;

            image->FreeResources ( device );

            vkDestroyFramebuffer ( device, framebuffer, nullptr );
            AV_UNREGISTER_FRAMEBUFFER ( "PointLightPass::_shadowmaps" )
        }

        _shadowmaps.clear ();
    }

    _lightBufferPool.Destroy ( device, "pbr::PointLightPass::_lightBufferPool" );
    _shadowmapBufferPool.Destroy ( device, "pbr::PointLightPass::_shadowmapBufferPool" );
    _shadowmapProgram.Destroy ( device );

    if ( _shadowmapRenderPass == VK_NULL_HANDLE )
        return;

    vkDestroyRenderPass ( device, _shadowmapRenderPass, nullptr );
    _shadowmapRenderPass = VK_NULL_HANDLE;
    AV_UNREGISTER_RENDER_PASS ( "PointLightPass::_shadowmapRenderPass" )
}

size_t PointLightPass::GetPointLightCount () const noexcept
{
    return _interacts.size ();
}

PointLightPass::PointLightInfo PointLightPass::GetPointLightInfo ( size_t lightIndex ) const noexcept
{
    assert ( lightIndex < _interacts.size () );

    // NOLINTNEXTLINE - downcast.
    auto* pointLight = static_cast<PointLight*> ( _interacts[ lightIndex ].first.get () );

    return std::make_pair ( pointLight, _shadowmaps[ lightIndex ].first.get () );
}

void PointLightPass::Reset () noexcept
{
    AV_TRACE ( "Point light reset" )

    _interacts.clear ();
    _usedShadowmaps = 0U;
}

void PointLightPass::Submit ( LightRef const &light ) noexcept
{
    _interacts.emplace_back ( std::make_pair ( light, ShadowCasters () ) );
}

bool PointLightPass::UploadGPUData ( android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer,
    GXMat4 const &viewerLocal,
    GXMat4 const &view,
    GXMat4 const &viewProjection
) noexcept
{
    if ( _interacts.empty () )
        return true;

    if ( !_lightup.UpdateGPUData ( renderer, commandBuffer, *this, viewerLocal, view ) )
        return false;

    UpdateLightGPUData ( renderer, commandBuffer, viewProjection );
    return true;
}

PointLightPass::PointLightShadowmapInfo* PointLightPass::AcquirePointLightShadowmap (
    android_vulkan::Renderer &renderer
) noexcept
{
    if ( !_shadowmaps.empty () && _usedShadowmaps < _shadowmaps.size () )
        return &_shadowmaps[ _usedShadowmaps++ ];

    PointLightShadowmapInfo info;
    auto& [shadowmap, framebuffer] = info;
    shadowmap = std::make_shared<android_vulkan::TextureCube> ();

    constexpr VkExtent2D resolution
    {
        .width = SHADOWMAP_RESOLUTION,
        .height = SHADOWMAP_RESOLUTION
    };

    constexpr VkImageUsageFlags flags = AV_VK_FLAG ( VK_IMAGE_USAGE_SAMPLED_BIT ) |
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
        .layers = 1U
    };

    VkDevice device = renderer.GetDevice ();

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateFramebuffer ( device, &framebufferInfo, nullptr, &framebuffer ),
        "pbr::RenderSession::AcquirePointLightShadowmap",
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

bool PointLightPass::CreateShadowmapRenderPass ( VkDevice device ) noexcept
{
    constexpr static VkAttachmentDescription const depthAttachment[] =
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
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL
        }
    };

    constexpr static VkAttachmentReference depthAttachmentReference
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

    constexpr size_t subpassCount = std::size ( subpasses );

    constexpr static uint32_t const viewMasks[] =
    {
        0b00000000'00000000'00000000'00111111U
    };

    constexpr static uint32_t const correlationMasks[] =
    {
        0b00000000'00000000'00000000'00111111U
    };

    constexpr static VkRenderPassMultiviewCreateInfo multiviewInfo
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

    constexpr static VkSubpassDependency const dependencies[] =
    {
        {
            .srcSubpass = 0U,
            .dstSubpass = VK_SUBPASS_EXTERNAL,
            .srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
            .dependencyFlags = 0U
        }
    };

    constexpr VkRenderPassCreateInfo renderPassInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = &multiviewInfo,
        .flags = 0U,
        .attachmentCount = static_cast<uint32_t> ( std::size ( depthAttachment ) ),
        .pAttachments = depthAttachment,
        .subpassCount = static_cast<uint32_t> ( subpassCount ),
        .pSubpasses = subpasses,
        .dependencyCount = static_cast<uint32_t> ( std::size ( dependencies ) ),
        .pDependencies = dependencies
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateRenderPass ( device, &renderPassInfo, nullptr, &_shadowmapRenderPass ),
        "pbr::PointLightPass::CreateShadowmapRenderPass",
        "Can't create render pass"
    );

    if ( !result )
        return false;

    AV_REGISTER_RENDER_PASS ( "PointLightPass::_shadowmapRenderPass" )

    // Must be static as well because it lose the function scope [read as garbage in debug builds].
    constexpr static VkClearValue const clearValues[] =
    {
        {
            .depthStencil
            {
                .depth = 0.0F,
                .stencil = 0U
            }
        }
    };

    // Must be static as well because it lose the function scope [read as garbage in debug builds].
    constexpr static VkRect2D renderArea
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

bool PointLightPass::GenerateShadowmaps ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer) noexcept
{
    constexpr VkDeviceSize offset = 0U;

    for ( auto const& [light, casters] : _interacts )
    {
        PointLightShadowmapInfo const* shadowmapInfo = AcquirePointLightShadowmap ( renderer );

        if ( !shadowmapInfo )
            return false;

        _shadowmapRenderPassInfo.framebuffer = shadowmapInfo->second;
        vkCmdBeginRenderPass ( commandBuffer, &_shadowmapRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

        // Note it's required to set graphic pipeline object every time when new render pass or sub pass begin with
        // multiview style rendering. see VkRenderPassMultiviewCreateInfo remarks in Vulkan spec.
        _shadowmapProgram.Bind ( commandBuffer );

        for ( auto const& unique : casters._uniques )
        {
            vkCmdBindVertexBuffers ( commandBuffer, 0U, 1U, &unique->GetVertexBuffer (), &offset );

            vkCmdBindIndexBuffer ( commandBuffer,
                unique->GetIndexBuffer (),
                offset,
                VK_INDEX_TYPE_UINT32
            );

            _shadowmapProgram.SetDescriptorSet ( commandBuffer, _shadowmapBufferPool.Acquire () );
            vkCmdDrawIndexed ( commandBuffer, unique->GetVertexCount (), 1U, 0U, 0, 0U );
        }

        for ( auto const& casterInfo : casters._batches )
        {
            auto const& [mesh, transforms] = casterInfo.second;
            vkCmdBindVertexBuffers ( commandBuffer, 0U, 1U, &mesh->GetVertexBuffer (), &offset );

            vkCmdBindIndexBuffer ( commandBuffer,
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

                _shadowmapProgram.SetDescriptorSet ( commandBuffer, _shadowmapBufferPool.Acquire () );

                vkCmdDrawIndexed ( commandBuffer,
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

        vkCmdEndRenderPass ( commandBuffer );
    }

    _shadowmapBufferPool.Commit ();
    return true;
}

void PointLightPass::UpdateShadowmapGPUData ( android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer,
    SceneData const &sceneData,
    size_t opaqueMeshCount
) noexcept
{
    PointLightShadowmapGeneratorProgram::InstanceData instanceData {};

    auto append = [ & ] ( PointLight::Matrices const &matrices, size_t instance, GXMat4 const &local ) {
        PointLightShadowmapGeneratorProgram::ObjectData& objectData = instanceData._instanceData[ instance ];

        for ( size_t i = 0U; i < PBR_POINT_LIGHT_SHADOW_CASTER_PROJECTION_COUNT; ++i )
        {
            objectData._transform[ i ].Multiply ( local, matrices[ i ] );
        }
    };

    for ( auto& [light, casters] : _interacts )
    {
        // NOLINTNEXTLINE - downcast.
        auto& pointLight = static_cast<PointLight &> ( *light );
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
                _shadowmapBufferPool.Push ( renderer, commandBuffer, &instanceData );
                uniques->push_back ( mesh );
            }

            // Note: the original mesh submit layout is stored by material groups. So there is a probability
            // that there are exact same meshes with different materials. Shadow casters do not care about
            // material only geometry matters. So to make shadowmap calls most efficiently it's need to collect
            // all instances first and only then fill uniform buffers.

            for ( auto const& [name, meshGroup] : opaque.GetBatchList () )
            {
                for ( auto const& geometryData : meshGroup._geometryData )
                {
                    if ( !lightBounds.IsOverlaped ( geometryData._worldBounds ) )
                        continue;

                    auto& [mesh, locals] = casters._batches[ name ];

                    if ( !mesh )
                    {
                        mesh = meshGroup._mesh;

                        // Heap relocation optimization.
                        locals.reserve ( PBR_POINT_LIGHT_MAX_SHADOW_CASTER_INSTANCE_COUNT );
                    }

                    locals.push_back ( geometryData._local );
                }
            }
        }

        // Commit uniform buffers for batch meshes.
        for ( auto const& batch : casters._batches )
        {
            auto const& caster = batch.second;
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

                _shadowmapBufferPool.Push ( renderer, commandBuffer, &instanceData );
                remain -= batches;
            }
            while ( remain );
        }
    }

    _shadowmapBufferPool.IssueSync ( renderer.GetDevice (), commandBuffer );
}

void PointLightPass::UpdateLightGPUData ( android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer,
    GXMat4 const &viewProjection
) noexcept
{
    size_t const lightCount = _interacts.size ();

    PointLightLightupProgram::VolumeData volumeData {};
    GXMat4& transform = volumeData._transform;
    GXMat4 local {};
    GXVec3 alpha {};

    for ( size_t i = 0U; i < lightCount; ++i )
    {
        auto const& [light, casters] = _interacts[ i ];

        // NOLINTNEXTLINE - downcast.
        auto& pointLight = static_cast<PointLight&> ( *light );
        GXAABB const bounds = pointLight.GetBounds ();

        local.Scale ( bounds.GetWidth (), bounds.GetHeight (), bounds.GetDepth () );
        bounds.GetCenter ( alpha );
        local.SetW ( alpha );
        transform.Multiply ( local, viewProjection );

        _lightBufferPool.Push ( renderer, commandBuffer, &volumeData );
    }

    _lightBufferPool.IssueSync ( renderer.GetDevice (), commandBuffer );
}

} // namespace pbr
