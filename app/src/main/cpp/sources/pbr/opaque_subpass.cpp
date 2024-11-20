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
    constexpr size_t normalIndexMask = ~( std::numeric_limits<size_t>::max () - 1U );

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

            GXQuat q {};
            q.FromFast ( localView );
            normalData._localView[ 0U ]._q0 = q.Compress64 ();

            colorData._colorData[ 0U ] = geometryData._colorData;
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

                GXQuat q {};
                q.FromFast ( localView );
                uint64_t const tbn64 = q.Compress64 ();

                GeometryPassProgram::TBN64 &dst = normalData._localView[ instanceIndex >> 1U ];
                size_t const ind = instanceIndex & normalIndexMask;
                uint64_t const casesQ0[] = { tbn64, dst._q0 };
                uint64_t const casesQ1[] = { dst._q1, tbn64 };
                dst._q0 = casesQ0[ ind ];
                dst._q1 = casesQ1[ ind ];

                colorData._colorData[ instanceIndex++ ] = opaqueData._colorData;
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
