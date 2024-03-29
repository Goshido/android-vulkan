#include <pbr/mesh_group.hpp>


namespace pbr {

MeshGroup::MeshGroup ( MeshRef &mesh,
    const GXMat4 &local,
    GXAABB const &worldBounds,
    GXColorRGB const &color0,
    GXColorRGB const &color1,
    GXColorRGB const &color2,
    GXColorRGB const &emission
) noexcept:
    _mesh ( mesh )
{
    _geometryData.emplace_back (
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

} // namespace pbr
