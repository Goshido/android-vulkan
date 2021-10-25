#ifndef ANDROID_VULKAN_CONTACT_MANAGER_H
#define ANDROID_VULKAN_CONTACT_MANAGER_H


#include "rigid_body.h"

GX_DISABLE_COMMON_WARNINGS

#include <array>
#include <vector>
#include <unordered_map>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

using LambdaClipHandler = float ( * ) ( float lambda, void* context ) noexcept;

struct VelocitySolverData final
{
    float                   _b = 0.0F;

    LambdaClipHandler       _clipHandler = nullptr;
    void*                   _clipContext = nullptr;

    float                   _effectiveMass = 0.0F;
    GXVec6                  _j[ 2U ] {};
    GXVec6                  _mj[ 2U ] {};
    float                   _lambda = 0.0F;
};

struct Contact final
{
    GXVec3                  _pointA {};
    GXVec3                  _pointB {};
    float                   _penetration = 0.0F;

    // Velocity solver entities.
    VelocitySolverData      _dataT {};
    VelocitySolverData      _dataB {};
    VelocitySolverData      _dataN {};

    float                   _friction = 0.5F;
    float                   _restitution = 0.5F;

    // The normal points the direction which body B must be moved to eliminate collision.
    GXVec3                  _tangent {};
    GXVec3                  _bitangent {};
    GXVec3                  _normal {};

    // Debug feature.
    bool                    _warmStarted = false;
};

struct ContactManifold final
{
    RigidBodyRef                    _bodyA {};
    RigidBodyRef                    _bodyB {};

    size_t                          _contactCount = 0U;
    Contact*                        _contacts = nullptr;

    bool                            _warmStartIsUsed = true;

    [[maybe_unused]] uint16_t       _epaSteps = 0U;
    [[maybe_unused]] uint16_t       _gjkSteps = 0U;
};

class ContactManager final
{
    private:
        using Key = std::pair<RigidBody*, RigidBody*>;
        using Set = std::pair<std::vector<ContactManifold>, std::vector<Contact>>;

        class Hasher final
        {
            public:
                Hasher () = default;

                Hasher ( Hasher const & ) = default;
                Hasher &operator = ( Hasher const & ) = default;

                Hasher ( Hasher && ) = default;
                Hasher &operator = ( Hasher && ) = default;

                ~Hasher() = default;

                // hash function. std::unordered_map requirement.
                [[nodiscard]] size_t operator () ( Key const &me ) const noexcept;

            private:
                std::hash<void const*>      _hashServer {};
        };

        using Mapper = std::unordered_map<Key, ContactManifold*, Hasher>;

    private:
        std::array<Set, 2U>                 _sets {};
        Mapper                              _warmStartMapper {};
        std::vector<size_t>                 _indices;

    public:
        ContactManager () noexcept;

        ContactManager ( ContactManager const & ) = delete;
        ContactManager& operator = ( ContactManager const & ) = delete;

        ContactManager ( ContactManager && ) = delete;
        ContactManager& operator = ( ContactManager && ) = delete;

        ~ContactManager () = default;

        [[nodiscard]] Contact& AllocateContact ( ContactManifold &contactManifold ) noexcept;
        [[nodiscard]] ContactManifold& AllocateContactManifold () noexcept;

        [[nodiscard]] std::vector<ContactManifold>& GetContactManifolds () noexcept;
        [[nodiscard]] std::vector<ContactManifold> const& GetContactManifolds () const noexcept;

        void Reset () noexcept;
        void Warm ( ContactManifold &manifold ) noexcept;

    private:
        void RemoveOutdatedWarmRecords () noexcept;
        void ResetFrontSet () noexcept;
        void SimplifyManifold ( ContactManifold &manifold ) noexcept;
        void SwapSets () noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_CONTACT_MANAGER_H
