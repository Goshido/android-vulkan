#include <velocity_solver.h>
#include <logger.h>

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
    RigidBody& bodyDynamic = *manifold._bodyA.get ();
    RigidBody const& bodyKinematic = *manifold._bodyB.get ();

    GXVec3 const& cmDynamic = bodyDynamic.GetLocation ();
    GXMat3 const& invInertiaTensor = bodyDynamic.GetInertiaTensorInverse ();
    float const invMass = bodyDynamic.GetMassInverse ();
    float const gamma = -( STABILIZATION_FACTOR / fixedTimeStep ) * manifold._penetration;

    GXVec3 const& cmKinematic = bodyKinematic.GetLocation ();
    GXVec3 const& vKinematic = bodyKinematic.GetVelocityLinear ();
    GXVec3 const& wKinematic = bodyKinematic.GetVelocityAngular ();
    GXVec6 const vwKinematic ( vKinematic, wKinematic );

    float const restitution = bodyDynamic.GetRestitution () * bodyKinematic.GetRestitution ();

    auto setup = [ & ] ( VelocitySolverData &data,
        GXVec3 const &axis,
        GXVec3 const &rA,
        GXVec3 const &rB,
        float b,
        LambdaClipHandler handler,
        void* context
    ) noexcept {
        data._b = b;

        data._clipHandler = handler;
        data._clipContext = context;

        GXVec3 cross {};
        cross.CrossProduct ( rB, axis );
        data._j[ 1U ].From ( axis, cross );

        // Note we changed operands to make negative vector.
        cross.CrossProduct ( axis, rA );

        GXVec3 neg ( axis );
        neg.Reverse ();

        data._j[ 0U ].From ( neg, cross );

        GXVec3 beta {};
        invInertiaTensor.MultiplyMatrixVector ( beta, cross );

        GXVec3 alpha {};
        alpha.Multiply ( neg, invMass );
        data._mj.From ( alpha, beta );
        data._effectiveMass = -1.0F / data._j[ 0U ].DotProduct ( data._mj );
    };

    for ( size_t i = 0U; i < manifold._contactCount; ++i )
    {
        Contact& contact = manifold._contacts[ i ];
        contact._friction = std::max ( bodyDynamic.GetFriction (), bodyKinematic.GetFriction () );

        GXVec3 rB {};
        rB.Subtract ( contact._pointB, cmKinematic );

        GXVec3 vRotB {};
        vRotB.CrossProduct ( wKinematic, rB );

        GXVec3 rA {};
        rA.Subtract ( contact._pointA, cmDynamic );

        // The operands are swapped to get negative vector. It will be used later for "b" calculation.
        GXVec3 vRotA {};
        vRotA.CrossProduct ( rA, bodyDynamic.GetVelocityAngular () );

        GXVec3 alpha {};
        alpha.Subtract ( vKinematic, bodyDynamic.GetVelocityLinear () );

        GXVec3 beta {};
        beta.Sum ( vRotA, vRotB );

        GXVec3 vClosing {};
        vClosing.Sum ( alpha, beta );

        float const b = gamma + restitution * vClosing.DotProduct ( manifold._normal );

        setup ( contact._dataT,
            manifold._tangent,
            rA,
            rB,
            0.0F,
            &VelocitySolver::LambdaClipFriction,
            &contact
        );

        setup ( contact._dataB,
            manifold._bitangent,
            rA,
            rB,
            0.0F,
            &VelocitySolver::LambdaClipFriction,
            &contact
        );

        setup ( contact._dataN, manifold._normal, rA, rB, b, &VelocitySolver::LambdaClipNormalForce, nullptr );
    }

    // Sequential impulse algorithm.

    auto solve = [ & ] ( VelocitySolverData &data ) noexcept {
        GXVec6 const vw1Dynamic ( bodyDynamic.GetVelocityLinear (), bodyDynamic.GetVelocityAngular () );

        float l = data._effectiveMass *
            ( data._j[ 0U ].DotProduct ( vw1Dynamic ) + data._j[ 1U ].DotProduct ( vwKinematic ) + data._b );

        float const oldLambda = data._lambda;
        data._lambda = data._clipHandler ( data._lambda + l, data._clipContext );
        l = data._lambda - oldLambda;

        GXVec6 vwDelta {};
        vwDelta.Multiply ( data._mj, l );

        GXVec6 vw2Dynamic {};
        vw2Dynamic.Sum ( vw1Dynamic, vwDelta );

        bodyDynamic.SetVelocityLinear ( vw2Dynamic._data[ 0U ], vw2Dynamic._data[ 1U ], vw2Dynamic._data[ 2U ] );
        bodyDynamic.SetVelocityAngular ( vw2Dynamic._data[ 3U ], vw2Dynamic._data[ 4U ], vw2Dynamic._data[ 5U ] );
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
