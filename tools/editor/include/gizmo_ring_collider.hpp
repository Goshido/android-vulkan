#ifndef EDITOR_GIZMO_RING_COLLIDER_HPP
#define EDITOR_GIZMO_RING_COLLIDER_HPP


#include <GXCommon/GXMath.hpp>


namespace editor {

class GizmoRingCollider final
{
    private:
        float       _radius = 1.0F;
        float       _thickness = 1.0F;

    public:
        GizmoRingCollider () = delete;

        GizmoRingCollider ( GizmoRingCollider const & ) = delete;
        GizmoRingCollider &operator = ( GizmoRingCollider const & ) = delete;

        GizmoRingCollider ( GizmoRingCollider && ) = default;
        GizmoRingCollider &operator = ( GizmoRingCollider && ) = default;

        explicit GizmoRingCollider ( float radius, float thickness ) noexcept;

        ~GizmoRingCollider () = default;

        [[nodiscard]] float Raycast ( GXVec3 const &rayOrigin,
            GXVec3 const &rayDirection,
            GXVec3 const &ringLocation,
            GXQuat const &ringOrientation,
            GXVec3 const &cameraLocation,
            GXMat3 const &cameraBasis,
            GXVec3 const &vi,
            bool billboard
        ) const noexcept;

    private:
        // Note 'p' is intentionally sent by value.
        [[nodiscard]] static float SDF ( GXVec3 p, GXVec2 const &sinCosAngle, float radius, float thickness ) noexcept;
};

} // namespace editor


#endif // EDITOR_GIZMO_RING_COLLIDER_HPP
