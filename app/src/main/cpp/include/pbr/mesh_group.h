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
        GXVec4 const &color0,
        GXVec4 const &color1,
        GXVec4 const &color2,
        GXVec4 const &color3
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
