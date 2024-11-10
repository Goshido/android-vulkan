#include <precompiled_headers.hpp>
#include <pbr/geometry_subpass_base.hpp>


namespace pbr {

void GeometrySubpassBase::Reset () noexcept
{
    _sceneData.clear ();
}

void GeometrySubpassBase::Submit ( MeshRef &mesh,
    MaterialRef const &material,
    GXMat4 const &local,
    GXAABB const &worldBounds,
    GXColorRGB const &color0,
    GXColorRGB const &color1,
    GXColorRGB const &color2,
    GXColorRGB const &emission
) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto &m = static_cast<GeometryPassMaterial &> ( *material );
    auto findResult = _sceneData.find ( m );

    if ( findResult != _sceneData.cend () )
    {
        findResult->second.Append ( mesh, local, worldBounds, color0, color1, color2, emission );
        return;
    }

    _sceneData.emplace (
        std::make_pair ( m, GeometryCall ( mesh, local, worldBounds, color0, color1, color2, emission ) )
    );
}

void GeometrySubpassBase::AppendDrawcalls ( VkCommandBuffer commandBuffer,
    GeometryPassProgram &program,
    GeometryPool &geometryPool,
    MaterialPool &materialPool,
    RenderSessionStats &renderSessionStats
) noexcept
{
    bool isProgramBind = false;
    constexpr VkDeviceSize const offset[] = { 0U, 0U };
    static_assert ( std::size ( offset ) == android_vulkan::MeshGeometry::GetVertexBufferCount () );

    for ( auto const &call : _sceneData )
    {
        GeometryCall const &geometryCall = call.second;
        VkDescriptorSet textureSet = materialPool.Acquire ();

        if ( !isProgramBind )
        {
            program.Bind ( commandBuffer );
            isProgramBind = true;
        }

        bool isUniformBind = false;

        auto const instanceDrawer = [ & ] ( MeshRef const &mesh, uint32_t batches ) noexcept {
            if ( isUniformBind )
            {
                VkDescriptorSet ds = geometryPool.Acquire ();
                program.SetDescriptorSet ( commandBuffer, &ds, 2U, 1U );
            }
            else
            {
                VkDescriptorSet sets[] = { textureSet, geometryPool.Acquire () };

                program.SetDescriptorSet ( commandBuffer,
                    sets,
                    1U,
                    static_cast<uint32_t> ( std::size ( sets ) )
                );

                isUniformBind = true;
            }

            vkCmdBindVertexBuffers ( commandBuffer,
                0U,
                android_vulkan::MeshGeometry::GetVertexBufferCount (),
                mesh->GetVertexBuffers (),
                offset
            );

            vkCmdBindIndexBuffer ( commandBuffer, mesh->GetIndexBuffer (), 0U, VK_INDEX_TYPE_UINT32 );

            vkCmdDrawIndexed ( commandBuffer,
                mesh->GetVertexCount (),
                batches,
                0U,
                0U,
                0U
            );

            ReportGeometry ( renderSessionStats, mesh->GetVertexCount (), batches );
        };

        for ( auto const &[mesh, geometryData] : geometryCall.GetUniqueList () )
        {
            if ( geometryData._isVisible )
            {
                instanceDrawer ( mesh, 1U );
            }
        }

        for ( auto const &item : geometryCall.GetBatchList () )
        {
            MeshGroup const &group = item.second;
            MeshRef const &mesh = group._mesh;
            size_t instanceCount = 0U;

            for ( auto const &geometryData : group._geometryData )
            {
                if ( geometryData._isVisible )
                {
                    ++instanceCount;
                }
            }

            size_t instanceIndex = 0U;
            size_t batches = 0U;

            while ( instanceIndex < instanceCount )
            {
                batches = std::min ( instanceCount - instanceIndex,
                    static_cast<size_t> ( PBR_OPAQUE_MAX_INSTANCE_COUNT )
                );

                instanceIndex += batches;

                if ( batches < PBR_OPAQUE_MAX_INSTANCE_COUNT )
                    continue;

                instanceDrawer ( mesh, static_cast<uint32_t> ( batches ) );
                batches = 0U;
            }

            if ( batches )
            {
                instanceDrawer ( mesh, static_cast<uint32_t> ( batches ) );
            }
        }
    }
}

} // namespace pbr
