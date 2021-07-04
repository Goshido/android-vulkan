#include <velocity_solver.h>

GX_DISABLE_COMMON_WARNINGS

#include <algorithm>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

constexpr static uint16_t const ITERATIONS = 4U;
constexpr static float const STABILIZATION_FACTOR = 0.2F;

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

        SolvePair ( manifold, fixedTimeStep );
    }
}

float VelocitySolver::LambdaClipNormalForce ( float lambda, void* /*context*/ ) noexcept
{
    return std::max ( 0.0F, lambda );
}

float VelocitySolver::LambdaClipFriction ( float lambda, void* context ) noexcept
{
    auto const& contact = *static_cast<Contact const*> ( context );
    float const limit = contact._dataN._lambda * contact._friction;
    return std::clamp ( lambda, -limit, limit );
}

void VelocitySolver::SolvePair ( ContactManifold &/*manifold*/, float /*fixedTimeStep*/ ) noexcept
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
    GXVec3 neg {};

    auto setup = [ & ] ( VelocitySolverData &data,
        GXVec3 const &axis,
        GXVec3 const &r,
        float b,
        LambdaClipHandler handler,
        void* context
    ) noexcept {
        data._b = b;

        data._clipHandler = handler;
        data._clipContext = context;

        cross.CrossProduct ( axis, r );

        neg = axis;
        neg.Reverse ();

        data._j.From ( neg, cross );

        invInertiaTensor.MultiplyMatrixVector ( beta, cross );
        alpha.Multiply ( neg, invMass );
        data._mj.From ( alpha, beta );
        data._effectiveMass = -1.0F / data._j.DotProduct ( data._mj );
    };

    for ( size_t i = 0U; i < manifold._contactCount; ++i )
    {
        Contact& contact = manifold._contacts[ i ];
        contact._friction = std::max ( dynamicBody.GetFriction (), kinematicBody.GetFriction () );

        rKin.Subtract ( contact._pointB, locKin );
        vKin.CrossProduct ( angVelKin, rKin );

        rDyn.Subtract ( contact._pointA, locDyn );

        // The operands are swapped to get negative vector. It will be used later for "b" calculation.
        vDyn.CrossProduct ( rDyn, dynamicBody.GetVelocityAngular () );

        diff.Subtract ( velKin, dynamicBody.GetVelocityLinear () );
        vTmp.Sum ( vDyn, vKin );
        rest.Sum ( diff, vTmp );

        float const restitution = dynamicBody.GetRestitution () * kinematicBody.GetRestitution ();
        float const b = gamma + restitution * rest.DotProduct ( manifold._normal );

        setup ( contact._dataT,
            manifold._tangent,
            rDyn,
            0.0F,
            &VelocitySolver::LambdaClipFriction,
            &contact
        );

        setup ( contact._dataB,
            manifold._bitangent,
            rDyn,
            0.0F,
            &VelocitySolver::LambdaClipFriction,
            &contact
        );

        setup ( contact._dataN, manifold._normal, rDyn, b, &VelocitySolver::LambdaClipNormalForce, nullptr );
    }

    // Sequential impulse algorithm.

    GXVec6 v2 {};

    auto solve = [ & ] ( VelocitySolverData &data ) noexcept {
        GXVec6 const v1 ( dynamicBody.GetVelocityLinear (), dynamicBody.GetVelocityAngular () );
        float l = data._effectiveMass * ( data._j.DotProduct ( v1 ) + data._b );

        float const oldLambda = data._lambda;
        data._lambda = data._clipHandler ( data._lambda + l, data._clipContext );
        l = data._lambda - oldLambda;

        v2.Multiply ( data._mj, l );
        v2.Sum ( v1, v2 );

        dynamicBody.SetVelocityLinear ( v2._data[ 0U ], v2._data[ 1U ], v2._data[ 2U ] );
        dynamicBody.SetVelocityAngular ( v2._data[ 3U ], v2._data[ 4U ], v2._data[ 5U ] );
    };

    for ( uint16_t iteration = 0U; iteration < ITERATIONS; ++iteration )
    {
        for ( size_t i = 0U; i < manifold._contactCount; ++i )
        {
            Contact& contact = manifold._contacts[ i ];
            solve ( contact._dataN );
            solve ( contact._dataT );
            solve ( contact._dataB );
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
