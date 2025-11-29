#ifndef EDITOR_MESH_INFO_HPP
#define EDITOR_MESH_INFO_HPP


#include "color_data.hpp"
#include "pbr_material.hpp"


namespace editor {

struct MeshInfo final
{
    PBRMaterial     _material {};
    GXAABB          _bounds {};
    ColorData       _color {};
    GXMat4          _local {};
};

} // namespace editor


#endif // EDITOR_MESH_INFO_HPP
