#include <location_solver.h>
#include <logger.h>


namespace android_vulkan {

void LocationSolver::Run ( ContactManager &contactManager ) noexcept
{
    for ( auto& manifold : contactManager.GetContactManifolds () )
    {
        RigidBody& bodyA = *manifold._bodyA;
        RigidBody& bodyB = *manifold._bodyB;

        if ( bodyB.IsKinematic () )
        {
            SolveSingle ( bodyA, manifold, -manifold._penetration );
            continue;
        }

        SolveSingle ( bodyB, manifold, manifold._penetration );
    }
}

void LocationSolver::SolveSingle ( RigidBody &body, ContactManifold &manifold, float penetration ) noexcept
{
    GXVec3 offset {};
    offset.Multiply ( manifold._normal, penetration );

    if ( &body == manifold._bodyA.get () )
    {
        for ( size_t i = 0U; i < manifold._contactCount; ++i )
        {
            Contact& c = manifold._contacts[ i ];
            c._pointAfterResolve.Sum ( c._pointA, offset );
        }

        return;
    }

    for ( size_t i = 0U; i < manifold._contactCount; ++i )
    {
        Contact& c = manifold._contacts[ i ];
        c._pointAfterResolve.Sum ( c._pointB, offset );
    }
}

} // namespace android_vulkan
