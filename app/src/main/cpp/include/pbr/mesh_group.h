#ifndef MESH_GROUP_H
#define MESH_GROUP_H


#include "types.h"
#include "opaque_data.h"


namespace pbr {

struct MeshGroup final
{
    MeshRef                     _mesh;
    std::vector<OpaqueData>     _opaqueData;

    explicit MeshGroup ( MeshRef &mesh,
        GXMat4 const &local,
        GXColorRGB const &color0,
        GXColorRGB const &color1,
        GXColorRGB const &color2,
        GXColorRGB const &color3
    );

    MeshGroup ( MeshGroup &&other ) = default;
    ~MeshGroup () = default;

    MeshGroup () = delete;
    MeshGroup ( const MeshGroup &other ) = delete;
    MeshGroup& operator = ( const MeshGroup &other ) = delete;

    MeshGroup& operator = ( MeshGroup &&other ) = default;
};

} // namespace pbr


#endif // MESH_GROUP_H
