#include <pbr/stipple_subpass.h>


namespace pbr {

bool StippleSubpass::Init ( android_vulkan::Renderer &renderer,
    VkCommandPool commandPool,
    VkExtent2D const &resolution,
    VkRenderPass renderPass
) noexcept
{
    if ( !InitBase ( renderer, commandPool ) )
        return false;

    return _program.Init ( renderer, renderPass, 0U, resolution );
}

void StippleSubpass::Destroy ( VkDevice device ) noexcept
{
    _program.Destroy ( device );
    DestroyBase ( device );
}

bool StippleSubpass::Execute ( android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer,
    GXMat4 const &view,
    GXMat4 const &viewProjection,
    DefaultTextureManager const &defaultTextureManager,
    RenderSessionStats &renderSessionStats,
    VkDescriptorSet samplerDescriptorSet,
    bool &isSamplerUsed
) noexcept
{
    if ( _sceneData.empty () )
        return true;

    // Note all stipple objects already pass frustum test at RenderSession::SubmitMesh call.
    // So all meshes are visible.

    if ( !UpdateGPUData ( renderer, view, viewProjection, defaultTextureManager ) )
        return false;

    if ( _uniformStorage.empty () )
        return true;

    if ( !isSamplerUsed )
    {
        _program.SetDescriptorSet ( commandBuffer, &samplerDescriptorSet, 0U, 1U );
        isSamplerUsed = true;
    }

    AppendDrawcalls ( commandBuffer, _program, renderSessionStats );
    return true;
}

void StippleSubpass::ReportGeometry ( RenderSessionStats &renderSessionStats,
    uint32_t vertexCount,
    uint32_t instanceCount
) noexcept
{
    renderSessionStats.RenderStipple ( vertexCount, instanceCount );
}

bool StippleSubpass::UpdateGPUData ( android_vulkan::Renderer &renderer,
    GXMat4 const &view,
    GXMat4 const &viewProjection,
    DefaultTextureManager const &defaultTextureManager
) noexcept
{
    VkDevice device = renderer.GetDevice ();

    if ( !AllocateTransferSystem ( device ) )
        return false;

    constexpr VkCommandBufferBeginInfo beginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    bool result = android_vulkan::Renderer::CheckVkResult ( vkBeginCommandBuffer ( _transferCommandBuffer, &beginInfo ),
        "pbr::StippleSubpass::UpdateGPUData",
        "Can't begin command buffer"
    );

    if ( !result )
        return false;

    size_t images = 0U;

    auto textureBinder = [ & ] ( Texture2DRef const &texture, Texture2DRef const &defaultTexture ) noexcept {
        Texture2DRef const& t = texture ? texture : defaultTexture;
        _imageStorage[ images++ ].imageView = t->GetImageView ();
    };

    size_t uniforms = 0U;

    auto uniformBinder = [ & ] ( VkBuffer uniformBuffer ) noexcept {
        _uniformStorage[ uniforms++ ].buffer = uniformBuffer;
    };

    for ( auto const& [material, call] : _sceneData )
    {
        auto& m = const_cast<GeometryPassMaterial&> ( material );
        textureBinder ( m.GetAlbedo (), defaultTextureManager.GetAlbedo () );
        textureBinder ( m.GetEmission (), defaultTextureManager.GetEmission () );
        textureBinder ( m.GetMask (), defaultTextureManager.GetMask () );
        textureBinder ( m.GetNormal (), defaultTextureManager.GetNormal () );
        textureBinder ( m.GetParam (), defaultTextureManager.GetParams () );
    }

    vkUpdateDescriptorSets ( device, static_cast<uint32_t> ( images ), _materialTransfer._writes.data (), 0U, nullptr );

    GeometryPassProgram::InstanceData instanceData {};
    _uniformPool.Reset ();

    size_t uniformUsed = 0U;
    size_t const maxUniforms = _uniformPool.GetItemCount ();

    constexpr VkPipelineStageFlags syncFlags = AV_VK_FLAG ( VK_PIPELINE_STAGE_VERTEX_SHADER_BIT ) |
        AV_VK_FLAG ( VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT );

    for ( auto const& call : _sceneData )
    {
        GeometryCall const& geometryCall = call.second;

        for ( auto const& [mesh, geometryData] : geometryCall.GetUniqueList () )
        {
            GeometryPassProgram::ObjectData& objectData = instanceData._instanceData[ 0U ];

            if ( uniformUsed >= maxUniforms )
            {
                android_vulkan::LogError (
                    "pbr::StippleSubpass::UpdateGPUData - Uniform pool overflow has been detected (branch 1)!"
                );

                return false;
            }

            GXMat4 const& local = geometryData._local;

            objectData._localView.Multiply ( local, view );
            objectData._localViewProjection.Multiply ( local, viewProjection );
            objectData._color0 = geometryData._color0;
            objectData._color1 = geometryData._color1;
            objectData._color2 = geometryData._color2;
            objectData._emission = geometryData._emission;

            uniformBinder (
                _uniformPool.Acquire ( renderer, _transferCommandBuffer, instanceData._instanceData, syncFlags )
            );

            ++uniformUsed;
        }

        for ( auto const& item : geometryCall.GetBatchList () )
        {
            MeshGroup const& group = item.second;
            size_t instanceIndex = 0U;

            for ( auto const& opaqueData : group._geometryData )
            {
                if ( uniformUsed >= maxUniforms )
                {
                    android_vulkan::LogError (
                        "pbr::StippleSubpass::UpdateGPUData - Uniform pool overflow has been detected (branch 0)!"
                    );

                    return false;
                }

                if ( instanceIndex >= PBR_OPAQUE_MAX_INSTANCE_COUNT )
                {
                    uniformBinder (
                        _uniformPool.Acquire ( renderer, _transferCommandBuffer, instanceData._instanceData, syncFlags )
                    );

                    instanceIndex = 0U;
                    ++uniformUsed;
                }

                GXMat4 const& local = opaqueData._local;

                GeometryPassProgram::ObjectData& objectData = instanceData._instanceData[ instanceIndex ];
                ++instanceIndex;

                objectData._localView.Multiply ( local, view );
                objectData._localViewProjection.Multiply ( local, viewProjection );
                objectData._color0 = opaqueData._color0;
                objectData._color1 = opaqueData._color1;
                objectData._color2 = opaqueData._color2;
                objectData._emission = opaqueData._emission;
            }

            if ( !instanceIndex )
                continue;

            uniformBinder (
                _uniformPool.Acquire ( renderer, _transferCommandBuffer, instanceData._instanceData, syncFlags )
            );

            ++uniformUsed;
        }
    }

    vkUpdateDescriptorSets ( device,
        static_cast<uint32_t> ( uniforms ),
        _uniformTransfer._writes.data (),
        0U,
        nullptr
    );

    result = android_vulkan::Renderer::CheckVkResult ( vkEndCommandBuffer ( _transferCommandBuffer ),
        "pbr::StippleSubpass::UpdateGPUData",
        "Can't end transfer command buffer"
    );

    if ( !result )
        return false;

    return android_vulkan::Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &_submitInfoTransfer, VK_NULL_HANDLE ),
        "pbr::StippleSubpass::UpdateGPUData",
        "Can't submit transfer command buffer"
    );
}

} // namespace pbr
