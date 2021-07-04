#ifndef ANDROID_VULKAN_VELOCITY_SOLVER_H
#define ANDROID_VULKAN_VELOCITY_SOLVER_H


#include "contact_manager.h"


namespace android_vulkan {

// The implementation is based on ideas from
// https://www.youtube.com/watch?v=MTVdBgQY9LY
// https://www.youtube.com/watch?v=pmdYzNF9x34
// http://allenchou.net/2013/12/game-physics-constraints-sequential-impulse/
// http://allenchou.net/2013/12/game-physics-resolution-contact-constraints/
class VelocitySolver final
{
    public:
        VelocitySolver () = delete;

        VelocitySolver ( VelocitySolver const & ) = delete;
        VelocitySolver& operator = ( VelocitySolver const & ) = delete;

        VelocitySolver ( VelocitySolver && ) = delete;
        VelocitySolver& operator = ( VelocitySolver && ) = delete;

        ~VelocitySolver () = delete;

        static void Run ( ContactManager &contactManager, float fixedTimeStep ) noexcept;

    private:
        // Basically it's a solver for dynamic vs dynamic body.
        static void SolvePair ( ContactManifold &manifold ) noexcept;

        // Basically it's a solver for dynamic vs kinematic body.
        // Note the body A in the manifold data structure is dynamic while body B is kinematic.
        static void SolveSingle ( ContactManifold &manifold, float fixedTimeStep ) noexcept;

        static void SwapBodies ( ContactManifold &manifold ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_VELOCITY_SOLVER_H
