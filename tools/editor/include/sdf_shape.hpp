#ifndef EDITOR_SDF_SHAPE_HPP
#define EDITOR_SDF_SHAPE_HPP


#include <platform/windows/pbr/gizmo_shapes.inc>


namespace editor {

enum class eSDFShape : uint32_t
{
    Sphere = SHAPE_SPHERE,
    Cone = SHAPE_CONE,
    Box = SHAPE_BOX,
    LineSegment = SHAPE_LINE_SEGMENT,
    CappedTorus = SHAPE_CAPPED_TORUS
};

} // namespace editor


#endif // EDITOR_SDF_SHAPE_HPP
