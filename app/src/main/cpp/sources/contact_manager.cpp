#include <precompiled_headers.hpp>
#include <contact_manager.hpp>


namespace android_vulkan {

namespace {

constexpr size_t INITIAL_CONTACT_MANIFOLDS = 1024U;
constexpr size_t INITIAL_CONTACTS = INITIAL_CONTACT_MANIFOLDS * 4U;
constexpr size_t INITIAL_INDICES = 32U;

constexpr float WARM_FINDER_TOLERANCE = 4.0e-3F;
constexpr float WARM_FINDER_FACTOR = WARM_FINDER_TOLERANCE * WARM_FINDER_TOLERANCE;

constexpr float WARM_TRANSFER_FACTOR = 0.85F;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

size_t ContactManager::Hasher::operator () ( Key const &me ) const noexcept
{
    // Hash function is based on Boost implementation:
    // https://www.boost.org/doc/libs/1_55_0/doc/html/hash/reference.html#boost.hash_combine

    size_t hash = 0U;

    auto hashCombine = [ & ] ( RigidBody const* body ) noexcept
    {
        constexpr size_t magic = 0x9E3779B9U;
        hash ^= _hashServer ( body ) + magic + ( hash << 6U ) + ( hash >> 2U );
    };

    auto const [a, b] = me;
    hashCombine ( a );
    hashCombine ( b );

    return hash;
}

//----------------------------------------------------------------------------------------------------------------------

class Grabber final
{
    public:
        GXVec3 const    _pointA;
        GXVec3 const    _pointB;
        GXVec3 const    _normal;

    private:
        float const     _penetration;
        GXVec3 const    _tangent;
        GXVec3 const    _bitangent;

    public:
        Grabber () = delete;

        Grabber ( Grabber const & ) = delete;
        Grabber &operator = ( Grabber const & ) = delete;

        Grabber ( Grabber && ) = delete;
        Grabber &operator = ( Grabber && ) = delete;

        explicit Grabber ( Contact const &contact ) noexcept:
            _pointA ( contact._pointA ),
            _pointB ( contact._pointB ),
            _normal ( contact._normal ),
            _penetration ( contact._penetration ),
            _tangent ( contact._tangent ),
            _bitangent ( contact._bitangent )
        {
            // NOTHING
        }

        ~Grabber() = default;

        void Apply ( Contact &target ) const noexcept
        {
            target._pointA = _pointA;
            target._pointB = _pointB;
            target._normal = _normal;
            target._penetration = _penetration;
            target._tangent = _tangent;
            target._bitangent = _bitangent;
        }
};

//----------------------------------------------------------------------------------------------------------------------

ContactManager::ContactManager () noexcept
{
    for ( auto &[contactManifolds, contacts] : _sets )
    {
        contacts.reserve ( INITIAL_CONTACTS );
        contactManifolds.reserve ( INITIAL_CONTACT_MANIFOLDS );
    }

    _indices.reserve ( INITIAL_INDICES );
}

Contact &ContactManager::AllocateContact ( ContactManifold &contactManifold ) noexcept
{
    Contact &contact = _sets.front ().second.emplace_back ();

    if ( !contactManifold._contacts )
        contactManifold._contacts = &contact;

    ++contactManifold._contactCount;
    return contact;
}

ContactManifold &ContactManager::AllocateContactManifold () noexcept
{
    return _sets.front ().first.emplace_back ();
}

std::vector<ContactManifold> &ContactManager::GetContactManifolds () noexcept
{
    return _sets.front ().first;
}

std::vector<ContactManifold> const &ContactManager::GetContactManifolds () const noexcept
{
    return _sets.front ().first;
}

void ContactManager::RemoveOutdatedWarmRecords () noexcept
{
    auto const end = _warmStartMapper.end ();

    for ( auto i = _warmStartMapper.begin (); i != end; )
    {
        ContactManifold &manifold = *i->second;

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
    if ( manifold._contactCount > 4U )
        SimplifyManifold ( manifold );

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

    ContactManifold const &cache = *findResult->second;
    Contact const* cacheContacts = cache._contacts;

    _indices.clear ();
    _indices.resize ( cache._contactCount );
    std::iota ( _indices.begin (), _indices.end (), 0U );

    size_t const contactCount = manifold._contactCount;
    Contact* contacts = manifold._contacts;

    for ( size_t i = 0U; i < contactCount; ++i )
    {
        Contact &contact = contacts[ i ];

        for ( auto cacheIndex = _indices.begin (); cacheIndex != _indices.end (); )
        {
            size_t &ind = *cacheIndex;
            Contact const &cacheContact = cacheContacts[ ind ];

            // Note this is rough approximation. Main assumption that both contact point pairs on previous step and
            // current step are relatively close to each other but relative far away from another pairs in
            // the same manifold. So only one distance estimation is enough.

            if ( contact._pointA.SquaredDistance ( cacheContact._pointA ) > WARM_FINDER_FACTOR )
            {
                ++cacheIndex;
                continue;
            }

            // Note: it seems that the solution is more stable without lambda reprojection to the new basis.

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
    auto &[contactManifolds, contacts] = _sets.front ();
    contactManifolds.clear ();
    contacts.clear ();
}

void ContactManager::SimplifyManifold ( ContactManifold &manifold ) noexcept
{
    // Based on ideas from "Contact Point Reduction"
    // http://media.steampowered.com/apps/valve/2015/DirkGregorius_Contacts.pdf

    // For example box vs box contact manifold could contain up to 8 contacts points. It's too excessive for stable
    // simulation. Most of the time 4 is enough. Basic idea is to select 4 contact points which will produce
    // the shape with maximum area possible.

    Contact* contacts = manifold._contacts;

    _indices.clear ();
    _indices.resize ( manifold._contactCount );
    std::iota ( _indices.begin (), _indices.end (), 0U );

    // Optimization: Selecting contact point with minimum support point in direction [1.0F, 0.0F, 0.0F].
    // It's global X axis direction. So it's be enough to make decision only by value of the x coordinate.
    auto search = _indices.begin ();

    for ( auto i = search + 1U; i != _indices.end (); ++i )
    {
        if ( contacts[ *i ]._pointA._data[ 0U ] < contacts[ *search ]._pointA._data[ 0U ] )
        {
            search = i;
        }
    }

    auto remove = [ & ] ( auto &itt ) noexcept {
        *itt = _indices.back ();
        _indices.pop_back ();
    };

    Grabber const contactA ( contacts[ *search ] );
    remove ( search );

    // Second point: the line must have maximum distance.

    GXVec3 const &contactAPoint = contactA._pointA;
    search = _indices.begin ();
    float distance = contacts[ *search ]._pointA.SquaredDistance ( contactAPoint );

    for ( auto i = search + 1U; i != _indices.end (); ++i )
    {
        float const d = contacts[ *search ]._pointA.SquaredDistance ( contactAPoint );

        if ( d <= distance )
            continue;

        distance = d;
        search = i;
    }

    Grabber const contactB ( contacts[ *search ] );
    remove ( search );

    // Third point: the triangle must have maximum area.

    GXVec3 const &contactBPoint = contactB._pointA;

    // Note you might guess why don't just use normal from the first contact point. But unfortunately we can't do that
    // reliably. We have to recalculate normal because it's unknown which in winding order contact points are.
    GXVec3 const &o = contacts[ 0U ]._pointA;

    GXVec3 ab {};
    ab.Subtract ( contacts[ 1U ]._pointA, o );

    GXVec3 ac {};
    ac.Subtract ( contacts[ 2U ]._pointA, o );

    GXVec3 normal {};
    normal.CrossProduct ( ab, ac );

    auto signedAreaFactor = [ & ] ( GXVec3 const &a, GXVec3 const &b, GXVec3 const &c ) noexcept -> float {
        GXVec3 ca {};
        ca.Subtract ( a, c );

        GXVec3 cb {};
        cb.Subtract ( b, c );

        GXVec3 cross {};
        cross.CrossProduct ( ca, cb );

        return cross.DotProduct ( normal );
    };

    search = _indices.begin ();
    float areaFactor = signedAreaFactor ( contactAPoint, contactBPoint, contacts[ *search ]._pointA );

    for ( auto i = search + 1U; i != _indices.end (); ++i )
    {
        float const area = signedAreaFactor ( contactAPoint, contactBPoint, contacts[ *i ]._pointA );

        if ( area <= areaFactor )
            continue;

        areaFactor = area;
        search = i;
    }

    Grabber const contactC ( contacts[ *search ] );
    remove ( search );

    // Forth point: Minimum signed area of remaining points with sides of the ABC triangle.

    GXVec3 const &contactCPoint = contactC._pointA;
    search = _indices.end ();
    auto const end = search;
    areaFactor = std::numeric_limits<float>::max ();

    auto checkSide = [ & ] ( GXVec3 const &a, GXVec3 const &b ) noexcept {
        for ( auto i = _indices.begin (); i != end; ++i )
        {
            float const area = signedAreaFactor ( a, b, contacts[ *i ]._pointA );

            if ( area > 0.0F || area >= areaFactor )
                continue;

            areaFactor = area;
            search = i;
        }
    };

    checkSide ( contactAPoint, contactBPoint );
    checkSide ( contactBPoint, contactCPoint );
    checkSide ( contactCPoint, contactAPoint );

    Grabber const contactD ( contacts[ *search ] );

    contactA.Apply ( contacts[ 0U ] );
    contactB.Apply ( contacts[ 1U ] );
    contactC.Apply ( contacts[ 2U ] );
    contactD.Apply ( contacts[ 3U ] );

    manifold._contactCount = 4U;
}

void ContactManager::SwapSets () noexcept
{
    auto &[frontContactManifolds, frontContacts] = _sets.front ();
    auto &[backContactManifolds, backContacts] = _sets.back ();

    backContactManifolds.swap ( frontContactManifolds );
    backContacts.swap ( frontContacts );
}

} // namespace android_vulkan
