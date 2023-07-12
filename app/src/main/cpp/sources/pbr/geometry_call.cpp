#include <pbr/geometry_call.hpp>


namespace pbr {

GeometryCall::GeometryCall ( MeshRef &mesh,
    GXMat4 const &local,
    GXAABB const &worldBounds,
    GXColorRGB const &color0,
    GXColorRGB const &color1,
    GXColorRGB const &color2,
    GXColorRGB const &emission
) noexcept
{
    Append ( mesh, local, worldBounds, color0, color1, color2, emission );
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
    GXColorRGB const &color0,
    GXColorRGB const &color1,
    GXColorRGB const &color2,
    GXColorRGB const &emission
) noexcept
{
    if ( mesh->IsUnique () )
    {
        AddUnique ( mesh, local, worldBounds, color0, color1, color2, emission );
        return;
    }

    AddBatch ( mesh, local, worldBounds, color0, color1, color2, emission );
}

void GeometryCall::AddBatch ( MeshRef &mesh,
    GXMat4 const &local,
    GXAABB const &worldBounds,
    GXColorRGB const &color0,
    GXColorRGB const &color1,
    GXColorRGB const &color2,
    GXColorRGB const &emission
) noexcept
{
    std::string const &name = mesh->GetName ();
    auto findResult = _batch.find ( name );

    if ( findResult == _batch.cend () )
    {
        _batch.insert (
            std::make_pair (
                std::string_view ( name ), MeshGroup ( mesh, local, worldBounds, color0, color1, color2, emission )
            )
        );

        return;
    }

    findResult->second._geometryData.emplace_back (
        GeometryData {
            ._isVisible = true,
            ._local = local,
            ._worldBounds = worldBounds,
            ._color0 = color0,
            ._color1 = color1,
            ._color2 = color2,
            ._emission = emission
        }
    );
}

void GeometryCall::AddUnique ( MeshRef &mesh,
    GXMat4 const &local,
    GXAABB const &worldBounds,
    GXColorRGB const &color0,
    GXColorRGB const &color1,
    GXColorRGB const &color2,
    GXColorRGB const &emission
) noexcept
{
    _unique.emplace_back (
        std::make_pair ( mesh,
            GeometryData {
                ._isVisible = true,
                ._local = local,
                ._worldBounds = worldBounds,
                ._color0 = color0,
                ._color1 = color1,
                ._color2 = color2,
                ._emission = emission
            }
        )
    );
}

} // namespace pbr
