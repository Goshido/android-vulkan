#include <precompiled_headers.hpp>
#include <pbr/stipple_subpass.hpp>


namespace pbr {

bool StippleSubpass::Init ( android_vulkan::Renderer &renderer,
    VkExtent2D const &resolution,
    VkRenderPass renderPass
) noexcept
{
    return _program.Init ( renderer, renderPass, 0U, resolution );
}

void StippleSubpass::Destroy ( VkDevice device ) noexcept
{
    _program.Destroy ( device );
}

void StippleSubpass::Execute ( VkCommandBuffer commandBuffer,
    GeometryPool &geometryPool,
    MaterialPool &materialPool,
    RenderSessionStats &renderSessionStats,
    VkDescriptorSet samplerDescriptorSet,
    bool &isSamplerUsed
) noexcept
{
    AV_VULKAN_GROUP ( commandBuffer, "Stipple geometry" )

    if ( _sceneData.empty () )
        return;

    if ( !isSamplerUsed )
    {
        _program.SetDescriptorSet ( commandBuffer, &samplerDescriptorSet, 0U, 1U );
        isSamplerUsed = true;
    }

    AppendDrawcalls ( commandBuffer, _program, geometryPool, materialPool, renderSessionStats );
}

void StippleSubpass::UpdateGPUData ( VkCommandBuffer commandBuffer,
    GeometryPool &geometryPool,
    MaterialPool &materialPool,
    GXMat4 const &view,
    GXMat4 const &viewProjection
) noexcept
{
    AV_VULKAN_GROUP ( commandBuffer, "Upload stipple data" )

    // Note all stipple objects already pass frustum test at RenderSession::SubmitMesh call.
    // So all meshes are visible.

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

                GXMat4 const &local = opaqueData._local;
                positionData._localViewProj[ instanceIndex ].Multiply ( local, viewProjection );

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

void StippleSubpass::ReportGeometry ( RenderSessionStats &renderSessionStats,
    uint32_t vertexCount,
    uint32_t instanceCount
) noexcept
{
    renderSessionStats.RenderStipple ( vertexCount, instanceCount );
}

} // namespace pbr
