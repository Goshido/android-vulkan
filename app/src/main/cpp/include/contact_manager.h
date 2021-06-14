#ifndef ANDROID_VULKAN_CONTACT_MANAGER_H
#define ANDROID_VULKAN_CONTACT_MANAGER_H


#include "rigid_body.h"

GX_DISABLE_COMMON_WARNINGS

#include <vector>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

struct Contact final
{
    [[maybe_unused]] GXVec3     _point {};

    [[maybe_unused]] GXVec3     _tangent {};
    [[maybe_unused]] GXVec3     _birangent {};
    [[maybe_unused]] GXVec3     _normal {};

    [[maybe_unused]] float      _penetration = 0.0F;

    Contact () = default;

    Contact ( Contact const & ) = default;
    Contact& operator = ( Contact const & ) = default;

    Contact ( Contact && ) = default;
    Contact& operator = ( Contact && ) = default;

    ~Contact () = default;
};

struct ContactManifold final
{
    [[maybe_unused]] RigidBodyRef       _bodyA {};
    [[maybe_unused]] RigidBodyRef       _bodyB {};

    size_t                              _contactCount = 0U;
    Contact*                            _contacts = nullptr;

    [[maybe_unused]] uint16_t           _gjkSteps = 0U;
    [[maybe_unused]] uint16_t           _epsSteps = 0U;

    [[maybe_unused]] uint16_t           _edges = 0U;
    [[maybe_unused]] uint16_t           _faces = 0U;
    [[maybe_unused]] uint16_t           _supportPoints = 0U;

    ContactManifold () = default;

    ContactManifold ( ContactManifold const & ) = default;
    ContactManifold& operator = ( ContactManifold const & ) = default;

    ContactManifold ( ContactManifold && ) = default;
    ContactManifold& operator = ( ContactManifold && ) = default;

    ~ContactManifold () = default;
};

class [[maybe_unused]] ContactManager final
{
    private:
        std::vector<Contact>            _contacts;
        std::vector<ContactManifold>    _contactManifolds;

    public:
        ContactManager () noexcept;

        ContactManager ( ContactManager const & ) = delete;
        ContactManager& operator = ( ContactManager const & ) = delete;

        ContactManager ( ContactManager && ) = delete;
        ContactManager& operator = ( ContactManager && ) = delete;

        ~ContactManager () = default;

        [[maybe_unused, nodiscard]] Contact& AllocateContact ( ContactManifold &contactManifold ) noexcept;
        [[maybe_unused, nodiscard]] ContactManifold& AllocateContactManifold () noexcept;
        void Reset () noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_CONTACT_MANAGER_H
