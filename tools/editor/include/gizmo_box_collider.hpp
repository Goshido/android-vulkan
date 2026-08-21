#ifndef EDITOR_GIZMO_BOX_COLLIDER_HPP
#define EDITOR_GIZMO_BOX_COLLIDER_HPP


#include <GXCommon/GXMath.hpp>


namespace editor {

// Implementation is based on ideas from
// https://how-to-3d.twodee.org/interaction/ray-box-intersection.html
// https://people.csail.mit.edu/amy/papers/box-jgt.pdf
class GizmoBoxCollider final
{
    private:
        GXQuat      _toBox = GXQuat::IDENTITY;
        GXVec3      _size = GXVec3::ONE;

    public:
        GizmoBoxCollider () = delete;

        GizmoBoxCollider ( GizmoBoxCollider const & ) = delete;
        GizmoBoxCollider &operator = ( GizmoBoxCollider const & ) = delete;

        GizmoBoxCollider ( GizmoBoxCollider && ) = default;
        GizmoBoxCollider &operator = ( GizmoBoxCollider && ) = default;

        explicit GizmoBoxCollider ( GXQuat const &orientation, GXVec3 const &size ) noexcept;

        ~GizmoBoxCollider () = default;

        [[nodiscard]] float Raycast ( GXVec3 const &rayOrigin,
            GXVec3 const &rayDirection,
            GXVec3 const &boxCenter
        ) const noexcept;
};

} // namespace editor


#endif // EDITOR_GIZMO_BOX_COLLIDER_HPP
