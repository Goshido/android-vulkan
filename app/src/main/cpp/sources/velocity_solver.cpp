#include <velocity_solver.h>

GX_DISABLE_COMMON_WARNINGS

#include <algorithm>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

constexpr static uint16_t const ITERATIONS = 10U;
constexpr static float const STABILIZATION_FACTOR = 0.5F;

static_assert ( STABILIZATION_FACTOR >= 0.0F && STABILIZATION_FACTOR <= 1.0F,
    "The stabilization factor must be in range [0.0F, 1.0F]" );

void VelocitySolver::Run ( ContactManager &contactManager, float fixedTimeStep ) noexcept
{
    for ( auto& manifold : contactManager.GetContactManifolds () )
    {
        RigidBody& bodyA = *manifold._bodyA;
        RigidBody& bodyB = *manifold._bodyB;

        if ( bodyA.IsKinematic () )
        {
            SwapBodies ( manifold );
            SolveSingle ( manifold, fixedTimeStep );
            continue;
        }

        if ( bodyB.IsKinematic () )
        {
            SolveSingle ( manifold, fixedTimeStep );
            continue;
        }

        SolvePair ( manifold );
    }
}

void VelocitySolver::SolvePair ( ContactManifold &/*manifold*/ ) noexcept
{
    // TODO
}

void VelocitySolver::SolveSingle ( ContactManifold &manifold, float fixedTimeStep ) noexcept
{
    RigidBody& dynamicBody = *manifold._bodyA.get ();
    RigidBody const& kinematicBody = *manifold._bodyB.get ();

    GXVec3 const& locDyn = dynamicBody.GetLocation ();
    GXMat3 const& invInertiaTensor = dynamicBody.GetInertiaTensorInverse ();
    float const invMass = dynamicBody.GetMassInverse ();
    float const gamma = -( STABILIZATION_FACTOR / fixedTimeStep ) * manifold._penetration;

    GXVec3 const& locKin = kinematicBody.GetLocation ();
    GXVec3 const& angVelKin = kinematicBody.GetVelocityAngular ();
    GXVec3 const& velKin = kinematicBody.GetVelocityLinear ();

    GXVec3 minusN ( manifold._normal );
    minusN.Reverse ();

    GXVec3 rDyn {};
    GXVec3 vDyn {};

    GXVec3 rKin {};
    GXVec3 vKin {};

    GXVec3 rest {};
    GXVec3 diff {};
    GXVec3 vTmp;

    GXVec3 alpha {};
    GXVec3 beta {};
    GXVec3 cross {};

    for ( size_t i = 0U; i < manifold._contactCount; ++i )
    {
        Contact& contact = manifold._contacts[ i ];
        VelocitySolverData& data = contact._dataN;

        rDyn.Subtract ( contact._pointA, locDyn );
        cross.CrossProduct ( manifold._normal, rDyn );
        data._j.From ( minusN, cross );

        // Vector matrix or matrix vector?
        invInertiaTensor.MultiplyMatrixVector ( beta, cross );
        alpha.Multiply ( minusN, invMass );
        data._mj.From ( alpha, beta );
        data._effectiveMass = -1.0F / data._j.DotProduct ( data._mj );

        rKin.Subtract ( contact._pointB, locKin );
        vKin.CrossProduct ( angVelKin, rKin );

        // The operands are swapped to get negative vector. It will be used later for "b" calculation.
        vDyn.CrossProduct ( rDyn, dynamicBody.GetVelocityAngular () );

        diff.Subtract ( velKin, dynamicBody.GetVelocityLinear () );
        vTmp.Sum ( vDyn, vKin );
        rest.Sum ( diff, vTmp );

        float const restitution = dynamicBody.GetRestitution () * kinematicBody.GetRestitution ();
        contact._b = gamma + restitution * rest.DotProduct ( manifold._normal );
    }

    GXVec6 v2 {};

    for ( uint16_t iteration = 0U; iteration < ITERATIONS; ++iteration )
    {
        for ( size_t i = 0U; i < manifold._contactCount; ++i )
        {
            Contact& contact = manifold._contacts[ i ];
            VelocitySolverData& data = contact._dataN;

            GXVec6 const v1 ( dynamicBody.GetVelocityLinear (), dynamicBody.GetVelocityAngular () );
            float l = data._effectiveMass * ( data._j.DotProduct ( v1 ) + contact._b );

            float const oldLambda = data._lambda;
            data._lambda = std::max ( 0.0F, data._lambda + l );
            l = data._lambda - oldLambda;

            v2.Multiply ( data._mj, l );
            v2.Sum ( v1, v2 );

            dynamicBody.SetVelocityLinear ( v2._data[ 0U ], v2._data[ 1U ], v2._data[ 2U ] );
            dynamicBody.SetVelocityAngular ( v2._data[ 3U ], v2._data[ 4U ], v2._data[ 5U ] );
        }
    }
}

void VelocitySolver::SwapBodies ( ContactManifold &manifold ) noexcept
{
    std::swap ( manifold._bodyA, manifold._bodyB );
    manifold._normal.Reverse ();
    manifold._tangent.Reverse ();

    for ( size_t i = 0U; i < manifold._contactCount; ++i )
    {
        Contact& contacts = manifold._contacts[ i ];
        std::swap ( contacts._pointA, contacts._pointB );
    }
}

} // namespace android_vulkan
