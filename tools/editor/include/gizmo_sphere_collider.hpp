#ifndef EDITOR_GIZMO_SPHERE_COLLIDER_HPP
#define EDITOR_GIZMO_SPHERE_COLLIDER_HPP


#include <GXCommon/GXMath.hpp>


namespace editor {

// Class is optimized for gizmo interaction needs.
// Idea from https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection.html
class GizmoSphereCollider final
{
    private:
        float       _radius = 1.0F;

    public:
        GizmoSphereCollider () = delete;

        GizmoSphereCollider ( GizmoSphereCollider const & ) = delete;
        GizmoSphereCollider &operator = ( GizmoSphereCollider const & ) = delete;

        GizmoSphereCollider ( GizmoSphereCollider && ) = default;
        GizmoSphereCollider &operator = ( GizmoSphereCollider && ) = default;

        explicit GizmoSphereCollider ( float radius ) noexcept;

        ~GizmoSphereCollider () = default;

        [[nodiscard]] float Raycast ( GXVec3 const &rayOrigin,
            GXVec3 const &rayDirection,
            GXVec3 const &sphereLocation,
            GXVec3 const &cameraLocation,
            GXVec3 const &vi
        ) const noexcept;
};

} // namespace editor


#endif // EDITOR_GIZMO_SPHERE_COLLIDER_HPP
