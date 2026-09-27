#ifndef EDITOR_SDF_SHAPE_HPP
#define EDITOR_SDF_SHAPE_HPP


#include <platform/windows/pbr/gizmo_shapes.inc>
#include <vulkan_utils.hpp>


namespace editor {

enum class eSDFShape : uint32_t
{
    Sphere = SHAPE_SPHERE,
    Cone = SHAPE_CONE,
    Box = SHAPE_BOX,
    LineSegment = SHAPE_LINE_SEGMENT,
    Ring = SHAPE_RING
};

AV_DX_ALIGNMENT_BEGIN

struct SDFShape final
{
    uint32_t    _palette: 24;
    uint32_t    _type: 8;
};

AV_DX_ALIGNMENT_END

} // namespace editor


#endif // EDITOR_SDF_SHAPE_HPP
