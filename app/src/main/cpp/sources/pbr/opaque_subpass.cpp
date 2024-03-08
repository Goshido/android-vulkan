#include <pbr/opaque_subpass.hpp>


namespace pbr {

SceneData &OpaqueSubpass::GetSceneData () noexcept
{
    return _sceneData;
}

bool OpaqueSubpass::Init ( android_vulkan::Renderer &renderer,
    VkExtent2D const &resolution,
    VkRenderPass renderPass
) noexcept
{
    return _program.Init ( renderer, renderPass, 0U, nullptr, resolution );
}

void OpaqueSubpass::Destroy ( VkDevice device ) noexcept
{
    _program.Destroy ( device );
}

void OpaqueSubpass::Execute ( VkCommandBuffer commandBuffer,
    MaterialPool &materialPool,
    UniformBufferPoolManager &uniformPool,
    RenderSessionStats &renderSessionStats,
    VkDescriptorSet samplerDescriptorSet,
    bool &isSamplerUsed
) noexcept
{
    AV_VULKAN_GROUP ( commandBuffer, "Opaque geometry" )

    if ( _sceneData.empty () )
        return;

    if ( !isSamplerUsed )
    {
        _program.SetDescriptorSet ( commandBuffer, &samplerDescriptorSet, 0U, 1U );
        isSamplerUsed = true;
    }

    AppendDrawcalls ( commandBuffer, _program, materialPool, uniformPool, renderSessionStats );
}

void OpaqueSubpass::UpdateGPUData ( VkCommandBuffer commandBuffer,
    MaterialPool &materialPool,
    UniformBufferPoolManager &uniformPool,
    GXProjectionClipPlanes const &frustum,
    GXMat4 const &view,
    GXMat4 const &viewProjection
) noexcept
{
    AV_VULKAN_GROUP ( commandBuffer, "Upload opaque data" )

    if ( _sceneData.empty () )
        return;

    for ( auto const &[material, call] : _sceneData )
        materialPool.Push ( const_cast<GeometryPassMaterial &> ( material ) );

    GeometryPassProgram::InstanceData instanceData {};

    for ( auto const &call : _sceneData )
    {
        GeometryCall const &geometryCall = call.second;

        for ( auto const &[mesh, geometryData] : geometryCall.GetUniqueList () )
        {
            GeometryPassProgram::ObjectData &objectData = instanceData._instanceData[ 0U ];
            GXMat4 const &local = geometryData._local;
            auto &uniqueGeometryData = const_cast<GeometryData &> ( geometryData );

            if ( !frustum.IsVisible ( uniqueGeometryData._worldBounds ) )
            {
                uniqueGeometryData._isVisible = false;
                continue;
            }

            uniqueGeometryData._isVisible = true;
            objectData._localView.Multiply ( local, view );
            objectData._localViewProjection.Multiply ( local, viewProjection );
            objectData._color0 = geometryData._color0;
            objectData._color1 = geometryData._color1;
            objectData._color2 = geometryData._color2;
            objectData._emission = geometryData._emission;

            uniformPool.Push ( commandBuffer, instanceData._instanceData, sizeof ( GeometryPassProgram::ObjectData ) );
        }

        for ( auto const &item : geometryCall.GetBatchList () )
        {
            MeshGroup const &group = item.second;
            size_t instanceIndex = 0U;

            for ( auto const &opaqueData : group._geometryData )
            {
                if ( instanceIndex >= PBR_OPAQUE_MAX_INSTANCE_COUNT )
                {
                    uniformPool.Push ( commandBuffer, instanceData._instanceData, sizeof ( instanceData ) );
                    instanceIndex = 0U;
                }

                auto &batchGeometryData = const_cast<GeometryData &> ( opaqueData );

                if ( !frustum.IsVisible ( batchGeometryData._worldBounds ) )
                {
                    batchGeometryData._isVisible = false;
                    continue;
                }

                GeometryPassProgram::ObjectData &objectData = instanceData._instanceData[ instanceIndex++ ];
                GXMat4 const &local = opaqueData._local;
                batchGeometryData._isVisible = true;

                objectData._localView.Multiply ( local, view );
                objectData._localViewProjection.Multiply ( local, viewProjection );
                objectData._color0 = opaqueData._color0;
                objectData._color1 = opaqueData._color1;
                objectData._color2 = opaqueData._color2;
                objectData._emission = opaqueData._emission;
            }

            if ( !instanceIndex )
                continue;

            uniformPool.Push ( commandBuffer,
                instanceData._instanceData,
                instanceIndex * sizeof ( GeometryPassProgram::ObjectData )
            );
        }
    }
}

void OpaqueSubpass::ReportGeometry ( RenderSessionStats &renderSessionStats,
    uint32_t vertexCount,
    uint32_t instanceCount
) noexcept
{
    renderSessionStats.RenderOpaque ( vertexCount, instanceCount );
}

} // namespace pbr
