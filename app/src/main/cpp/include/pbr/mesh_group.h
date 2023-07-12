#ifndef PBR_MESH_GROUP_H
#define PBR_MESH_GROUP_H


#include "types.h"
#include "geometry_data.h"


namespace pbr {

struct MeshGroup final
{
    MeshRef                         _mesh;
    std::vector<GeometryData>       _geometryData;

    MeshGroup () = delete;

    MeshGroup ( MeshGroup const & ) = delete;
    MeshGroup &operator = ( MeshGroup const & ) = delete;

    MeshGroup ( MeshGroup && ) = default;
    MeshGroup &operator = ( MeshGroup && ) = default;

    explicit MeshGroup ( MeshRef &mesh,
        GXMat4 const &local,
        GXAABB const &worldBounds,
        GXColorRGB const &color0,
        GXColorRGB const &color1,
        GXColorRGB const &color2,
        GXColorRGB const &emission
    ) noexcept;

    ~MeshGroup () = default;
};

} // namespace pbr


#endif // PBR_MESH_GROUP_H
