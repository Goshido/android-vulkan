#include <contact_manager.h>


namespace android_vulkan {

constexpr static size_t const INITIAL_CONTACT_MANIFOLDS = 1024U;
constexpr static size_t const INITIAL_CONTACTS = INITIAL_CONTACT_MANIFOLDS * 4U;
constexpr static size_t const INITIAL_INDICES = 32U;

constexpr static float const WARM_FINDER_TOLERANCE = 2.0e-1F;
constexpr static float const WARM_START_FACTOR = 1.0F;

static_assert ( WARM_START_FACTOR >= 0.0F && WARM_START_FACTOR <= 1.0F,
    "The warm start factor must be in range [0.0F, 1.0F]" );

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
    return _sets.front().first.emplace_back ();
}

std::vector<ContactManifold>& ContactManager::GetContactManifolds () noexcept
{
    return _sets.front().first;
}

std::vector<ContactManifold> const& ContactManager::GetContactManifolds () const noexcept
{
    return _sets.front().first;
}

void ContactManager::Reset () noexcept
{
    SwapSets ();
    UpdateWarmCache ();
    ResetFrontSet ();
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

void ContactManager::UpdateWarmCache () noexcept
{
    auto& frontContactManifolds = _sets.front ().first;

    if ( frontContactManifolds.empty () )
    {
        _warmStartMapper.clear ();
        return;
    }

    auto const timePoint = std::chrono::steady_clock::now ();

    uint64_t const now = static_cast<uint64_t> (
        std::chrono::duration_cast<std::chrono::nanoseconds> ( timePoint.time_since_epoch () ).count ()
    );

    // Updating|appending existing elements.
    constexpr float const tolerance = WARM_FINDER_TOLERANCE * WARM_FINDER_TOLERANCE;
    auto const warmMapperEnd = _warmStartMapper.end ();

    for ( auto& manifold : frontContactManifolds )
    {
        manifold._timeStamp = now;

        Key const key = std::make_pair ( manifold._bodyA.get (), manifold._bodyB.get () );
        auto findResult = _warmStartMapper.find ( key );

        if ( findResult == warmMapperEnd )
        {
            _warmStartMapper.emplace ( std::make_pair ( key, &manifold ) );
            continue;
        }

        ContactManifold const& cache = *findResult->second;
        _indices.clear ();

        for ( size_t i = 0U; i < cache._contactCount; ++i )
            _indices.push_back ( i );

        Contact const* cacheContacts = cache._contacts;
        Contact* contacts = manifold._contacts;

        for ( size_t i = 0U; i < manifold._contactCount && !_indices.empty (); ++i )
        {
            Contact& contact = contacts[ i ];

            for ( auto ind = _indices.begin (); ind != _indices.end (); )
            {
                Contact const& cacheContact = cacheContacts[ *ind ];

                if ( contact._pointA.SquaredDistance ( cacheContact._pointA ) > tolerance )
                {
                    ++ind;
                    continue;
                }

                if ( contact._pointB.SquaredDistance ( cacheContact._pointB ) > tolerance )
                {
                    ++ind;
                    continue;
                }

                GXVec3 finalLambda {};

                finalLambda.Multiply (
                    GXVec3 ( cacheContact._dataT._lambda, cacheContact._dataB._lambda, cacheContact._dataN._lambda ),
                    WARM_START_FACTOR
                );

                contact._dataT._lambda = finalLambda._data[ 0U ];
                contact._dataB._lambda = finalLambda._data[ 1U ];
                contact._dataN._lambda = finalLambda._data[ 2U ];

                *ind = _indices.back ();
                _indices.pop_back ();
            }
        }

        findResult->second = &manifold;
    }

    // Removing outdated elements.

    for ( auto i = _warmStartMapper.begin (); i != warmMapperEnd; )
    {
        ContactManifold const& manifold = *i->second;

        if ( manifold._timeStamp == now )
        {
            ++i;
            continue;
        }

        i = _warmStartMapper.erase ( i );
    }
}

} // namespace android_vulkan
