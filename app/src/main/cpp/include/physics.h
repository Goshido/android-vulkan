#ifndef ANDROID_VULKAN_PHYSICS_H
#define ANDROID_VULKAN_PHYSICS_H


#include "contact_detector.h"
#include "contact_manager.h"
#include "global_force.h"

GX_DISABLE_COMMON_WARNINGS

#include <unordered_set>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

class Physics final
{
    private:
        float                                   _accumulator;
        ContactDetector                         _contactDetector;
        ContactManager                          _contactManager;
        float                                   _fixedTimeStep;
        std::unordered_set<GlobalForceRef>      _globalForces;
        bool                                    _isPause;
        std::unordered_set<RigidBodyRef>        _rigidBodies;
        float                                   _timeSpeed;

    public:
        Physics () noexcept;

        Physics ( Physics const & ) = delete;
        Physics& operator = ( Physics const & ) = delete;

        Physics ( Physics && ) = delete;
        Physics& operator = ( Physics && ) = delete;

        ~Physics () = default;

        [[maybe_unused, nodiscard]] bool AddGlobalForce ( GlobalForceRef const &globalForce ) noexcept;
        [[maybe_unused, nodiscard]] bool RemoveGlobalForce ( GlobalForceRef const &globalForce ) noexcept;

        [[maybe_unused, nodiscard]] bool AddRigidBody ( RigidBodyRef const &rigidBody ) noexcept;
        [[maybe_unused, nodiscard]] bool RemoveRigidBody ( RigidBodyRef const &rigidBody ) noexcept;

        [[nodiscard]] std::vector<ContactManifold> const& GetContactManifolds () const noexcept;

        [[maybe_unused, nodiscard]] float GetTimeSpeed () const noexcept;
        [[maybe_unused]] void SetTimeSpeed ( float speed ) noexcept;

        [[nodiscard]] bool IsPaused () const noexcept;
        void Pause () noexcept;
        void Resume () noexcept;
        void Simulate ( float deltaTime ) noexcept;

    private:
        void CollectContacts () noexcept;
        void Integrate () noexcept;
        void Prepare () noexcept;
};

} // namespace android_vulkan


#endif //  ANDROID_VULKAN_PHYSICS_H
