#include <velocity_solver.h>
#include <logger.h>

GX_DISABLE_COMMON_WARNINGS

#include <algorithm>
#include <cassert>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

constexpr static uint16_t const ITERATIONS = 5U;
constexpr static float const STABILIZATION_FACTOR = 0.21F;
constexpr static float const PENETRATION_SLOPE = 5.0e-4F;
constexpr static float const RESTITUTION_SLOPE = 5.0e-1F;

static_assert ( STABILIZATION_FACTOR >= 0.0F && STABILIZATION_FACTOR <= 1.0F,
    "The stabilization factor must be in range [0.0F, 1.0F]" );

//----------------------------------------------------------------------------------------------------------------------

void VelocitySolver::Run ( ContactManager &contactManager, float fixedTimeStepInverse ) noexcept
{
    //DebugContactInManifold ( contactManager );
    //DebugWarmStart ( contactManager );

    float const stabilizationFactor = -STABILIZATION_FACTOR * fixedTimeStepInverse;
    auto& manifolds = contactManager.GetContactManifolds ();

    for ( auto& manifold : manifolds )
    {
        if ( manifold._bodyA->IsKinematic () )
        {
            SwapBodies ( manifold );
            PrepareSingle ( manifold );
            continue;
        }

        if ( manifold._bodyB->IsKinematic () )
        {
            PrepareSingle ( manifold );
            continue;
        }

        PreparePair ( manifold );
    }

    // Sequential impulse algorithm:
    // The order is based on idea from Bullet: Solving normal impulses first. Then solving frictional impulses.

    for ( uint16_t i = 0U; i < ITERATIONS; ++i )
    {
        // Solving normal impulses.

        for ( auto& manifold : manifolds )
        {
            // Note after preprocessing only B could be kinematic object.
            if ( manifold._bodyB->IsKinematic () )
            {
                SolveSingleNormal ( manifold, stabilizationFactor );
                continue;
            }

            SolvePairNormal ( manifold, stabilizationFactor );
        }

        // Solving friction impulses

        for ( auto& manifold : manifolds )
        {
            // Note after preprocessing only B could be kinematic object.
            if ( manifold._bodyB->IsKinematic () )
            {
                SolveSingleFriction ( manifold );
                continue;
            }

            SolvePairFriction ( manifold );
        }
    }
}

float VelocitySolver::ComputeBaumgarteTerm ( GXVec3 const &wA,
    GXVec3 const &wB,
    GXVec3 const &cmA,
    GXVec3 const &cmB,
    GXVec3 const &dV,
    Contact const &contact,
    float stabilizationFactor
) noexcept
{
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

    return stabilizationFactor * std::max ( contact._penetration - PENETRATION_SLOPE, 0.0F ) +
        contact._restitution * std::max ( vClosing.DotProduct ( contact._normal ) - RESTITUTION_SLOPE, 0.0F );
}

[[maybe_unused]] void VelocitySolver::DebugContactInManifold ( ContactManager const &contactManager ) noexcept
{
    static size_t manifolds = 0U;
    static std::array<size_t, 8U> histogram = { 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U };

    for ( auto& manifold : contactManager.GetContactManifolds () )
    {
        assert ( manifold._contactCount > 0U );
        assert ( manifold._contactCount <= histogram.size () );
        ++manifolds;
        ++( histogram[ manifold._contactCount - 1U ] );
    }

    if ( manifolds == 0U )
        return;

    constexpr char const format[] =
R"__(Manifold variety:
      #1      #2      #3      #4      #5      #6      #7      #8
%7.01f%%%7.01f%%%7.01f%%%7.01f%%%7.01f%%%7.01f%%%7.01f%%%7.01f%%)__";

    float const a = 1.0F / static_cast<float> ( manifolds );

    auto conv = [ & ] ( size_t v ) noexcept -> float {
        return a * static_cast<float> ( 100U * v );
    };

    android_vulkan::LogDebug ( format,
        conv ( histogram[ 0U ] ),
        conv ( histogram[ 1U ] ),
        conv ( histogram[ 2U ] ),
        conv ( histogram[ 3U ] ),
        conv ( histogram[ 4U ] ),
        conv ( histogram[ 5U ] ),
        conv ( histogram[ 6U ] ),
        conv ( histogram[ 7U ] )
    );
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

void VelocitySolver::PreparePair ( ContactManifold &manifold ) noexcept
{
    RigidBody& bodyA = *manifold._bodyA.get ();
    RigidBody& bodyB = *manifold._bodyB.get ();
    Contact* contacts = manifold._contacts;

    GXVec3 const& cmA = bodyA.GetLocation ();
    GXVec3 const& cmB = bodyB.GetLocation ();

    GXMat3 const& invInertiaTensorA = bodyA.GetInertiaTensorInverse ();
    GXMat3 const& invInertiaTensorB = bodyB.GetInertiaTensorInverse ();

    float const invMassA = bodyA.GetMassInverse ();
    float const invMassB = bodyB.GetMassInverse ();

    auto setup = [ & ] ( VelocitySolverData &data, GXVec3 const &axis, GXVec3 const &rA, GXVec3 const &rB ) noexcept {
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
        Contact& contact = contacts[ i ];

        GXVec3 rA {};
        rA.Subtract ( contact._pointA, cmA );

        GXVec3 rB {};
        rB.Subtract ( contact._pointB, cmB );

        setup ( contact._dataT, contact._tangent, rA, rB );
        setup ( contact._dataB, contact._bitangent, rA, rB );
        setup ( contact._dataN, contact._normal, rA, rB );
    }

    // Apply warm start impulses.

    for ( size_t i = 0U; i < manifold._contactCount; ++i )
    {
        Contact& contact = contacts[ i ];

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

void VelocitySolver::PrepareSingle ( ContactManifold &manifold ) noexcept
{
    RigidBody& bodyDynamic = *manifold._bodyA.get ();
    RigidBody const& bodyKinematic = *manifold._bodyB.get ();

    Contact* contacts = manifold._contacts;

    GXVec3 const& cmDynamic = bodyDynamic.GetLocation ();
    GXMat3 const& invInertiaTensor = bodyDynamic.GetInertiaTensorInverse ();
    float const invMass = bodyDynamic.GetMassInverse ();

    GXVec3 const& cmKinematic = bodyKinematic.GetLocation ();

    auto setup = [ & ] ( VelocitySolverData &data, GXVec3 const &axis, GXVec3 const &rA, GXVec3 const &rB ) noexcept {
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
        Contact& contact = contacts[ i ];

        GXVec3 rA {};
        rA.Subtract ( contact._pointA, cmDynamic );

        GXVec3 rB {};
        rB.Subtract ( contact._pointB, cmKinematic );

        setup ( contact._dataT, contact._tangent, rA, rB );
        setup ( contact._dataB, contact._bitangent, rA, rB );
        setup ( contact._dataN, contact._normal, rA, rB );
    }

    // Apply warm start impulses.

    for ( size_t i = 0U; i < manifold._contactCount; ++i )
    {
        Contact& contact = contacts[ i ];
        UpdateVelocitySingle ( bodyDynamic, contact._dataN, bodyDynamic.GetVelocities (), contact._dataN._lambda );
        UpdateVelocitySingle ( bodyDynamic, contact._dataT, bodyDynamic.GetVelocities (), contact._dataT._lambda );
        UpdateVelocitySingle ( bodyDynamic, contact._dataB, bodyDynamic.GetVelocities (), contact._dataB._lambda );
    }
}

void VelocitySolver::SolvePairFriction ( ContactManifold &manifold ) noexcept
{
    // Note: the trick with total impulse calculation does not work for friction. The simulation is very unstable
    // for some reason. There is no any explanation for this. So sticking with traditional velocity updating.

    RigidBody& bodyA = *manifold._bodyA.get ();
    RigidBody& bodyB = *manifold._bodyB.get ();

    auto solve = [ & ] ( VelocitySolverData &data, float limitMin, float limitMax ) noexcept {
        GXVec6 const vw1A = bodyA.GetVelocities ();
        GXVec6 const vw1B = bodyB.GetVelocities ();

        float const d0 = data._j[ 0U ].DotProduct ( vw1A );
        float const d1 = data._j[ 1U ].DotProduct ( vw1B );

        float l = data._effectiveMass * ( d0 + d1 );
        float const oldLambda = data._lambda;
        data._lambda = std::clamp ( data._lambda + l, limitMin, limitMax );
        l = data._lambda - oldLambda;

        UpdateVelocityPair ( bodyA, bodyB, data, vw1A, vw1B, l );
    };

    for ( size_t i = 0U; i < manifold._contactCount; ++i )
    {
        Contact& contact = manifold._contacts[ i ];

        float const limitMax = contact._dataN._lambda * contact._friction;
        float const limitMin = -limitMax;

        solve ( contact._dataT, limitMin, limitMax );
        solve ( contact._dataB, limitMin, limitMax );
    }
}

void VelocitySolver::SolvePairNormal ( ContactManifold &manifold, float stabilizationFactor ) noexcept
{
    RigidBody& bodyA = *manifold._bodyA.get ();
    RigidBody& bodyB = *manifold._bodyB.get ();

    GXVec6 const vw1A = bodyA.GetVelocities ();
    GXVec6 const vw1B = bodyB.GetVelocities ();

    GXVec3 const& wA = bodyA.GetVelocityAngular ();
    GXVec3 const& wB = bodyB.GetVelocityAngular ();

    GXVec3 const& cmA = bodyA.GetLocation ();
    GXVec3 const& cmB = bodyB.GetLocation ();

    GXVec6 vwADelta ( 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F );
    GXVec6 vwBDelta ( 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F );

    GXVec3 deltaVelocityLinear {};
    deltaVelocityLinear.Subtract ( bodyB.GetVelocityLinear (), bodyA.GetVelocityLinear () );

    auto solve = [ & ] ( Contact &contact ) noexcept {
        VelocitySolverData& data = contact._dataN;
        data._b = ComputeBaumgarteTerm ( wA, wB, cmA, cmB, deltaVelocityLinear, contact, stabilizationFactor );

        float const d0 = data._j[ 0U ].DotProduct ( vw1A );
        float const d1 = data._j[ 1U ].DotProduct ( vw1B );

        float l = data._effectiveMass * ( d0 + d1 + data._b );
        float const oldLambda = data._lambda;
        data._lambda = std::max ( 0.0F, data._lambda + l );
        l = data._lambda - oldLambda;

        vwADelta.Sum ( vwADelta, l, data._mj[ 0U ] );
        vwBDelta.Sum ( vwBDelta, l, data._mj[ 1U ] );
    };

    Contact* contacts = manifold._contacts;

    for ( size_t i = 0U; i < manifold._contactCount; ++i )
        solve ( contacts[ i ] );

    GXVec6 vw2A {};
    vw2A.Sum ( vw1A, vwADelta );

    GXVec6 vw2B {};
    vw2B.Sum ( vw1B, vwBDelta );

    bodyA.SetVelocities ( vw2A );
    bodyB.SetVelocities ( vw2B );
}

void VelocitySolver::SolveSingleFriction ( ContactManifold &manifold ) noexcept
{
    // Note: the trick with total impulse calculation does not work for friction. The simulation is very unstable
    // for some reason. There is no any explanation for this. So sticking with traditional velocity updating.

    RigidBody& bodyDynamic = *manifold._bodyA.get ();
    RigidBody const& bodyKinematic = *manifold._bodyB.get ();

    GXVec6 const vwKinematic = bodyKinematic.GetVelocities ();

    auto solve = [ & ] ( VelocitySolverData &data, float limitMin, float limitMax ) noexcept {
        GXVec6 const vw1Dynamic = bodyDynamic.GetVelocities ();

        float const d0 = data._j[ 0U ].DotProduct ( vw1Dynamic );
        float const d1 = data._j[ 1U ].DotProduct ( vwKinematic );

        float l = data._effectiveMass * ( d0 + d1 );
        float const oldLambda = data._lambda;
        data._lambda = std::clamp ( data._lambda + l, limitMin, limitMax );
        l = data._lambda - oldLambda;

        UpdateVelocitySingle ( bodyDynamic, data, vw1Dynamic, l );
    };

    for ( size_t i = 0U; i < manifold._contactCount; ++i )
    {
        Contact& contact = manifold._contacts[ i ];

        float const limitMax = contact._dataN._lambda * contact._friction;
        float const limitMin = -limitMax;

        solve ( contact._dataT, limitMin, limitMax );
        solve ( contact._dataB, limitMin, limitMax );
    }
}

void VelocitySolver::SolveSingleNormal ( ContactManifold &manifold, float stabilizationFactor ) noexcept
{
    RigidBody& bodyDynamic = *manifold._bodyA.get ();
    RigidBody const& bodyKinematic = *manifold._bodyB.get ();

    GXVec6 const vwKinematic = bodyKinematic.GetVelocities ();
    GXVec6 const vwDynamic = bodyDynamic.GetVelocities ();

    GXVec3 const& wDynamic = bodyDynamic.GetVelocityAngular ();
    GXVec3 const& wKinematic = bodyKinematic.GetVelocityAngular ();

    GXVec3 const& cmDynamic = bodyDynamic.GetLocation ();
    GXVec3 const& cmKinematic = bodyKinematic.GetLocation ();

    GXVec6 vwDelta ( 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F );

    GXVec3 deltaVelocityLinear {};
    deltaVelocityLinear.Subtract ( bodyKinematic.GetVelocityLinear (), bodyDynamic.GetVelocityLinear () );

    auto solve = [ & ] ( Contact &contact ) noexcept {
        VelocitySolverData& data = contact._dataN;

        data._b = ComputeBaumgarteTerm ( wDynamic,
            wKinematic,
            cmDynamic,
            cmKinematic,
            deltaVelocityLinear,
            contact,
            stabilizationFactor
        );

        float const d0 = data._j[ 0U ].DotProduct ( vwDynamic );
        float const d1 = data._j[ 1U ].DotProduct ( vwKinematic );

        float l = data._effectiveMass * ( d0 + d1 + data._b );
        float const oldLambda = data._lambda;
        data._lambda = std::max ( 0.0F, data._lambda + l );
        l = data._lambda - oldLambda;

        vwDelta.Sum ( vwDelta, l, data._mj[ 0U ] );
    };

    Contact* contacts = manifold._contacts;

    for ( size_t i = 0U; i < manifold._contactCount; ++i )
        solve ( contacts[ i ] );

    GXVec6 vw2 {};
    vw2.Sum ( vwDynamic, vwDelta );
    bodyDynamic.SetVelocities ( vw2 );
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
    GXVec6 vw2A {};
    vw2A.Sum ( vw1A, lambda, data._mj[ 0U ] );

    GXVec6 vw2B {};
    vw2B.Sum ( vw1B, lambda, data._mj[ 1U ] );

    bodyA.SetVelocities ( vw2A );
    bodyB.SetVelocities ( vw2B );
}

void VelocitySolver::UpdateVelocitySingle ( RigidBody &body,
    VelocitySolverData const &data,
    GXVec6 const &vw1,
    float lambda
) noexcept
{
    GXVec6 vw2 {};
    vw2.Sum ( vw1, lambda, data._mj[ 0U ] );
    body.SetVelocities ( vw2 );
}

} // namespace android_vulkan
