#include <precompiled_headers.hpp>
#include <platform/android/pbr/mesh_group.hpp>


namespace pbr {

MeshGroup::MeshGroup ( MeshRef &mesh,
    GXMat4 const &local,
    GXAABB const &worldBounds,
    GeometryPassProgram::ColorData const &colorData
) noexcept:
    _mesh ( mesh )
{
    _geometryData.emplace_back (
        GeometryData {
            ._isVisible = true,
            ._local = local,
            ._worldBounds = worldBounds,
            ._colorData = colorData
        }
    );
}

} // namespace pbr
