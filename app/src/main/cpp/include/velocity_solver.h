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
class VelocitySolver final
{
    private:
        float       _stabilizationFactor = 0.0F;

    public:
        VelocitySolver () = default;

        VelocitySolver ( VelocitySolver const & ) = delete;
        VelocitySolver& operator = ( VelocitySolver const & ) = delete;

        VelocitySolver ( VelocitySolver && ) = delete;
        VelocitySolver& operator = ( VelocitySolver && ) = delete;

        ~VelocitySolver () = default;

        void Run ( ContactManager &contactManager, float fixedTimeStepInverse ) noexcept;

    private:
        [[nodiscard]] float ComputeBaumgarteTerm ( GXVec3 const &wA,
            GXVec3 const &wB,
            GXVec3 const &cmA,
            GXVec3 const &cmB,
            GXVec3 const &dV,
            Contact const &contact
        ) const noexcept;

        // Basically it's a solver for dynamic vs dynamic body.
        void SolvePair ( ContactManifold &manifold ) noexcept;

        // Basically it's a solver for dynamic vs kinematic body.
        // Note the body A in the manifold data structure is dynamic while body B is kinematic.
        void SolveSingle ( ContactManifold &manifold ) noexcept;

        [[maybe_unused]] static void DebugContactInManifold ( ContactManager const &contactManager ) noexcept;
        [[maybe_unused]] static void DebugWarmStart ( ContactManager const &contactManager ) noexcept;

        [[nodiscard]] static float LambdaClipNormalForce ( float lambda, void* context ) noexcept;
        [[nodiscard]] static float LambdaClipFriction ( float lambda, void* context ) noexcept;

        static void PreparePair ( ContactManifold &manifold ) noexcept;
        static void PrepareSingle ( ContactManifold &manifold ) noexcept;

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
