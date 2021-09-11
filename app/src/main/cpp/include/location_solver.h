#ifndef ANDROID_VULKAN_LOCATION_SOLVER_H
#define ANDROID_VULKAN_LOCATION_SOLVER_H


#include "contact_manager.h"


namespace android_vulkan {

class [[maybe_unused]] LocationSolver final
{
    public:
        LocationSolver () = delete;

        LocationSolver ( LocationSolver const & ) = delete;
        LocationSolver& operator = ( LocationSolver const & ) = delete;

        LocationSolver ( LocationSolver && ) = delete;
        LocationSolver& operator = ( LocationSolver && ) = delete;

        ~LocationSolver () = delete;

        static void Run ( ContactManager &contactManager ) noexcept;

    private:
        static void SolveSingle ( RigidBody &body, ContactManifold &manifold, bool negatePenetration ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_LOCATION_SOLVER_H
