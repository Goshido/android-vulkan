#ifndef ANDROID_VULKAN_VELOCITY_SOLVER_H
#define ANDROID_VULKAN_VELOCITY_SOLVER_H


#include "contact_manager.h"


namespace android_vulkan {

// The implementation is based on ideas from
// https://www.youtube.com/watch?v=MTVdBgQY9LY
// https://www.youtube.com/watch?v=pmdYzNF9x34
// http://allenchou.net/2013/12/game-physics-constraints-sequential-impulse/
// http://allenchou.net/2013/12/game-physics-resolution-contact-constraints/
// http://allenchou.net/2014/01/game-physics-stability-slops/
//
// Bullet engine ver. 3.20 (commit: 48dc1c45da685c77d3642545c4851b05fb3a1e8b):
// src/BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.cpp
class VelocitySolver final
{
    public:
        VelocitySolver () = default;

        VelocitySolver ( VelocitySolver const & ) = delete;
        VelocitySolver& operator = ( VelocitySolver const & ) = delete;

        VelocitySolver ( VelocitySolver && ) = delete;
        VelocitySolver& operator = ( VelocitySolver && ) = delete;

        ~VelocitySolver () = default;

        static void Run ( ContactManager &contactManager, float fixedTimeStepInverse ) noexcept;

    private:
        [[nodiscard]] static float ComputeBaumgarteTerm ( GXVec3 const &wA,
            GXVec3 const &wB,
            GXVec3 const &cmA,
            GXVec3 const &cmB,
            GXVec3 const &dV,
            Contact const &contact,
            float stabilizationFactor
        ) noexcept;

        [[maybe_unused]] static void DebugContactInManifold ( ContactManager const &contactManager ) noexcept;
        [[maybe_unused]] static void DebugWarmStart ( ContactManager const &contactManager ) noexcept;

        static void PreparePair ( ContactManifold &manifold ) noexcept;
        static void PrepareSingle ( ContactManifold &manifold ) noexcept;

        // Basically it's a solver for dynamic vs dynamic body.
        static void SolvePairFriction ( ContactManifold &manifold ) noexcept;
        static void SolvePairNormal ( ContactManifold &manifold, float stabilizationFactor ) noexcept;

        // Basically it's a solver for dynamic vs kinematic body.
        // Note the body A in the manifold data structure is dynamic while body B is kinematic.
        static void SolveSingleFriction ( ContactManifold &manifold ) noexcept;
        static void SolveSingleNormal ( ContactManifold &manifold, float stabilizationFactor ) noexcept;

        static void SwapBodies ( ContactManifold &manifold ) noexcept;

        static void UpdateVelocityPair ( RigidBody &bodyA,
            RigidBody &bodyB,
            VelocitySolverData const &data,
            GXVec6 const &vw1A,
            GXVec6 const &vw1B,
            float lambda
        ) noexcept;

        static void UpdateVelocitySingle ( RigidBody &body,
            VelocitySolverData const &data,
            GXVec6 const &vw1,
            float lambda
        ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_VELOCITY_SOLVER_H
