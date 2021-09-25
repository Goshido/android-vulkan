#include <velocity_solver.h>
#include <logger.h>

GX_DISABLE_COMMON_WARNINGS

#include <algorithm>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

constexpr static uint16_t const ITERATIONS = 7U;
constexpr static float const STABILIZATION_FACTOR = 0.4F;
constexpr static float const PENETRATION_SLOPE = 5.0e-4F;
constexpr static float const RESTITUTION_SLOPE = 5.0e-1F;

static_assert ( STABILIZATION_FACTOR >= 0.0F && STABILIZATION_FACTOR <= 1.0F,
    "The stabilization factor must be in range [0.0F, 1.0F]" );

//----------------------------------------------------------------------------------------------------------------------

void VelocitySolver::Run ( ContactManager &contactManager, float fixedTimeStepInverse ) noexcept
{
    //DebugWarmStart ( contactManager );
    auto& manifolds = contactManager.GetContactManifolds ();

    for ( auto& manifold : manifolds )
    {
        if ( manifold._bodyA->IsKinematic () )
        {
            SwapBodies ( manifold );
            PrepareSingle ( manifold, fixedTimeStepInverse );
            continue;
        }

        if ( manifold._bodyB->IsKinematic () )
        {
            PrepareSingle ( manifold, fixedTimeStepInverse );
            continue;
        }

        PreparePair ( manifold, fixedTimeStepInverse );
    }

    // Sequential impulse algorithm.

    for ( uint16_t i = 0U; i < ITERATIONS; ++i )
    {
        for ( auto& manifold : manifolds )
        {
            // Note after preprocessing only B could be kinematic object.
            if ( manifold._bodyB->IsKinematic () )
            {
                SolveSingle ( manifold );
                continue;
            }

            SolvePair ( manifold );
        }
    }
}

[[maybe_unused]] void VelocitySolver::DebugWarmStart ( ContactManager const &contactManager ) noexcept
{
    size_t totalContacts = 0U;
    size_t warmStarted = 0U;

    for ( auto& manifold : contactManager.GetContactManifolds () )
    {
        totalContacts += manifold._contactCount;
        Contact const* contacts = manifold._contacts;
        size_t contactCount = manifold._contactCount;

        for ( size_t i = 0U; i < contactCount; ++i )
        {
            if ( contacts[ i ]._warmStarted )
            {
                ++warmStarted;
            }
        }
    }

    constexpr char const format[] =
R"__(Warm starting:
>>>
    Total contacts: %zu
    Warm started: %6.2f%%
<<<)__";

    android_vulkan::LogDebug ( format,
        totalContacts,
        totalContacts ? static_cast<float> ( 100U * warmStarted ) / static_cast<float> ( totalContacts ) : 0.0F
    );
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

void VelocitySolver::PreparePair ( ContactManifold &manifold, float fixedTimeStepInverse ) noexcept
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

        float const b = gamma * std::max ( contact._penetration - PENETRATION_SLOPE, 0.0F ) +
            contact._restitution * std::max ( vClosing.DotProduct ( contact._normal ) - RESTITUTION_SLOPE, 0.0F );

        setup ( contact._dataT,
            contact._tangent,
            rA,
            rB,
            0.0F,
            &VelocitySolver::LambdaClipFriction,
            &contact
        );

        setup ( contact._dataB,
            contact._bitangent,
            rA,
            rB,
            0.0F,
            &VelocitySolver::LambdaClipFriction,
            &contact
        );

        setup ( contact._dataN, contact._normal, rA, rB, b, &VelocitySolver::LambdaClipNormalForce, nullptr );
    }

    // Apply warm start impulses.

    for ( size_t i = 0U; i < manifold._contactCount; ++i )
    {
        Contact& contact = manifold._contacts[ i ];

        UpdateVelocityPair ( bodyA,
            bodyB,
            contact._dataN,
            bodyA.GetVelocities (),
            bodyB.GetVelocities (),
            contact._dataN._lambda
        );

        UpdateVelocityPair ( bodyA,
            bodyB,
            contact._dataT,
            bodyA.GetVelocities (),
            bodyB.GetVelocities (),
            contact._dataT._lambda
        );

        UpdateVelocityPair ( bodyA,
            bodyB,
            contact._dataB,
            bodyA.GetVelocities (),
            bodyB.GetVelocities (),
            contact._dataB._lambda
        );
    }
}

void VelocitySolver::PrepareSingle ( ContactManifold &manifold, float fixedTimeStepInverse ) noexcept
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

        float const b = gamma * std::max ( contact._penetration - PENETRATION_SLOPE, 0.0F ) +
            contact._restitution * std::max ( vClosing.DotProduct ( contact._normal ) - RESTITUTION_SLOPE, 0.0F );

        setup ( contact._dataT,
            contact._tangent,
            rA,
            rB,
            0.0F,
            &VelocitySolver::LambdaClipFriction,
            &contact
        );

        setup ( contact._dataB,
            contact._bitangent,
            rA,
            rB,
            0.0F,
            &VelocitySolver::LambdaClipFriction,
            &contact
        );

        setup ( contact._dataN, contact._normal, rA, rB, b, &VelocitySolver::LambdaClipNormalForce, nullptr );
    }

    // Apply warm start impulses.

    for ( size_t i = 0U; i < manifold._contactCount; ++i )
    {
        Contact& contact = manifold._contacts[ i ];
        UpdateVelocitySingle ( bodyDynamic, contact._dataN, bodyDynamic.GetVelocities (), contact._dataN._lambda );
        UpdateVelocitySingle ( bodyDynamic, contact._dataT, bodyDynamic.GetVelocities (), contact._dataT._lambda );
        UpdateVelocitySingle ( bodyDynamic, contact._dataB, bodyDynamic.GetVelocities (), contact._dataB._lambda );
    }
}

void VelocitySolver::SolvePair ( ContactManifold &manifold ) noexcept
{
    RigidBody& bodyA = *manifold._bodyA.get ();
    RigidBody& bodyB = *manifold._bodyB.get ();

    auto solve = [ & ] ( VelocitySolverData &data ) noexcept {
        GXVec6 const vw1A = bodyA.GetVelocities ();
        GXVec6 const vw1B = bodyB.GetVelocities ();

        float const d0 = data._j[ 0U ].DotProduct ( vw1A );
        float const d1 = data._j[ 1U ].DotProduct ( vw1B );

        float l = data._effectiveMass * ( d0 + d1 + data._b );

        float const oldLambda = data._lambda;
        data._lambda = data._clipHandler ( data._lambda + l, data._clipContext );
        l = data._lambda - oldLambda;

        UpdateVelocityPair ( bodyA, bodyB, data, vw1A, vw1B, l );
    };

    for ( size_t i = 0U; i < manifold._contactCount; ++i )
    {
        Contact& contact = manifold._contacts[ i ];
        solve ( contact._dataN );
        solve ( contact._dataT );
        solve ( contact._dataB );
    }
}

void VelocitySolver::SolveSingle ( ContactManifold &manifold ) noexcept
{
    RigidBody& bodyDynamic = *manifold._bodyA.get ();
    RigidBody const& bodyKinematic = *manifold._bodyB.get ();
    GXVec6 const vwKinematic = bodyKinematic.GetVelocities ();

    auto solve = [ & ] ( VelocitySolverData &data ) noexcept {
        GXVec6 const vw1Dynamic = bodyDynamic.GetVelocities ();

        float l = data._effectiveMass *
            ( data._j[ 0U ].DotProduct ( vw1Dynamic ) + data._j[ 1U ].DotProduct ( vwKinematic ) + data._b );

        float const oldLambda = data._lambda;
        data._lambda = data._clipHandler ( data._lambda + l, data._clipContext );
        l = data._lambda - oldLambda;

        UpdateVelocitySingle ( bodyDynamic, data, vw1Dynamic, l );
    };

    for ( size_t i = 0U; i < manifold._contactCount; ++i )
    {
        Contact& contact = manifold._contacts[ i ];
        solve ( contact._dataN );
        solve ( contact._dataT );
        solve ( contact._dataB );
    }
}

void VelocitySolver::SwapBodies ( ContactManifold &manifold ) noexcept
{
    std::swap ( manifold._bodyA, manifold._bodyB );

    for ( size_t i = 0U; i < manifold._contactCount; ++i )
    {
        Contact& contact = manifold._contacts[ i ];
        contact._normal.Reverse ();
        contact._tangent.Reverse ();
        std::swap ( contact._pointA, contact._pointB );
    }
}

void VelocitySolver::UpdateVelocityPair ( RigidBody &bodyA,
    RigidBody &bodyB,
    VelocitySolverData const &data,
    GXVec6 const &vw1A,
    GXVec6 const &vw1B,
    float lambda
) noexcept
{
    GXVec6 vwADelta {};
    vwADelta.Multiply ( data._mj[ 0U ], lambda );

    GXVec6 vwBDelta {};
    vwBDelta.Multiply ( data._mj[ 1U ], lambda );

    GXVec6 vw2A {};
    vw2A.Sum ( vw1A, vwADelta );

    GXVec6 vw2B {};
    vw2B.Sum ( vw1B, vwBDelta );

    bodyA.SetVelocities ( vw2A );
    bodyB.SetVelocities ( vw2B );
}

void VelocitySolver::UpdateVelocitySingle ( RigidBody &body,
    VelocitySolverData const &data,
    GXVec6 const &vw1,
    float lambda
) noexcept
{
    GXVec6 vwDelta {};
    vwDelta.Multiply ( data._mj[ 0U ], lambda );

    GXVec6 vw2 {};
    vw2.Sum ( vw1, vwDelta );

    body.SetVelocities ( vw2 );
}

} // namespace android_vulkan
