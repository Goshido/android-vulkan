#include <pbr/mesh_group.h>


namespace pbr {

MeshGroup::MeshGroup ( MeshRef &mesh,
    const GXMat4 &local,
    android_vulkan::Half4 const &color0,
    android_vulkan::Half4 const &color1,
    android_vulkan::Half4 const &color2,
    android_vulkan::Half4 const &color3
):
    _mesh ( mesh )
{
    _opaqueData.emplace_back (
        OpaqueData {
            ._local = local,
            ._color0 = color0,
            ._color1 = color1,
            ._color2 = color2,
            ._color3 = color3
        }
    );
}

} // namespace pbr
