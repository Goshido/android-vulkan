#ifndef EDITOR_GIZMO_INFO_HPP
#define EDITOR_GIZMO_INFO_HPP


#include "sdf_shape.hpp"
#include "sdf_pixel.hpp"
#include "sdf_vertex.hpp"


namespace editor {

class GizmoNode;

struct GizmoInfo final
{
    GizmoNode*      _node = nullptr;
    SDFVertex       _vertex {};
    SDFPixel        _pixel {};
    eSDFShape       _shape = eSDFShape::Sphere;
};

} // namespace editor


#endif // EDITOR_GIZMO_INFO_HPP
