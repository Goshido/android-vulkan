#ifndef ANDROID_VULKAN_RAY_CASTER_HPP
#define ANDROID_VULKAN_RAY_CASTER_HPP


#include "gjk_base.hpp"
#include "shape.hpp"
#include "contact_detector.hpp"


namespace android_vulkan {

struct RaycastResult final
{
    RigidBodyRef    _body {};
    GXVec3          _point {};
    GXVec3          _normal {};
};

// The implementation is based on ideas from
// Ray Casting against General Convex Objects with Application to Continuous Collision Detection
// by Gino van den Bergen
//
// Bullet v3.20 (SHA-1: ac3c283842fe5ceddc55fcdef5489fdf4458f6f6)
// <repo>/src/BulletCollision/NarrowPhaseCollision/btSubSimplexConvexCast.cpp
class RayCaster final : public GJKBase
{
    public:
        RayCaster () = default;

        RayCaster ( RayCaster const & ) = delete;
        RayCaster &operator = ( RayCaster const & ) = delete;

        RayCaster ( RayCaster && ) = delete;
        RayCaster &operator = ( RayCaster && ) = delete;

        ~RayCaster () override = default;

        // The method returns true if ray hits anything. Otherwise the method returns false.
        [[nodiscard]] bool Run ( RaycastResult &result,
            GXVec3 const &from,
            GXVec3 const &to,
            Shape const &shape
        ) noexcept;

    private:
        [[nodiscard]] GXVec3 TestLine () noexcept;
        [[nodiscard]] GXVec3 TestTetrahedron ( GXVec3 const &bcdClosest, float bcdClosestDistance ) noexcept;
        [[nodiscard]] GXVec3 TestTriangle () noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_RAY_CASTER_HPP
