#include <precompiled_headers.hpp>
#include <pbr/point_light_pass.hpp>
#include <av_assert.hpp>
#include <trace.hpp>


namespace pbr {

namespace {

constexpr uint32_t SHADOWMAP_RESOLUTION = 512U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

PointLightPass::PointLightPass ( UMAUniformPool &volumeDataPool ) noexcept:
    _volumeDataPool ( volumeDataPool )
{
    // NOTHING
}

void PointLightPass::ExecuteLightupPhase ( VkCommandBuffer commandBuffer,
    android_vulkan::MeshGeometry &unitCube
) noexcept
{
    if ( _interacts.empty () )
        return;

    _lightup.BindProgram ( commandBuffer );

    android_vulkan::MeshBufferInfo const &bufferInfo = unitCube.GetMeshBufferInfo ();
    VkBuffer buffer = bufferInfo._buffer;
    vkCmdBindVertexBuffers ( commandBuffer, 0U, 1U, &bufferInfo._buffer, bufferInfo._vertexDataOffsets );
    vkCmdBindIndexBuffer ( commandBuffer, buffer, 0U, bufferInfo._indexType );

    uint32_t const volumeVertices = unitCube.GetVertexCount ();
    size_t const limit = _interacts.size ();

    for ( size_t i = 0U; i < limit; ++i )
        _lightup.Lightup ( commandBuffer, _volumeDataPool.Acquire (), volumeVertices );

    _lightup.Commit ();
}

bool PointLightPass::ExecuteShadowPhase ( VkCommandBuffer commandBuffer ) noexcept
{
    return _interacts.empty () || GenerateShadowmaps ( commandBuffer );
}

bool PointLightPass::Init ( android_vulkan::Renderer &renderer,
    VkExtent2D const &resolution,
    VkRenderPass lightupRenderPass
) noexcept
{
    constexpr VkExtent2D shadowmapResolution
    {
        .width = SHADOWMAP_RESOLUTION,
        .height = SHADOWMAP_RESOLUTION
    };

    return CreateShadowmapRenderPass ( renderer.GetDevice () ) &&
        _shadowmapProgram.Init ( renderer, _shadowmapRenderPass, 0U, shadowmapResolution ) &&
        _shadowmapPool.Init ( renderer ) &&
        _lightup.Init ( renderer, lightupRenderPass, resolution );
}

void PointLightPass::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();
    _lightup.Destroy ( renderer );

    if ( !_shadowmaps.empty () )
    {
        for ( auto &[image, framebuffer ] : _shadowmaps )
        {
            if ( framebuffer == VK_NULL_HANDLE )
                continue;

            image->FreeResources ( renderer );
            vkDestroyFramebuffer ( device, framebuffer, nullptr );
        }

        _shadowmaps.clear ();
    }

    _shadowmapPool.Destroy ( renderer );
    _shadowmapProgram.Destroy ( device );

    if ( _shadowmapRenderPass == VK_NULL_HANDLE )
        return;

    vkDestroyRenderPass ( device, _shadowmapRenderPass, nullptr );
    _shadowmapRenderPass = VK_NULL_HANDLE;
}

size_t PointLightPass::GetPointLightCount () const noexcept
{
    return _interacts.size ();
}

PointLightPass::PointLightInfo PointLightPass::GetPointLightInfo ( size_t lightIndex ) const noexcept
{
    AV_ASSERT ( lightIndex < _interacts.size () )

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
    _interacts.emplace_back ( light, ShadowCasters () );
}

bool PointLightPass::UploadGPUData ( android_vulkan::Renderer &renderer,
    SceneData const &sceneData,
    size_t opaqueMeshCount,
    GXMat4 const &viewerLocal,
    GXMat4 const &view,
    GXMat4 const &viewProjection
) noexcept
{
    if ( _interacts.empty () )
        return true;

    VkDevice device = renderer.GetDevice ();

    bool const result = UpdateShadowmapGPUData ( device, sceneData, opaqueMeshCount ) &&
        ReserveShadowmaps ( renderer ) &&
        _lightup.UpdateGPUData ( device, *this, viewerLocal, view );

    if ( !result ) [[unlikely]]
        return false;

    UpdateLightGPUData ( viewProjection );
    return true;
}

bool PointLightPass::CreateShadowmapRenderPass ( VkDevice device ) noexcept
{
    constexpr static VkAttachmentDescription depthAttachment
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
    };

    constexpr static VkAttachmentReference depthAttachmentReference
    {
        .attachment = 0U,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    constexpr static VkSubpassDescription subpass
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
    };

    constexpr static uint32_t viewMask = 0b00000000'00000000'00000000'00111111U;
    constexpr static uint32_t correlationMask = 0b00000000'00000000'00000000'00111111U;

    constexpr static VkRenderPassMultiviewCreateInfo multiviewInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO,
        .pNext = nullptr,
        .subpassCount = 1U,
        .pViewMasks = &viewMask,
        .dependencyCount = 0U,
        .pViewOffsets = nullptr,
        .correlationMaskCount = 1U,
        .pCorrelationMasks = &correlationMask
    };

    constexpr static VkSubpassDependency const dependencies[] =
    {
        {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0U,
            .srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,

            .dstStageMask = AV_VK_FLAG ( VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT ) |
                AV_VK_FLAG ( VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT ),

            .srcAccessMask = VK_ACCESS_NONE,
            .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
        },
        {
            .srcSubpass = 0U,
            .dstSubpass = VK_SUBPASS_EXTERNAL,
            .srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
        }
    };

    constexpr VkRenderPassCreateInfo renderPassInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = &multiviewInfo,
        .flags = 0U,
        .attachmentCount = 1U,
        .pAttachments = &depthAttachment,
        .subpassCount = 1U,
        .pSubpasses = &subpass,
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

    AV_SET_VULKAN_OBJECT_NAME ( device, _shadowmapRenderPass, VK_OBJECT_TYPE_RENDER_PASS, "Point light shadowmap" )

    // Must be static as well because it lose the function scope [read as garbage in debug builds].
    constexpr static VkClearValue clearValues
    {
        .depthStencil
        {
            .depth = 0.0F,
            .stencil = 0U
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
    _shadowmapRenderPassInfo.clearValueCount = 1U;
    _shadowmapRenderPassInfo.pClearValues = &clearValues;

    return true;
}

bool PointLightPass::GenerateShadowmaps ( VkCommandBuffer commandBuffer) noexcept
{
    for ( auto const &[light, casters] : _interacts )
    {
        AV_VULKAN_GROUP ( commandBuffer, "Point light shadowmap" )

        _shadowmapRenderPassInfo.framebuffer = _shadowmaps[ _usedShadowmaps++ ].second;
        vkCmdBeginRenderPass ( commandBuffer, &_shadowmapRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

        // Note it's required to set graphic pipeline object every time when new render pass or sub pass begin with
        // multiview style rendering. see VkRenderPassMultiviewCreateInfo remarks in Vulkan spec.
        _shadowmapProgram.Bind ( commandBuffer );

        for ( auto const &unique : casters._uniques )
        {
            android_vulkan::MeshBufferInfo const &bufferInfo = unique->GetMeshBufferInfo ();
            VkBuffer buffer = bufferInfo._buffer;
            vkCmdBindVertexBuffers ( commandBuffer, 0U, 1U, &buffer, bufferInfo._vertexDataOffsets );
            vkCmdBindIndexBuffer ( commandBuffer, buffer, 0U, bufferInfo._indexType );

            _shadowmapProgram.SetDescriptorSet ( commandBuffer, _shadowmapPool.Acquire () );
            vkCmdDrawIndexed ( commandBuffer, unique->GetVertexCount (), 1U, 0U, 0, 0U );
        }

        for ( auto const &casterInfo : casters._batches )
        {
            auto const &[mesh, transforms] = casterInfo.second;
            android_vulkan::MeshBufferInfo const &bufferInfo = mesh->GetMeshBufferInfo ();
            VkBuffer buffer = bufferInfo._buffer;
            vkCmdBindVertexBuffers ( commandBuffer, 0U, 1U, &buffer, bufferInfo._vertexDataOffsets );
            vkCmdBindIndexBuffer ( commandBuffer, buffer, 0U, bufferInfo._indexType );

            size_t remain = transforms.size ();
            uint32_t const vertexCount = mesh->GetVertexCount ();

            do
            {
                size_t const instances = std::min ( remain,
                    static_cast<size_t> ( PBR_POINT_LIGHT_MAX_SHADOW_CASTER_INSTANCE_COUNT )
                );

                _shadowmapProgram.SetDescriptorSet ( commandBuffer, _shadowmapPool.Acquire () );

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

    _shadowmapPool.Commit ();
    return true;
}

bool PointLightPass::ReserveShadowmaps ( android_vulkan::Renderer &renderer ) noexcept
{
    size_t const needed = _interacts.size ();
    size_t const allocated = _shadowmaps.size ();

    if ( needed <= allocated ) [[likely]]
        return true;

    constexpr VkExtent2D resolution
    {
        .width = SHADOWMAP_RESOLUTION,
        .height = SHADOWMAP_RESOLUTION
    };

    constexpr VkImageUsageFlags flags = AV_VK_FLAG ( VK_IMAGE_USAGE_SAMPLED_BIT ) |
        AV_VK_FLAG ( VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT );

    VkFramebufferCreateInfo framebufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .renderPass = _shadowmapRenderPass,
        .attachmentCount = 1U,
        .pAttachments = nullptr,
        .width = resolution.width,
        .height = resolution.height,
        .layers = 1U
    };

    VkFormat const depthFormat = renderer.GetDefaultDepthFormat ();
    VkDevice device = renderer.GetDevice ();

    for ( size_t i = allocated; i < needed; ++i )
    {
        PointLightShadowmapInfo info;
        auto &[shadowmap, framebuffer] = info;
        shadowmap = std::make_shared<android_vulkan::TextureCube> ();

        if ( !shadowmap->CreateRenderTarget ( renderer, resolution, depthFormat, flags ) ) [[unlikely]]
            return false;

        framebufferInfo.pAttachments = &shadowmap->GetImageView ();

        bool const result = android_vulkan::Renderer::CheckVkResult (
            vkCreateFramebuffer ( device, &framebufferInfo, nullptr, &framebuffer ),
            "pbr::RenderSession::ReserveShadowmaps",
            "Can't create framebuffer"
        );

        if ( !result ) [[unlikely]]
        {
            shadowmap->FreeResources ( renderer );
            return false;
        }

        AV_SET_VULKAN_OBJECT_NAME ( device, framebuffer, VK_OBJECT_TYPE_FRAMEBUFFER, "Point light #%zu", i )
        _shadowmaps.push_back ( std::move ( info ) );
    }

    return true;
}

bool PointLightPass::UpdateShadowmapGPUData ( VkDevice device,
    SceneData const &sceneData,
    size_t opaqueMeshCount
) noexcept
{
    PointLightShadowmapGeneratorProgram::InstanceData instanceData {};

    auto append = [ & ] ( PointLight::Matrices const &matrices, size_t instance, GXMat4 const &local ) {
        PointLightShadowmapGeneratorProgram::ObjectData &objectData = instanceData._instanceData[ instance ];

        for ( size_t i = 0U; i < PBR_POINT_LIGHT_SHADOW_CASTER_PROJECTION_COUNT; ++i )
        {
            objectData._transform[ i ].Multiply ( local, matrices[ i ] );
        }
    };

    for ( auto &[light, casters] : _interacts )
    {
        // NOLINTNEXTLINE - downcast.
        auto &pointLight = static_cast<PointLight &> ( *light );
        GXAABB const &lightBounds = pointLight.GetBounds ();
        PointLight::Matrices const &matrices = pointLight.GetMatrices ();

        for ( auto const &[material, opaque] : sceneData )
        {
            std::vector<MeshRef>* uniques = nullptr;

            for ( auto const &[mesh, opaqueData] : opaque.GetUniqueList () )
            {
                if ( !lightBounds.IsOverlapped ( opaqueData._worldBounds ) )
                    continue;

                if ( !uniques )
                {
                    uniques = &casters._uniques;

                    // Estimating from top. Worst case: all unique meshes are interacting with point light.
                    uniques->reserve ( opaqueMeshCount );
                }

                append ( matrices, 0U, opaqueData._local );
                _shadowmapPool.Push ( instanceData, 1U );
                uniques->push_back ( mesh );
            }

            // Note: the original mesh submit layout is stored by material groups. So there is a probability
            // that there are exact same meshes with different materials. Shadow casters do not care about
            // material only geometry matters. So to make shadowmap calls most efficiently it's need to collect
            // all instances first and only then fill uniform buffers.

            for ( auto const &[name, meshGroup] : opaque.GetBatchList () )
            {
                for ( auto const &geometryData : meshGroup._geometryData )
                {
                    if ( !lightBounds.IsOverlapped ( geometryData._worldBounds ) )
                        continue;

                    auto &[mesh, locals] = casters._batches[ name ];

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
        for ( auto const &batch : casters._batches )
        {
            auto const &caster = batch.second;
            std::vector<GXMat4> const &locals = caster.second;
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

                _shadowmapPool.Push ( instanceData, batches );
                remain -= batches;
            }
            while ( remain );
        }
    }

    return _shadowmapPool.IssueSync ( device );
}

void PointLightPass::UpdateLightGPUData ( GXMat4 const &viewProjection ) noexcept
{
    size_t const lightCount = _interacts.size ();

    VolumeData volumeData {};
    GXMat4 &transform = volumeData._transform;
    GXMat4 local {};
    GXVec3 alpha {};

    for ( size_t i = 0U; i < lightCount; ++i )
    {
        auto const &[light, casters] = _interacts[ i ];

        // NOLINTNEXTLINE - downcast.
        auto &pointLight = static_cast<PointLight &> ( *light );
        GXAABB const bounds = pointLight.GetBounds ();

        local.Scale ( bounds.GetWidth (), bounds.GetHeight (), bounds.GetDepth () );
        bounds.GetCenter ( alpha );
        local.SetW ( alpha );
        transform.Multiply ( local, viewProjection );

        _volumeDataPool.Push ( &volumeData );
    }
}

} // namespace pbr
