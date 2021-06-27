#include <contact_manager.h>


namespace android_vulkan {

constexpr static size_t INITIAL_CONTACT_MANIFOLDS = 1024U;
constexpr static size_t INITIAL_CONTACTS = INITIAL_CONTACT_MANIFOLDS * 4U;

ContactManager::ContactManager () noexcept:
    _contacts {},
    _contactManifolds {}
{
    _contacts.reserve ( INITIAL_CONTACTS );
    _contactManifolds.reserve ( INITIAL_CONTACT_MANIFOLDS );
}

Contact& ContactManager::AllocateContact ( ContactManifold &contactManifold ) noexcept
{
    Contact& contact = _contacts.emplace_back ();

    if ( !contactManifold._contacts )
        contactManifold._contacts = &contact;

    ++contactManifold._contactCount;
    return contact;
}

ContactManifold& ContactManager::AllocateContactManifold () noexcept
{
    return _contactManifolds.emplace_back ();
}

std::vector<ContactManifold> const& ContactManager::GetContactManifolds () const noexcept
{
    return _contactManifolds;
}

void ContactManager::Reset () noexcept
{
    _contactManifolds.clear ();
    _contacts.clear ();
}

} // namespace android_vulkan
