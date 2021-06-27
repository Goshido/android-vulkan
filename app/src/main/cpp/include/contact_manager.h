#ifndef ANDROID_VULKAN_CONTACT_MANAGER_H
#define ANDROID_VULKAN_CONTACT_MANAGER_H


#include "rigid_body.h"

GX_DISABLE_COMMON_WARNINGS

#include <vector>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

struct Contact final
{
    GXVec3      _point {};

    GXVec3      _tangent {};
    GXVec3      _bitangent {};
    GXVec3      _normal {};

    float       _penetration = 0.0F;

    Contact () = default;

    Contact ( Contact const & ) = default;
    Contact& operator = ( Contact const & ) = default;

    Contact ( Contact && ) = default;
    Contact& operator = ( Contact && ) = default;

    ~Contact () = default;
};

struct ContactManifold final
{
    RigidBodyRef    _bodyA {};
    RigidBodyRef    _bodyB {};

    size_t          _contactCount = 0U;
    Contact*        _contacts = nullptr;

    uint16_t        _epaSteps = 0U;
    uint16_t        _gjkSteps = 0U;

    ContactManifold () = default;

    ContactManifold ( ContactManifold const & ) = default;
    ContactManifold& operator = ( ContactManifold const & ) = default;

    ContactManifold ( ContactManifold && ) = default;
    ContactManifold& operator = ( ContactManifold && ) = default;

    ~ContactManifold () = default;
};

class ContactManager final
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

        [[nodiscard]] Contact& AllocateContact ( ContactManifold &contactManifold ) noexcept;
        [[nodiscard]] ContactManifold& AllocateContactManifold () noexcept;

        [[nodiscard]] std::vector<ContactManifold> const& GetContactManifolds () const noexcept;
        void Reset () noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_CONTACT_MANAGER_H
