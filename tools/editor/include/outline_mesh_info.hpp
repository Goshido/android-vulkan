#ifndef EDITOR_OUTLINE_MESH_INFO_HPP
#define EDITOR_OUTLINE_MESH_INFO_HPP


#include "transform.hpp"


namespace editor {

class OutlineMeshNode;

struct OutlineMeshInfo final
{
    OutlineMeshNode*    _node = nullptr;
    Model               _model {};
    GXAABB              _boundWorld {};
};

} // namespace editor


#endif // EDITOR_OUTLINE_MESH_INFO_HPP
