#include <pbr/mesh_group.h>


namespace pbr {

MeshGroup::MeshGroup ( MeshRef &mesh,
    const GXMat4 &local,
    GXAABB const &worldBounds,
    android_vulkan::ColorUnorm const &color0,
    android_vulkan::ColorUnorm const &color1,
    android_vulkan::ColorUnorm const &color2,
    android_vulkan::ColorUnorm const &color3
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
