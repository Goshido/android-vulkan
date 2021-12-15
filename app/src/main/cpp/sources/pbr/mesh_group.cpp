#include <pbr/mesh_group.h>


namespace pbr {

MeshGroup::MeshGroup ( MeshRef &mesh,
    const GXMat4 &local,
    GXAABB const &worldBounds,
    GXColorRGB const &color0,
    GXColorRGB const &color1,
    GXColorRGB const &color2,
    GXColorRGB const &color3
) noexcept:
    _mesh ( mesh )
{
    _opaqueData.emplace_back (
        OpaqueData {
            ._isVisible = true,
            ._local = local,
            ._worldBounds = worldBounds,
            ._color0 = color0,
            ._color1 = color1,
            ._color2 = color2,
            ._color3 = color3
        }
    );
}

} // namespace pbr
