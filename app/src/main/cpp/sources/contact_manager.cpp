#include <contact_manager.h>

GX_DISABLE_COMMON_WARNINGS

#include <numeric>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

constexpr static size_t const INITIAL_CONTACT_MANIFOLDS = 1024U;
constexpr static size_t const INITIAL_CONTACTS = INITIAL_CONTACT_MANIFOLDS * 4U;
constexpr static size_t const INITIAL_INDICES = 32U;

constexpr static float const WARM_FINDER_TOLERANCE = 4.0e-3F;
constexpr static float const WARM_FINDER_FACTOR = WARM_FINDER_TOLERANCE * WARM_FINDER_TOLERANCE;

constexpr static float const WARM_TRANSFER_FACTOR = 0.7F;

//----------------------------------------------------------------------------------------------------------------------

size_t ContactManager::Hasher::operator () ( Key const &me ) const noexcept
{
    // Hash function is based on Boost implementation:
    // https://www.boost.org/doc/libs/1_55_0/doc/html/hash/reference.html#boost.hash_combine

    size_t hash = 0U;

    auto hashCombine = [ & ] ( RigidBody const* body ) noexcept
    {
        constexpr size_t const magic = 0x9e3779b9U;
        hash ^= _hashServer ( body ) + magic + ( hash << 6U ) + ( hash >> 2U );
    };

    auto const [a, b] = me;
    hashCombine ( a );
    hashCombine ( b );

    return hash;
}

//----------------------------------------------------------------------------------------------------------------------

ContactManager::ContactManager () noexcept:
    _sets {},
    _warmStartMapper {}
{
    for ( auto& [contactManifolds, contacts] : _sets )
    {
        contacts.reserve ( INITIAL_CONTACTS );
        contactManifolds.reserve ( INITIAL_CONTACT_MANIFOLDS );
    }

    _indices.reserve ( INITIAL_INDICES );
}

Contact& ContactManager::AllocateContact ( ContactManifold &contactManifold ) noexcept
{
    Contact& contact = _sets.front().second.emplace_back ();

    if ( !contactManifold._contacts )
        contactManifold._contacts = &contact;

    ++contactManifold._contactCount;
    return contact;
}

ContactManifold& ContactManager::AllocateContactManifold () noexcept
{
    return _sets.front ().first.emplace_back ();
}

std::vector<ContactManifold>& ContactManager::GetContactManifolds () noexcept
{
    return _sets.front ().first;
}

std::vector<ContactManifold> const& ContactManager::GetContactManifolds () const noexcept
{
    return _sets.front ().first;
}

void ContactManager::RemoveOutdatedWarmRecords () noexcept
{
    auto const end = _warmStartMapper.end ();

    for ( auto i = _warmStartMapper.begin (); i != end; )
    {
        ContactManifold& manifold = *i->second;

        if ( !manifold._warmStartIsUsed )
        {
            i = _warmStartMapper.erase ( i );
            continue;
        }

        manifold._warmStartIsUsed = false;
        ++i;
    }
}

void ContactManager::Reset () noexcept
{
    SwapSets ();
    ResetFrontSet ();
    RemoveOutdatedWarmRecords ();
}

void ContactManager::Warm ( ContactManifold &manifold ) noexcept
{
    Key const key = std::make_pair ( manifold._bodyA.get (), manifold._bodyB.get () );
    auto findResult = _warmStartMapper.find ( key );
    auto const end = _warmStartMapper.end ();

    if ( findResult == end )
    {
        findResult = _warmStartMapper.find ( std::make_pair ( key.second, key.first ) );

        if ( findResult == end )
        {
            _warmStartMapper.emplace ( key, &manifold );
            return;
        }
    }

    ContactManifold const& cache = *findResult->second;
    Contact const* cacheContacts = cache._contacts;

    _indices.clear ();
    _indices.resize ( cache._contactCount );
    std::iota ( _indices.begin (), _indices.end (), 0U );

    size_t const contactCount = manifold._contactCount;
    Contact* contacts = manifold._contacts;

    for ( size_t i = 0U; i < contactCount; ++i )
    {
        Contact& contact = contacts[ i ];

        for ( auto cacheIndex = _indices.begin (); cacheIndex != _indices.end (); )
        {
            size_t& ind = *cacheIndex;
            Contact const& cacheContact = cacheContacts[ ind ];

            // Note this is rough approximation. Main assumption that both contact point pairs on previous step and
            // current step are relatively close to each other but relative far away from another pairs in
            // the same manifold. So only one distance estimation is enough.
            if ( contact._pointA.SquaredDistance ( cacheContact._pointA ) > WARM_FINDER_FACTOR )
            {
                ++cacheIndex;
                continue;
            }

            GXVec3 lambda {};

            lambda.Multiply (
                GXVec3 ( cacheContact._dataT._lambda, cacheContact._dataB._lambda, cacheContact._dataN._lambda ),
                WARM_TRANSFER_FACTOR
            );

            contact._dataT._lambda = lambda._data[ 0U ];
            contact._dataB._lambda = lambda._data[ 1U ];
            contact._dataN._lambda = lambda._data[ 2U ];
            contact._warmStarted = true;

            ind = _indices.back ();
            _indices.pop_back ();
            break;
        }
    }

    findResult->second = &manifold;
}

void ContactManager::ResetFrontSet () noexcept
{
    auto& [contactManifolds, contacts] = _sets.front ();
    contactManifolds.clear ();
    contacts.clear ();
}

void ContactManager::SwapSets () noexcept
{
    auto& [frontContactManifolds, frontContacts] = _sets.front ();
    auto& [backContactManifolds, backContacts] = _sets.back ();

    backContactManifolds.swap ( frontContactManifolds );
    backContacts.swap ( frontContacts );
}

} // namespace android_vulkan
