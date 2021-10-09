#include <manifold_solver_x4.h>
#include <velocity_solver.h>


namespace android_vulkan {

constexpr static float const STABILIZATION_FACTOR = 0.4F;
constexpr static float const PENETRATION_SLOPE = 5.0e-4F;
constexpr static float const RESTITUTION_SLOPE = 5.0e-1F;

static_assert ( STABILIZATION_FACTOR >= 0.0F && STABILIZATION_FACTOR <= 1.0F,
    "The stabilization factor must be in range [0.0F, 1.0F]" );

[[maybe_unused]] void ManifoldSolverX4::PreparePair ( ContactManifold &manifold, float fixedTimeStepInverse ) noexcept
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

    GXVec3 dV {};
    dV.Subtract ( bodyB.GetVelocityLinear (), bodyA.GetVelocityLinear () );

    float const gamma = -STABILIZATION_FACTOR * fixedTimeStepInverse;

    Contact* contacts = manifold._contacts;
    GXVec4& b = manifold._b;

    for ( uint8_t i = 0U; i < 4U; ++i )
    {
        Contact& contact = contacts[ i ];

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

        GXVec3 const& normal = contact._normal;

        b._data[ i ] = gamma * std::max ( contact._penetration - PENETRATION_SLOPE, 0.0F ) +
            contact._restitution * std::max ( vClosing.DotProduct ( normal ) - RESTITUTION_SLOPE, 0.0F );

        // Note we changed operands to make negative vector.
        GXVec3 crossA {};
        crossA.CrossProduct ( normal, rA );

        GXVec3 crossB {};
        crossB.CrossProduct ( rB, normal );

        GXVec3 neg ( normal );
        neg.Reverse ();

        VelocitySolverData& data = contact._dataN;

        data._j[ 0U ].From ( neg, crossA );
        data._j[ 1U ].From ( normal, crossB );

        GXVec3 zeta {};
        invInertiaTensorA.MultiplyMatrixVector ( zeta, crossA );

        GXVec3 kappa {};
        invInertiaTensorB.MultiplyMatrixVector ( kappa, crossB );

        GXVec3 omega {};
        omega.Multiply ( neg, invMassA );

        GXVec3 iota {};
        iota.Multiply ( normal, invMassB );

        data._mj[ 0U ].From ( omega, zeta );
        data._mj[ 1U ].From ( iota, kappa );
    }

    VelocitySolverData const& c0 = contacts[ 0U ]._dataN;
    VelocitySolverData const& c1 = contacts[ 1U ]._dataN;
    VelocitySolverData const& c2 = contacts[ 2U ]._dataN;
    VelocitySolverData const& c3 = contacts[ 3U ]._dataN;

    GXMat4 m {};
    m._m[ 0U ][ 0U ] = c0._j[ 0U ].DotProduct ( c0._mj[ 0U ] ) + c0._j[ 1U ].DotProduct ( c0._mj[ 1U ] );
    m._m[ 0U ][ 1U ] = c0._j[ 0U ].DotProduct ( c1._mj[ 0U ] ) + c0._j[ 1U ].DotProduct ( c1._mj[ 1U ] );
    m._m[ 0U ][ 2U ] = c0._j[ 0U ].DotProduct ( c2._mj[ 0U ] ) + c0._j[ 1U ].DotProduct ( c2._mj[ 1U ] );
    m._m[ 0U ][ 3U ] = c0._j[ 0U ].DotProduct ( c3._mj[ 0U ] ) + c0._j[ 1U ].DotProduct ( c3._mj[ 1U ] );

    m._m[ 1U ][ 0U ] = c1._j[ 0U ].DotProduct ( c0._mj[ 0U ] ) + c1._j[ 1U ].DotProduct ( c0._mj[ 1U ] );
    m._m[ 1U ][ 1U ] = c1._j[ 0U ].DotProduct ( c1._mj[ 0U ] ) + c1._j[ 1U ].DotProduct ( c1._mj[ 1U ] );
    m._m[ 1U ][ 2U ] = c1._j[ 0U ].DotProduct ( c2._mj[ 0U ] ) + c1._j[ 1U ].DotProduct ( c2._mj[ 1U ] );
    m._m[ 1U ][ 3U ] = c1._j[ 0U ].DotProduct ( c3._mj[ 0U ] ) + c1._j[ 1U ].DotProduct ( c3._mj[ 1U ] );

    m._m[ 2U ][ 0U ] = c2._j[ 0U ].DotProduct ( c0._mj[ 0U ] ) + c2._j[ 1U ].DotProduct ( c0._mj[ 1U ] );
    m._m[ 2U ][ 1U ] = c2._j[ 0U ].DotProduct ( c1._mj[ 0U ] ) + c2._j[ 1U ].DotProduct ( c1._mj[ 1U ] );
    m._m[ 2U ][ 2U ] = c2._j[ 0U ].DotProduct ( c2._mj[ 0U ] ) + c2._j[ 1U ].DotProduct ( c2._mj[ 1U ] );
    m._m[ 2U ][ 3U ] = c2._j[ 0U ].DotProduct ( c3._mj[ 0U ] ) + c2._j[ 1U ].DotProduct ( c3._mj[ 1U ] );

    m._m[ 3U ][ 0U ] = c3._j[ 0U ].DotProduct ( c0._mj[ 0U ] ) + c3._j[ 1U ].DotProduct ( c0._mj[ 1U ] );
    m._m[ 3U ][ 1U ] = c3._j[ 0U ].DotProduct ( c1._mj[ 0U ] ) + c3._j[ 1U ].DotProduct ( c1._mj[ 1U ] );
    m._m[ 3U ][ 2U ] = c3._j[ 0U ].DotProduct ( c2._mj[ 0U ] ) + c3._j[ 1U ].DotProduct ( c2._mj[ 1U ] );
    m._m[ 3U ][ 3U ] = c3._j[ 0U ].DotProduct ( c3._mj[ 0U ] ) + c3._j[ 1U ].DotProduct ( c3._mj[ 1U ] );

    manifold._effectiveMass.Inverse ( m );

    // Apply warm start impulses.

    for ( uint8_t i = 0U; i < 4U; ++i )
    {
        Contact& contact = contacts[ i ];

        VelocitySolver::UpdateVelocityPair ( bodyA,
            bodyB,
            contact._dataN,
            bodyA.GetVelocities (),
            bodyB.GetVelocities (),
            contact._dataN._lambda
        );
    }
}

[[maybe_unused]] void ManifoldSolverX4::PrepareSingle ( ContactManifold &manifold, float fixedTimeStepInverse ) noexcept
{
    RigidBody& bodyDynamic = *manifold._bodyA.get ();
    RigidBody const& bodyKinematic = *manifold._bodyB.get ();

    GXVec3 const& cmDynamic = bodyDynamic.GetLocation ();
    GXVec3 const& wDynamic = bodyDynamic.GetVelocityAngular ();
    GXMat3 const& invInertiaTensor = bodyDynamic.GetInertiaTensorInverse ();
    float const invMass = bodyDynamic.GetMassInverse ();

    float const gamma = -STABILIZATION_FACTOR * fixedTimeStepInverse;

    GXVec3 const& cmKinematic = bodyKinematic.GetLocation ();
    GXVec3 const& vKinematic = bodyKinematic.GetVelocityLinear ();
    GXVec3 const& wKinematic = bodyKinematic.GetVelocityAngular ();

    GXVec3 dV {};
    dV.Subtract ( vKinematic, bodyDynamic.GetVelocityLinear () );

    Contact* contacts = manifold._contacts;
    GXVec4& b = manifold._b;

    for ( uint8_t i = 0U; i < 4U; ++i )
    {
        Contact& contact = contacts[ i ];

        GXVec3 rA {};
        rA.Subtract ( contact._pointA, cmDynamic );

        GXVec3 rB {};
        rB.Subtract ( contact._pointB, cmKinematic );

        // The operands are swapped to get negative vector. It will be used later for "b" calculation.
        GXVec3 vRotA {};
        vRotA.CrossProduct ( rA, wDynamic );

        GXVec3 vRotB {};
        vRotB.CrossProduct ( wKinematic, rB );

        GXVec3 alpha {};
        alpha.Sum ( vRotA, vRotB );

        GXVec3 vClosing {};
        vClosing.Sum ( dV, alpha );

        GXVec3 const& normal = contact._normal;

        b._data[ i ] = gamma * std::max ( contact._penetration - PENETRATION_SLOPE, 0.0F ) +
            contact._restitution * std::max ( vClosing.DotProduct ( normal ) - RESTITUTION_SLOPE, 0.0F );

        // Note we changed operands to make negative vector.
        GXVec3 crossA {};
        crossA.CrossProduct ( normal, rA );

        GXVec3 crossB {};
        crossB.CrossProduct ( rB, normal );

        GXVec3 neg ( normal );
        neg.Reverse ();

        VelocitySolverData& data = contact._dataN;

        data._j[ 0U ].From ( neg, crossA );
        data._j[ 1U ].From ( normal, crossB );

        GXVec3 beta {};
        invInertiaTensor.MultiplyMatrixVector ( beta, crossA );

        GXVec3 zeta {};
        zeta.Multiply ( neg, invMass );
        data._mj[ 0U ].From ( zeta, beta );
    }

    VelocitySolverData const& c0 = contacts[ 0U ]._dataN;
    VelocitySolverData const& c1 = contacts[ 1U ]._dataN;
    VelocitySolverData const& c2 = contacts[ 2U ]._dataN;
    VelocitySolverData const& c3 = contacts[ 3U ]._dataN;

    GXMat4 m {};
    m._m[ 0U ][ 0U ] = c0._j[ 0U ].DotProduct ( c0._mj[ 0U ] );
    m._m[ 0U ][ 1U ] = c0._j[ 0U ].DotProduct ( c1._mj[ 0U ] );
    m._m[ 0U ][ 2U ] = c0._j[ 0U ].DotProduct ( c2._mj[ 0U ] );
    m._m[ 0U ][ 3U ] = c0._j[ 0U ].DotProduct ( c3._mj[ 0U ] );

    m._m[ 1U ][ 0U ] = c1._j[ 0U ].DotProduct ( c0._mj[ 0U ] );
    m._m[ 1U ][ 1U ] = c1._j[ 0U ].DotProduct ( c1._mj[ 0U ] );
    m._m[ 1U ][ 2U ] = c1._j[ 0U ].DotProduct ( c2._mj[ 0U ] );
    m._m[ 1U ][ 3U ] = c1._j[ 0U ].DotProduct ( c3._mj[ 0U ] );

    m._m[ 2U ][ 0U ] = c2._j[ 0U ].DotProduct ( c0._mj[ 0U ] );
    m._m[ 2U ][ 1U ] = c2._j[ 0U ].DotProduct ( c1._mj[ 0U ] );
    m._m[ 2U ][ 2U ] = c2._j[ 0U ].DotProduct ( c2._mj[ 0U ] );
    m._m[ 2U ][ 3U ] = c2._j[ 0U ].DotProduct ( c3._mj[ 0U ] );

    m._m[ 3U ][ 0U ] = c3._j[ 0U ].DotProduct ( c0._mj[ 0U ] );
    m._m[ 3U ][ 1U ] = c3._j[ 0U ].DotProduct ( c1._mj[ 0U ] );
    m._m[ 3U ][ 2U ] = c3._j[ 0U ].DotProduct ( c2._mj[ 0U ] );
    m._m[ 3U ][ 3U ] = c3._j[ 0U ].DotProduct ( c3._mj[ 0U ] );

    manifold._effectiveMass.Inverse ( m );

    // Apply warm start impulses.

    for ( size_t i = 0U; i < manifold._contactCount; ++i )
    {
        VelocitySolverData& data = contacts[ i ]._dataN;
        VelocitySolver::UpdateVelocitySingle ( bodyDynamic, data, bodyDynamic.GetVelocities (), data._lambda );
    }
}

[[maybe_unused]] void ManifoldSolverX4::SolvePair ( ContactManifold &manifold ) noexcept
{
    RigidBody& bodyA = *manifold._bodyA.get ();
    RigidBody& bodyB = *manifold._bodyB.get ();

    GXVec6 const vw1A = bodyA.GetVelocities ();
    GXVec6 const vw1B = bodyB.GetVelocities ();

    Contact* contacts = manifold._contacts;
    VelocitySolverData& c0 = contacts[ 0U ]._dataN;
    VelocitySolverData& c1 = contacts[ 1U ]._dataN;
    VelocitySolverData& c2 = contacts[ 2U ]._dataN;
    VelocitySolverData& c3 = contacts[ 3U ]._dataN;

    GXVec4 jv {};
    jv._data[ 0U ] = c0._j[ 0U ].DotProduct ( vw1A ) + c0._j[ 1U ].DotProduct ( vw1B );
    jv._data[ 1U ] = c1._j[ 0U ].DotProduct ( vw1A ) + c1._j[ 1U ].DotProduct ( vw1B );
    jv._data[ 2U ] = c2._j[ 0U ].DotProduct ( vw1A ) + c2._j[ 1U ].DotProduct ( vw1B );
    jv._data[ 3U ] = c3._j[ 0U ].DotProduct ( vw1A ) + c3._j[ 1U ].DotProduct ( vw1B );

    GXVec4 alpha {};
    alpha.Sum ( jv, manifold._b );

    GXVec4 l {};
    manifold._effectiveMass.MultiplyMatrixVector ( l, alpha );

    GXVec4 const oldLambda ( c0._lambda, c1._lambda, c2._lambda, c3._lambda );

    GXVec4 rawLambda {};
    rawLambda.Sum ( oldLambda, l );

    GXVec4 const finalLambda (
        std::max ( 0.0F, rawLambda._data[ 0U ] ),
        std::max ( 0.0F, rawLambda._data[ 1U ] ),
        std::max ( 0.0F, rawLambda._data[ 2U ] ),
        std::max ( 0.0F, rawLambda._data[ 3U ] )
    );

    c0._lambda = finalLambda._data[ 0U ];
    c1._lambda = finalLambda._data[ 1U ];
    c2._lambda = finalLambda._data[ 2U ];
    c3._lambda = finalLambda._data[ 3U ];

    GXVec4 lambda {};
    lambda.Subtract ( finalLambda, oldLambda );

    GXVec6 dvwA0 {};
    dvwA0.Multiply ( c0._mj[ 0U ], lambda._data[ 0U ] );

    GXVec6 dvwB0 {};
    dvwB0.Multiply ( c0._mj[ 1U ], lambda._data[ 0U ] );

    GXVec6 dvwA2 {};
    dvwA0.Multiply ( c2._mj[ 0U ], lambda._data[ 2U ] );

    GXVec6 dvwB2 {};
    dvwB2.Multiply ( c2._mj[ 1U ], lambda._data[ 2U ] );

    GXVec6 dvwA01 {};
    dvwA01.Sum ( dvwA0, lambda._data[ 1U ], c1._mj[ 0U ] );

    GXVec6 dvwB01 {};
    dvwB01.Sum ( dvwB0, lambda._data[ 1U ], c1._mj[ 1U ] );

    GXVec6 dvA23 {};
    dvwA01.Sum ( dvwA2, lambda._data[ 3U ], c3._mj[ 0U ] );

    GXVec6 dvwB23 {};
    dvwB23.Sum ( dvwB2, lambda._data[ 3U ], c3._mj[ 1U ] );

    GXVec6 dvwA {};
    dvwA.Sum ( dvwA01, dvA23 );

    GXVec6 dvwB {};
    dvwB.Sum ( dvwB01, dvwB23 );

    GXVec6 vw2A {};
    vw2A.Sum ( vw1A, dvwA );

    GXVec6 vw2B {};
    vw2B.Sum ( vw1B, dvwB );

    bodyA.SetVelocities ( vw2A );
    bodyB.SetVelocities ( vw2B );
}

[[maybe_unused]] void ManifoldSolverX4::SolveSingle ( ContactManifold &manifold ) noexcept
{
    RigidBody& bodyDynamic = *manifold._bodyA.get ();
    RigidBody const& bodyKinematic = *manifold._bodyB.get ();

    GXVec6 const vw1Dynamic = bodyDynamic.GetVelocities ();
    GXVec6 const vwKinematic = bodyKinematic.GetVelocities ();

    Contact* contacts = manifold._contacts;
    VelocitySolverData& c0 = contacts[ 0U ]._dataN;
    VelocitySolverData& c1 = contacts[ 1U ]._dataN;
    VelocitySolverData& c2 = contacts[ 2U ]._dataN;
    VelocitySolverData& c3 = contacts[ 3U ]._dataN;

    GXVec4 jv {};
    jv._data[ 0U ] = c0._j[ 0U ].DotProduct ( vw1Dynamic ) + c0._j[ 1U ].DotProduct ( vwKinematic );
    jv._data[ 1U ] = c1._j[ 0U ].DotProduct ( vw1Dynamic ) + c1._j[ 1U ].DotProduct ( vwKinematic );
    jv._data[ 2U ] = c2._j[ 0U ].DotProduct ( vw1Dynamic ) + c2._j[ 1U ].DotProduct ( vwKinematic );
    jv._data[ 3U ] = c3._j[ 0U ].DotProduct ( vw1Dynamic ) + c3._j[ 1U ].DotProduct ( vwKinematic );

    GXVec4 alpha {};
    alpha.Sum ( jv, manifold._b );

    GXVec4 l {};
    manifold._effectiveMass.MultiplyMatrixVector ( l, alpha );

    GXVec4 const oldLambda ( c0._lambda, c1._lambda, c2._lambda, c3._lambda );

    GXVec4 rawLambda {};
    rawLambda.Sum ( oldLambda, l );

    GXVec4 const finalLambda (
        std::max ( 0.0F, rawLambda._data[ 0U ] ),
        std::max ( 0.0F, rawLambda._data[ 1U ] ),
        std::max ( 0.0F, rawLambda._data[ 2U ] ),
        std::max ( 0.0F, rawLambda._data[ 3U ] )
    );

    c0._lambda = finalLambda._data[ 0U ];
    c1._lambda = finalLambda._data[ 1U ];
    c2._lambda = finalLambda._data[ 2U ];
    c3._lambda = finalLambda._data[ 3U ];

    GXVec4 lambda {};
    lambda.Subtract ( finalLambda, oldLambda );

    GXVec6 dvwDynamic0 {};
    dvwDynamic0.Multiply ( c0._mj[ 0U ], lambda._data[ 0U ] );

    GXVec6 dvwDynamic2 {};
    dvwDynamic0.Multiply ( c2._mj[ 0U ], lambda._data[ 2U ] );

    GXVec6 dvwDynamic01 {};
    dvwDynamic01.Sum ( dvwDynamic0, lambda._data[ 1U ], c1._mj[ 0U ] );

    GXVec6 dvwDynamic23 {};
    dvwDynamic01.Sum ( dvwDynamic2, lambda._data[ 3U ], c3._mj[ 0U ] );

    GXVec6 dvwDynamic {};
    dvwDynamic.Sum ( dvwDynamic01, dvwDynamic23 );

    GXVec6 vw2Dynamic {};
    vw2Dynamic.Sum ( vw1Dynamic, dvwDynamic );

    bodyDynamic.SetVelocities ( vw2Dynamic );
}

} // namespace android_vulkan
