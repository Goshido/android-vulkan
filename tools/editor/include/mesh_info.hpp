#ifndef EDITOR_MESH_INFO_HPP
#define EDITOR_MESH_INFO_HPP


#include "color_data.hpp"
#include "pbr_material.hpp"


namespace editor {

struct MeshInfo final
{
    PBRMaterial     _material {};
    GXMat4          _local {};
    GXAABB          _boundLocal {};
    GXAABB          _boundWorld {};
    ColorData       _color {};
};

} // namespace editor


#endif // EDITOR_MESH_INFO_HPP
