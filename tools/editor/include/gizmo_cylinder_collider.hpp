#ifndef EDITOR_GIZMO_CYLINDER_COLLIDER_HPP
#define EDITOR_GIZMO_CYLINDER_COLLIDER_HPP


#include <GXCommon/GXMath.hpp>


namespace editor {

// Class is optimized for gizmo interaction needs.
class GizmoCylinderCollider final
{
    private:
        float       _w = 1.0F;
        float       _u = 1.0F;

    public:
        GizmoCylinderCollider () = delete;

        GizmoCylinderCollider ( GizmoCylinderCollider const & ) = delete;
        GizmoCylinderCollider &operator = ( GizmoCylinderCollider const & ) = delete;

        GizmoCylinderCollider ( GizmoCylinderCollider && ) = default;
        GizmoCylinderCollider &operator = ( GizmoCylinderCollider && ) = default;

        explicit GizmoCylinderCollider ( float radius, float length ) noexcept;

        ~GizmoCylinderCollider () = default;

        // Note
        // 'cylinderCapLocation' is intentionally sent by value.
        // The intersection will return miss in case ray direction is parallel to cylinder axis. This in intentional
        // for gizmo operate. For example such scenario for move tool would teleport object to +/- infinity.
        // It's needed to avoid.
        [[nodiscard]] float Raycast ( GXVec3 const &rayOrigin,
            GXVec3 const &rayDirection,
            GXVec3 cylinderCapLocation,
            GXVec3 const &cylinderAxis
        ) const noexcept;
};

} // namespace editor


#endif // EDITOR_GIZMO_CYLINDER_COLLIDER_HPP
