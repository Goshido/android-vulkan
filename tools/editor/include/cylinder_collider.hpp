#ifndef EDITOR_CYLINDER_COLLIDER_HPP
#define EDITOR_CYLINDER_COLLIDER_HPP


#include <GXCommon/GXMath.hpp>


namespace editor {

class CylinderCollider final
{
    private:
        float       _w = 1.0F;
        float       _u = 1.0F;

    public:
        CylinderCollider () = delete;

        CylinderCollider ( CylinderCollider const & ) = delete;
        CylinderCollider &operator = ( CylinderCollider const & ) = delete;

        CylinderCollider ( CylinderCollider && ) = default;
        CylinderCollider &operator = ( CylinderCollider && ) = default;

        explicit CylinderCollider ( float radius, float length ) noexcept;

        ~CylinderCollider () = default;

        // Note 'cylinderCapLocation' is intentionally sent by value.
        [[nodiscard]] float Raycast ( GXVec3 const &rayOrigin,
            GXVec3 const &rayDirection,
            GXVec3 cylinderCapLocation,
            GXVec3 const &cylinderAxis
        ) const noexcept;
};

} // namespace editor


#endif // EDITOR_CYLINDER_COLLIDER_HPP
