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

        if ( bodyA.IsKinematic () )
        {
            SolveSingle ( bodyB, manifold, manifold._penetration, manifold._normal );
            continue;
        }

        if ( bodyB.IsKinematic () )
        {
            SolveSingle ( bodyA, manifold, -manifold._penetration, manifold._normal );
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
        c._pointAfterResolve.Sum ( c._pointA, offset );
    }
}

} // namespace android_vulkan
