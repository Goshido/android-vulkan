#ifndef MESH_GROUP_H
#define MESH_GROUP_H


#include "types.h"
#include "opaque_data.h"


namespace pbr {

struct MeshGroup final
{
    MeshRef                     _mesh;
    std::vector<OpaqueData>     _opaqueData;

    MeshGroup () = delete;

    MeshGroup ( MeshGroup const & ) = delete;
    MeshGroup& operator = ( MeshGroup const & ) = delete;

    MeshGroup ( MeshGroup && ) = default;
    MeshGroup& operator = ( MeshGroup && ) = default;

    explicit MeshGroup ( MeshRef &mesh,
        GXMat4 const &local,
        GXAABB const &worldBounds,
        android_vulkan::Half4 const &color0,
        android_vulkan::Half4 const &color1,
        android_vulkan::Half4 const &color2,
        android_vulkan::Half4 const &color3
    );

    ~MeshGroup () = default;
};

} // namespace pbr


#endif // MESH_GROUP_H
