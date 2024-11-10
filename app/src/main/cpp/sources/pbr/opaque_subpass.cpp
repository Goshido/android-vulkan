#include <precompiled_headers.hpp>
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
    return _program.Init ( renderer, renderPass, 0U, resolution );
}

void OpaqueSubpass::Destroy ( VkDevice device ) noexcept
{
    _program.Destroy ( device );
}

void OpaqueSubpass::Execute ( VkCommandBuffer commandBuffer,
    GeometryPool &geometryPool,
    MaterialPool &materialPool,
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

    AppendDrawcalls ( commandBuffer, _program, geometryPool, materialPool, renderSessionStats );
}

void OpaqueSubpass::UpdateGPUData ( VkCommandBuffer commandBuffer,
    GeometryPool &geometryPool,
    MaterialPool &materialPool,
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

    GeometryPassProgram::InstancePositionData positionData {};
    GeometryPassProgram::InstanceNormalData normalData {};
    GeometryPassProgram::InstanceColorData colorData {};

    for ( auto const &call : _sceneData )
    {
        GeometryCall const &geometryCall = call.second;

        for ( auto const &[mesh, geometryData] : geometryCall.GetUniqueList () )
        {
            auto &uniqueGeometryData = const_cast<GeometryData &> ( geometryData );

            if ( !frustum.IsVisible ( uniqueGeometryData._worldBounds ) )
            {
                uniqueGeometryData._isVisible = false;
                continue;
            }

            uniqueGeometryData._isVisible = true;
            GXMat4 const &local = geometryData._local;
            positionData._localViewProj[ 0U ].Multiply ( local, viewProjection );

            GXMat4 localView {};
            localView.Multiply ( local, view );

            auto &x = *reinterpret_cast<GXVec3*> ( &localView );
            auto &y = *reinterpret_cast<GXVec3*> ( &localView._m[ 1U ][ 0U ] );
            x.Normalize ();

            auto &z = *reinterpret_cast<GXVec3*> ( &localView._m[ 2U ][ 0U ] );
            y.Normalize ();
            z.Normalize ();

            normalData._localView[ 0U ].FromFast ( localView );

            colorData._colorData[ 0U ] =
            {
                ._color0 = geometryData._color0,
                ._color1 = geometryData._color1,
                ._color2 = geometryData._color2,
                ._emission = geometryData._emission
            };

            geometryPool.Push ( commandBuffer, positionData, normalData, colorData, 1U );
        }

        for ( auto const &item : geometryCall.GetBatchList () )
        {
            MeshGroup const &group = item.second;
            size_t instanceIndex = 0U;

            for ( auto const &opaqueData : group._geometryData )
            {
                if ( instanceIndex >= PBR_OPAQUE_MAX_INSTANCE_COUNT )
                {
                    geometryPool.Push ( commandBuffer,
                        positionData,
                        normalData,
                        colorData,
                        PBR_OPAQUE_MAX_INSTANCE_COUNT
                    );

                    instanceIndex = 0U;
                }

                auto &batchGeometryData = const_cast<GeometryData &> ( opaqueData );

                if ( !frustum.IsVisible ( batchGeometryData._worldBounds ) )
                {
                    batchGeometryData._isVisible = false;
                    continue;
                }

                GXMat4 const &local = opaqueData._local;
                positionData._localViewProj[ instanceIndex ].Multiply ( local, viewProjection );
                batchGeometryData._isVisible = true;

                GXMat4 localView {};
                localView.Multiply ( local, view );
                auto &x = *reinterpret_cast<GXVec3*> ( &localView );
                auto &y = *reinterpret_cast<GXVec3*> ( &localView._m[ 1U ][ 0U ] );
                x.Normalize ();

                auto &z = *reinterpret_cast<GXVec3*> ( &localView._m[ 2U ][ 0U ] );
                y.Normalize ();
                z.Normalize ();

                normalData._localView[ instanceIndex ].FromFast ( localView );

                colorData._colorData[ instanceIndex++ ] =
                {
                    ._color0 = opaqueData._color0,
                    ._color1 = opaqueData._color1,
                    ._color2 = opaqueData._color2,
                    ._emission = opaqueData._emission
                };
            }

            if ( !instanceIndex ) [[likely]]
                continue;

            geometryPool.Push ( commandBuffer, positionData, normalData, colorData, instanceIndex );
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
