#ifndef ANDROID_VULKAN_MANIFOLD_SOLVER_X4_H
#define ANDROID_VULKAN_MANIFOLD_SOLVER_X4_H


#include "contact_manager.h"


namespace android_vulkan {

class [[maybe_unused]] ManifoldSolverX4 final
{
    public:
        ManifoldSolverX4 () = delete;

        ManifoldSolverX4 ( ManifoldSolverX4 const & ) = delete;
        ManifoldSolverX4& operator = ( ManifoldSolverX4 const & ) = delete;

        ManifoldSolverX4 ( ManifoldSolverX4 && ) = delete;
        ManifoldSolverX4& operator = ( ManifoldSolverX4 && ) = delete;

        ~ManifoldSolverX4 () = delete;

        [[maybe_unused]] static void PreparePair ( ContactManifold &manifold, float fixedTimeStepInverse ) noexcept;
        [[maybe_unused]] static void PrepareSingle ( ContactManifold &manifold, float fixedTimeStepInverse ) noexcept;

        [[maybe_unused]] static void SolvePair ( ContactManifold &manifold ) noexcept;
        [[maybe_unused]] static void SolveSingle ( ContactManifold &manifold ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_MANIFOLD_SOLVER_X4_H
