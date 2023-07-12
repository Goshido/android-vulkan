#ifndef ANDROID_VULKAN_PHYSICS_HPP
#define ANDROID_VULKAN_PHYSICS_HPP


#include "contact_detector.hpp"
#include "contact_manager.hpp"
#include "global_force.hpp"
#include "ray_caster.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <mutex>
#include <unordered_set>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

struct Penetration final
{
    RigidBodyRef    _body;
    float           _depth;

    // The direction in world space to move testing shape to resolve penetration.
    GXVec3          _normal;
};

class Physics final
{
    private:
        float                                   _accumulator = 0.0F;
        ContactDetector                         _contactDetector {};
        ContactManager                          _contactManager {};
        std::unordered_set<RigidBodyRef>        _dynamics {};
        float                                   _fixedTimeStep;
        float                                   _fixedTimeStepInverse;
        std::unordered_set<GlobalForceRef>      _globalForces {};
        bool                                    _isPause = true;
        std::unordered_set<RigidBodyRef>        _kinematics {};
        std::mutex                              _mutex {};
        float                                   _timeSpeed;

        bool                                    _debugRun = false;

    public:
        Physics () noexcept;

        Physics ( Physics const & ) = delete;
        Physics &operator = ( Physics const & ) = delete;

        Physics ( Physics && ) = delete;
        Physics &operator = ( Physics && ) = delete;

        ~Physics () = default;

        [[maybe_unused, nodiscard]] bool AddGlobalForce ( GlobalForceRef const &globalForce ) noexcept;
        [[maybe_unused, nodiscard]] bool RemoveGlobalForce ( GlobalForceRef const &globalForce ) noexcept;

        [[maybe_unused, nodiscard]] bool AddRigidBody ( RigidBodyRef const &rigidBody ) noexcept;
        [[maybe_unused, nodiscard]] bool RemoveRigidBody ( RigidBodyRef const &rigidBody ) noexcept;

        [[nodiscard]] std::vector<ContactManifold> const &GetContactManifolds () const noexcept;

        [[maybe_unused, nodiscard]] float GetTimeSpeed () const noexcept;
        [[maybe_unused]] void SetTimeSpeed ( float speed ) noexcept;

        [[nodiscard]] bool IsPaused () const noexcept;
        void OnIntegrationTypeChanged ( RigidBody &rigidBody ) noexcept;
        void Pause () noexcept;

        // "groups" will be used as filter during the test.
        void PenetrationTest ( std::vector<Penetration> &result,
            EPA &epa,
            ShapeRef const &shape,
            uint32_t groups
        ) const noexcept;

        // The method returns true if ray hits anything. Otherwise the method returns false.
        // "groups" will be used as filter during the ray casting.
        // The closest hit point will be returned.
        [[nodiscard]] bool Raycast ( RaycastResult &result,
            uint32_t groups,
            GXVec3 const &from,
            GXVec3 const &to
        ) const noexcept;

        void Resume () noexcept;
        void Reset () noexcept;
        void Simulate ( float deltaTime ) noexcept;

        // "result" vector will be resized if needed.
        // "groups" will be used as filter during the test.
        void SweepTest ( std::vector<RigidBodyRef> &result, ShapeRef const &sweepShape, uint32_t groups ) noexcept;

        void OnDebugRun () noexcept;

    private:
        void CollectContacts () noexcept;
        void Integrate () noexcept;
        void ResolveIntegrationType ( RigidBody &rigidBody ) noexcept;
};

} // namespace android_vulkan


#endif //  ANDROID_VULKAN_PHYSICS_HPP
