#ifndef EDITOR_GIZMO_INFO_HPP
#define EDITOR_GIZMO_INFO_HPP


#include <GXCommon/GXMath.hpp>


namespace editor {

struct GizmoInfo final
{
    GXMat4          _local {};
    GXAABB          _bounds {};
    GXColorUNORM    _color { 255U, 255U, 255U, 255U };
};

} // namespace editor


#endif // EDITOR_GIZMO_INFO_HPP
