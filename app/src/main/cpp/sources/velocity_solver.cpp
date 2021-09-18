#include <velocity_solver.h>
#include <logger.h>

GX_DISABLE_COMMON_WARNINGS

#include <algorithm>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

constexpr static uint16_t const ITERATIONS = 4U;
constexpr static float const STABILIZATION_FACTOR = 0.4F;

static_assert ( STABILIZATION_FACTOR >= 0.0F && STABILIZATION_FACTOR <= 1.0F,
    "The stabilization factor must be in range [0.0F, 1.0F]" );

//----------------------------------------------------------------------------------------------------------------------

void VelocitySolver::Run ( ContactManager &contactManager, float fixedTimeStepInverse ) noexcept
{
    for ( auto& manifold : contactManager.GetContactManifolds () )
    {
        RigidBody& bodyA = *manifold._bodyA;
        RigidBody& bodyB = *manifold._bodyB;

        if ( bodyA.IsKinematic () )
        {
            SwapBodies ( manifold );
            SolveSingle ( manifold, fixedTimeStepInverse );
            continue;
        }

        if ( bodyB.IsKinematic () )
        {
            SolveSingle ( manifold, fixedTimeStepInverse );
            continue;
        }

        SolvePair ( manifold, fixedTimeStepInverse );
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

void VelocitySolver::SolvePair ( ContactManifold &manifold, float fixedTimeStepInverse ) noexcept
{
    RigidBody& bodyA = *manifold._bodyA.get ();
    RigidBody& bodyB = *manifold._bodyB.get ();

    GXVec3 const& cmA = bodyA.GetLocation ();
    GXVec3 const& cmB = bodyB.GetLocation ();

    GXVec3 const& wA = bodyA.GetVelocityAngular ();
    GXVec3 const& wB = bodyB.GetVelocityAngular ();

    GXMat3 const& invInertiaTensorA = bodyA.GetInertiaTensorInverse ();
    GXMat3 const& invInertiaTensorB = bodyB.GetInertiaTensorInverse ();

    float const invMassA = bodyA.GetMassInverse ();
    float const invMassB = bodyB.GetMassInverse ();

    float const gamma = -STABILIZATION_FACTOR * fixedTimeStepInverse;

    GXVec3 dV {};
    dV.Subtract ( bodyB.GetVelocityLinear (), bodyA.GetVelocityLinear () );

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

        // Note we changed operands to make negative vector.
        GXVec3 crossA {};
        crossA.CrossProduct ( axis, rA );

        GXVec3 crossB {};
        crossB.CrossProduct ( rB, axis );

        GXVec3 neg ( axis );
        neg.Reverse ();

        data._j[ 0U ].From ( neg, crossA );
        data._j[ 1U ].From ( axis, crossB );

        GXVec3 alpha {};
        invInertiaTensorA.MultiplyMatrixVector ( alpha, crossA );

        GXVec3 beta {};
        invInertiaTensorB.MultiplyMatrixVector ( beta, crossB );

        GXVec3 omega {};
        omega.Multiply ( neg, invMassA );

        GXVec3 iota {};
        iota.Multiply ( axis, invMassB );

        data._mj[ 0U ].From ( omega, alpha );
        data._mj[ 1U ].From ( iota, beta );

        float const d0 = data._j[ 0U ].DotProduct ( data._mj[ 0U ] );
        float const d1 = data._j[ 1U ].DotProduct ( data._mj[ 1U ] );

        data._effectiveMass = -1.0F / ( d0 + d1 );
    };

    for ( size_t i = 0U; i < manifold._contactCount; ++i )
    {
        Contact& contact = manifold._contacts[ i ];

        GXVec3 rA {};
        rA.Subtract ( contact._pointA, cmA );

        GXVec3 rB {};
        rB.Subtract ( contact._pointB, cmB );

        // The operands are swapped to get negative vector. It will be used later for "b" calculation.
        GXVec3 rotA {};
        rotA.CrossProduct ( rA, wA );

        GXVec3 rotB {};
        rotB.CrossProduct ( wB, rB );

        GXVec3 alpha {};
        alpha.Sum ( rotA, rotB );

        GXVec3 vClosing {};
        vClosing.Sum ( alpha, dV );

        float const b = gamma * contact._penetration + contact._restitution * vClosing.DotProduct ( manifold._normal );

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

    // Apply warm start impulses.

    auto update = [ & ] ( VelocitySolverData const &data,
        GXVec6 const &vw1A,
        GXVec6 const &vw1B,
        float lambda
    ) noexcept {
        GXVec6 vwADelta {};
        vwADelta.Multiply ( data._mj[ 0U ], lambda );

        GXVec6 vwBDelta {};
        vwBDelta.Multiply ( data._mj[ 1U ], lambda );

        GXVec6 vw2A {};
        vw2A.Sum ( vw1A, vwADelta );

        GXVec6 vw2B {};
        vw2B.Sum ( vw1B, vwBDelta );

        bodyA.SetVelocityLinear ( vw2A._data[ 0U ], vw2A._data[ 1U ], vw2A._data[ 2U ] );
        bodyB.SetVelocityLinear ( vw2B._data[ 0U ], vw2B._data[ 1U ], vw2B._data[ 2U ] );

        bodyA.SetVelocityAngular ( vw2A._data[ 3U ], vw2A._data[ 4U ], vw2A._data[ 5U ] );
        bodyB.SetVelocityAngular ( vw2B._data[ 3U ], vw2B._data[ 4U ], vw2B._data[ 5U ] );
    };

    for ( size_t i = 0U; i < manifold._contactCount; ++i )
    {
        Contact& contact = manifold._contacts[ i ];

        update ( contact._dataN,
            GXVec6 ( bodyA.GetVelocityLinear (), bodyA.GetVelocityAngular () ),
            GXVec6 ( bodyB.GetVelocityLinear (), bodyB.GetVelocityAngular () ),
            contact._dataN._lambda
        );

        update ( contact._dataT,
            GXVec6 ( bodyA.GetVelocityLinear (), bodyA.GetVelocityAngular () ),
            GXVec6 ( bodyB.GetVelocityLinear (), bodyB.GetVelocityAngular () ),
            contact._dataT._lambda
        );

        update ( contact._dataB,
            GXVec6 ( bodyA.GetVelocityLinear (), bodyA.GetVelocityAngular () ),
            GXVec6 ( bodyB.GetVelocityLinear (), bodyB.GetVelocityAngular () ),
            contact._dataB._lambda
        );
    }

    // Sequential impulse algorithm.

    auto solve = [ & ] ( VelocitySolverData &data ) noexcept {
        GXVec6 const vw1A ( bodyA.GetVelocityLinear (), bodyA.GetVelocityAngular () );
        GXVec6 const vw1B ( bodyB.GetVelocityLinear (), bodyB.GetVelocityAngular () );

        float const d0 = data._j[ 0U ].DotProduct ( vw1A );
        float const d1 = data._j[ 1U ].DotProduct ( vw1B );

        float l = data._effectiveMass * ( d0 + d1 + data._b );

        float const oldLambda = data._lambda;
        data._lambda = data._clipHandler ( data._lambda + l, data._clipContext );
        l = data._lambda - oldLambda;

        update ( data,
            GXVec6 ( bodyA.GetVelocityLinear (), bodyA.GetVelocityAngular () ),
            GXVec6 ( bodyB.GetVelocityLinear (), bodyB.GetVelocityAngular () ),
            l
        );
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

void VelocitySolver::SolveSingle ( ContactManifold &manifold, float fixedTimeStepInverse ) noexcept
{
    RigidBody& bodyDynamic = *manifold._bodyA.get ();
    RigidBody const& bodyKinematic = *manifold._bodyB.get ();

    GXVec3 const& cmDynamic = bodyDynamic.GetLocation ();
    GXMat3 const& invInertiaTensor = bodyDynamic.GetInertiaTensorInverse ();
    float const invMass = bodyDynamic.GetMassInverse ();

    float const gamma = -STABILIZATION_FACTOR * fixedTimeStepInverse;

    GXVec3 const& cmKinematic = bodyKinematic.GetLocation ();
    GXVec3 const& vKinematic = bodyKinematic.GetVelocityLinear ();
    GXVec3 const& wKinematic = bodyKinematic.GetVelocityAngular ();
    GXVec6 const vwKinematic ( vKinematic, wKinematic );

    GXVec3 dV {};
    dV.Subtract ( vKinematic, bodyDynamic.GetVelocityLinear () );

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

        // Note we changed operands to make negative vector.
        GXVec3 crossA {};
        crossA.CrossProduct ( axis, rA );

        GXVec3 crossB {};
        crossB.CrossProduct ( rB, axis );

        GXVec3 neg ( axis );
        neg.Reverse ();

        data._j[ 0U ].From ( neg, crossA );
        data._j[ 1U ].From ( axis, crossB );

        GXVec3 beta {};
        invInertiaTensor.MultiplyMatrixVector ( beta, crossA );

        GXVec3 alpha {};
        alpha.Multiply ( neg, invMass );
        data._mj[ 0U ].From ( alpha, beta );
        data._effectiveMass = -1.0F / data._j[ 0U ].DotProduct ( data._mj[ 0U ] );
    };

    for ( size_t i = 0U; i < manifold._contactCount; ++i )
    {
        Contact& contact = manifold._contacts[ i ];

        GXVec3 rA {};
        rA.Subtract ( contact._pointA, cmDynamic );

        GXVec3 rB {};
        rB.Subtract ( contact._pointB, cmKinematic );

        // The operands are swapped to get negative vector. It will be used later for "b" calculation.
        GXVec3 vRotA {};
        vRotA.CrossProduct ( rA, bodyDynamic.GetVelocityAngular () );

        GXVec3 vRotB {};
        vRotB.CrossProduct ( wKinematic, rB );

        GXVec3 alpha {};
        alpha.Sum ( vRotA, vRotB );

        GXVec3 vClosing {};
        vClosing.Sum ( dV, alpha );

        float const b = gamma * contact._penetration + contact._restitution * vClosing.DotProduct ( manifold._normal );

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

    // Apply warm start impulses.

    auto update = [ & ] ( VelocitySolverData const &data, GXVec6 const &vw1A, float lambda ) noexcept {
        GXVec6 vwDelta {};
        vwDelta.Multiply ( data._mj[ 0U ], lambda );

        GXVec6 vw2Dynamic {};
        vw2Dynamic.Sum ( vw1A, vwDelta );

        bodyDynamic.SetVelocityLinear ( vw2Dynamic._data[ 0U ], vw2Dynamic._data[ 1U ], vw2Dynamic._data[ 2U ] );
        bodyDynamic.SetVelocityAngular ( vw2Dynamic._data[ 3U ], vw2Dynamic._data[ 4U ], vw2Dynamic._data[ 5U ] );
    };

    for ( size_t i = 0U; i < manifold._contactCount; ++i )
    {
        Contact& contact = manifold._contacts[ i ];

        update ( contact._dataN,
            GXVec6 ( bodyDynamic.GetVelocityLinear (), bodyDynamic.GetVelocityAngular () ),
            contact._dataN._lambda
        );

        update ( contact._dataT,
            GXVec6 ( bodyDynamic.GetVelocityLinear (), bodyDynamic.GetVelocityAngular () ),
            contact._dataT._lambda
        );

        update ( contact._dataB,
            GXVec6 ( bodyDynamic.GetVelocityLinear (), bodyDynamic.GetVelocityAngular () ),
            contact._dataB._lambda
        );

        update ( contact._dataT,
            GXVec6 ( bodyDynamic.GetVelocityLinear (), bodyDynamic.GetVelocityAngular () ),
            contact._dataT._lambda
        );
    }

    // Sequential impulse algorithm.

    auto solve = [ & ] ( VelocitySolverData &data ) noexcept {
        GXVec6 const vw1Dynamic ( bodyDynamic.GetVelocityLinear (), bodyDynamic.GetVelocityAngular () );

        float l = data._effectiveMass *
            ( data._j[ 0U ].DotProduct ( vw1Dynamic ) + data._j[ 1U ].DotProduct ( vwKinematic ) + data._b );

        float const oldLambda = data._lambda;
        data._lambda = data._clipHandler ( data._lambda + l, data._clipContext );
        l = data._lambda - oldLambda;

        update ( data, vw1Dynamic, l );
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
