#ifndef MESH_GROUP_H
#define MESH_GROUP_H


#include "types.h"


namespace pbr {

struct MeshGroup final
{
    MeshRef                 _mesh;
    std::vector<GXMat4>     _locals;

    explicit MeshGroup ( MeshRef &mesh, const GXMat4 &local );
    MeshGroup ( MeshGroup &&other ) = default;
    ~MeshGroup () = default;

    MeshGroup () = delete;
    MeshGroup ( const MeshGroup &other ) = delete;
    MeshGroup& operator = ( const MeshGroup &other ) = delete;

    MeshGroup& operator = ( MeshGroup &&other ) = default;
};

} // namespace pbr


#endif // MESH_GROUP_H
