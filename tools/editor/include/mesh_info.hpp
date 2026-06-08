#ifndef EDITOR_MESH_INFO_HPP
#define EDITOR_MESH_INFO_HPP


#include "shading.hpp"
#include "transform.hpp"


namespace editor {

class MeshNode;

struct MeshInfo final
{
    MeshNode*       _node = nullptr;
    Transform       _transform {};
    GXAABB          _boundWorld {};
    Shading         _shading {};
    bool            _isStipple = false;
};

} // namespace editor


#endif // EDITOR_MESH_INFO_HPP
