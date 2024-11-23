#include <precompiled_headers.hpp>
#include <pbr/geometry_call.hpp>


namespace pbr {

GeometryCall::GeometryCall ( MeshRef &mesh,
    GXMat4 const &local,
    GXAABB const &worldBounds,
    GeometryPassProgram::ColorData const &colorData
) noexcept
{
    Append ( mesh, local, worldBounds, colorData );
}

GeometryCall::BatchList const &GeometryCall::GetBatchList () const noexcept
{
    return _batch;
}

GeometryCall::UniqueList const &GeometryCall::GetUniqueList () const noexcept
{
    return _unique;
}

void GeometryCall::Append ( MeshRef &mesh,
    GXMat4 const &local,
    GXAABB const &worldBounds,
    GeometryPassProgram::ColorData const &colorData
) noexcept
{
    if ( mesh->IsUnique () )
    {
        AddUnique ( mesh, local, worldBounds, colorData );
        return;
    }

    AddBatch ( mesh, local, worldBounds, colorData );
}

void GeometryCall::AddBatch ( MeshRef &mesh,
    GXMat4 const &local,
    GXAABB const &worldBounds,
    GeometryPassProgram::ColorData const &colorData
) noexcept
{
    std::string const &name = mesh->GetName ();
    auto findResult = _batch.find ( name );

    if ( findResult == _batch.cend () )
    {
        _batch.insert (
            std::make_pair ( std::string_view ( name ), MeshGroup ( mesh, local, worldBounds, colorData ) )
        );

        return;
    }

    findResult->second._geometryData.push_back (
        GeometryData
        {
            ._isVisible = true,
            ._local = local,
            ._worldBounds = worldBounds,
            ._colorData = colorData
        }
    );
}

void GeometryCall::AddUnique ( MeshRef &mesh,
    GXMat4 const &local,
    GXAABB const &worldBounds,
    GeometryPassProgram::ColorData const &colorData
) noexcept
{
    _unique.emplace_back (
        std::make_pair ( mesh,
            GeometryData
            {
                ._isVisible = true,
                ._local = local,
                ._worldBounds = worldBounds,
                ._colorData = colorData
            }
        )
    );
}

} // namespace pbr
