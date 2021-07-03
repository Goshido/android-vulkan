#ifndef ANDROID_VULKAN_LOCATION_SOLVER_H
#define ANDROID_VULKAN_LOCATION_SOLVER_H


#include "contact_manager.h"


namespace android_vulkan {

class LocationSolver final
{
    public:
        LocationSolver () = delete;

        LocationSolver ( LocationSolver const & ) = delete;
        LocationSolver& operator = ( LocationSolver const & ) = delete;

        LocationSolver ( LocationSolver && ) = delete;
        LocationSolver& operator = ( LocationSolver && ) = delete;

        ~LocationSolver () = delete;

        static void Solve ( ContactManager &contactManager ) noexcept;

    private:
        static void SolvePair ( RigidBody &bodyA, RigidBody &bodyB, ContactManifold const &manifold ) noexcept;
        static void SolveSingle ( RigidBody &body, ContactManifold &manifold, float penetration ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_LOCATION_SOLVER_H
