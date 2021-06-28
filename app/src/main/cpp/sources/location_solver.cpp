#include <location_solver.h>
#include <logger.h>
#include <cassert>


namespace android_vulkan {

void LocationSolver::Solve ( ContactManager &contactManager ) noexcept
{
    for ( auto& manifold : contactManager.GetContactManifolds () )
    {
        RigidBody& bodyA = *manifold._bodyA;
        RigidBody& bodyB = *manifold._bodyB;
        Contact const& firstContact = manifold._contacts[ 0U ];

        if ( bodyA.IsKinematic () )
        {
            SolveSingle ( bodyB, manifold, firstContact._penetration, firstContact._normal );
            continue;
        }

        if ( bodyB.IsKinematic () )
        {
            SolveSingle ( bodyA, manifold, -firstContact._penetration, firstContact._normal );
            continue;
        }

        SolvePair ( bodyA, bodyB, manifold );
    }
}

void LocationSolver::SolvePair ( RigidBody &/*bodyA*/,
    RigidBody &/*bodyB*/,
    ContactManifold const &/*manifold*/
) noexcept
{
    assert ( false );
}

void LocationSolver::SolveSingle ( RigidBody &body,
    ContactManifold &manifold,
    float penetration,
    GXVec3 const &normal
) noexcept
{
    GXVec3 offset {};
    offset.Multiply ( normal, penetration );

    GXVec3 location {};
    location.Sum ( body.GetLocation (), offset);
    body.SetLocation ( location );

    for ( size_t i = 0U; i < manifold._contactCount; ++i )
    {
        Contact& c = manifold._contacts[ i ];
        c._pointAfterResolve.Sum ( c._point, offset );
    }
}

} // namespace android_vulkan
