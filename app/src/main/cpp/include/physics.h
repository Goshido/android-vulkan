#ifndef ANDROID_VULKAN_PHYSICS_H
#define ANDROID_VULKAN_PHYSICS_H


#include "contact_manager.h"

GX_DISABLE_COMMON_WARNINGS

#include <unordered_set>

GX_RESTORE_WARNING_STATE

#include "global_force.h"


namespace android_vulkan {

class Physics final
{
    private:
        float                                   _accumulator;
        ContactManager                          _contactManager;
        std::unordered_set<GlobalForceRef>      _globalForces;
        bool                                    _isPause;
        std::unordered_set<RigidBodyRef>        _rigidBodies;

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
