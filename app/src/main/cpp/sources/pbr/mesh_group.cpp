#include <pbr/mesh_group.h>


namespace pbr {

MeshGroup::MeshGroup ( MeshRef &mesh, const GXMat4 &local ):
    _mesh ( mesh )
{
    _locals.push_back ( local );
}

} // namespace pbr
