#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <physics.hpp>
#include <contact_detector.hpp>
#include <logger.hpp>
#include <trace.hpp>
#include <velocity_solver.hpp>


namespace android_vulkan {

namespace {

constexpr uint16_t STEPS_PER_SECOND = 60U;
constexpr float DEFAULT_TIME_SPEED = 1.0F;
constexpr float FIXED_TIME_STEP = DEFAULT_TIME_SPEED / static_cast<float> ( STEPS_PER_SECOND );
constexpr float FIXED_TIME_STEP_INVERSE = 1.0F / FIXED_TIME_STEP;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

Physics::Physics () noexcept:
    _fixedTimeStep ( FIXED_TIME_STEP ),
    _fixedTimeStepInverse ( FIXED_TIME_STEP_INVERSE ),
    _timeSpeed ( DEFAULT_TIME_SPEED )
{
    // NOTHING
}

[[maybe_unused]] bool Physics::AddGlobalForce ( GlobalForceRef const &globalForce ) noexcept
{
    std::lock_guard const lock ( _mutex );
    auto const result = _globalForces.insert ( globalForce );

    if ( result.second )
        return true;

    LogError ( "Physics::AddGlobalForce - Can't insert global force. Same one presents already." );
    return false;
}

[[maybe_unused]] bool Physics::RemoveGlobalForce ( GlobalForceRef const &globalForce ) noexcept
{
    std::lock_guard const lock ( _mutex );
    auto const result = _globalForces.erase ( globalForce );

    if ( result > 0U )
        return true;

    LogError ( "Physics::RemoveGlobalForce - Can't find global force." );
    return false;
}

[[maybe_unused]] bool Physics::AddRigidBody ( RigidBodyRef const &rigidBody ) noexcept
{
    std::lock_guard const lock ( _mutex );
    RigidBody &body = *rigidBody.get ();

    if ( !body.HasShape() )
    {
        LogError ( "Physics::AddRigidBody - Can't insert rigid body. The rigid body does not have a shape." );
        return false;
    }

    auto const result = body.IsKinematic () ? _kinematics.insert ( rigidBody ) : _dynamics.insert ( rigidBody );

    if ( !result.second )
    {
        LogError ( "Physics::AddRigidBody - Can't insert rigid body. Same one presents already." );
        return false;
    }

    body.OnRegister ( *this );
    ResolveIntegrationType ( body );
    return true;
}

[[maybe_unused]] bool Physics::RemoveRigidBody ( RigidBodyRef const &rigidBody ) noexcept
{
    std::lock_guard const lock ( _mutex );

    RigidBody &body = *rigidBody.get ();
    auto const result = body.IsKinematic () ? _kinematics.erase ( rigidBody ) : _dynamics.erase ( rigidBody );

    if ( result > 0U )
    {
        body.OnUnregister ();
        return true;
    }

    LogError ( "Physics::RemoveRigidBody - Can't find the rigid body." );
    return false;
}

std::vector<ContactManifold> const &Physics::GetContactManifolds () const noexcept
{
    return _contactManager.GetContactManifolds ();
}

[[maybe_unused]] float Physics::GetTimeSpeed () const noexcept
{
    return _timeSpeed;
}

[[maybe_unused]] void Physics::SetTimeSpeed ( float speed ) noexcept
{
    AV_ASSERT ( speed != 0.0F )
    _timeSpeed = speed;
    _fixedTimeStep = FIXED_TIME_STEP * speed;
    _fixedTimeStepInverse = 1.0F / _fixedTimeStep;
}

bool Physics::IsPaused () const noexcept
{
    return _isPause;
}

void Physics::OnIntegrationTypeChanged ( RigidBody &rigidBody ) noexcept
{
    std::lock_guard const lock ( _mutex );
    ResolveIntegrationType ( rigidBody );
}

void Physics::Pause () noexcept
{
    _isPause = true;
}

void Physics::PenetrationTest ( std::vector<Penetration> &result,
    EPA &epa,
    ShapeRef const &shape,
    uint32_t groups
) const noexcept
{
    result.clear ();
    Shape const &s = *shape;

    auto check = [ &result, &epa, &s, groups ] ( std::unordered_set<RigidBodyRef> const &set ) noexcept {
        GJK gjk {};

        for ( auto &body : set )
        {
            Shape const &bodyShape = body->GetShape ();
            gjk.Reset ();

            if ( !( bodyShape.GetCollisionGroups () & groups ) || !gjk.Run ( bodyShape, s ) )
                continue;

            epa.Reset ();

            if ( !epa.Run ( gjk.GetSimplex (), bodyShape, s ) )
                continue;

            result.emplace_back (
                Penetration {
                    ._body = body,
                    ._depth = epa.GetDepth (),
                    ._normal = epa.GetNormal ()
                }
            );
        }
    };

    check ( _kinematics );
    check ( _dynamics );
}

bool Physics::Raycast ( RaycastResult &result, GXVec3 const &from, GXVec3 const &to, uint32_t groups ) const noexcept
{
    float dist = std::numeric_limits<float>::max ();

    bool isHit = false;
    RayCaster rayCaster {};
    RaycastResult current {};

    auto check = [ & ] ( std::unordered_set<RigidBodyRef> const &set ) noexcept {
        for ( auto &body : set )
        {
            Shape const &shape = body->GetShape ();

            if ( !( shape.GetCollisionGroups () & groups ) || !rayCaster.Run ( current, from, to, shape ) )
                continue;

            float const d = current._point.SquaredDistance ( from );

            if ( d >= dist )
                continue;

            dist = d;
            result._point = current._point;
            result._normal = current._normal;
            result._body = body;
            isHit = true;
        }
    };

    check ( _kinematics );
    check ( _dynamics );

    return isHit;
}

void Physics::Reset () noexcept
{
    std::lock_guard const lock ( _mutex );

    _contactManager.Reset ();
    _dynamics.clear ();
    _globalForces.clear ();
    _kinematics.clear ();
}

void Physics::Resume () noexcept
{
    if ( !_isPause )
        return;

    _accumulator = 0.0F;
    _isPause = false;
}

void Physics::Simulate ( float deltaTime ) noexcept
{
    AV_TRACE ( "Physics simulation" )

    if ( _isPause )
    {
        if ( _debugRun )
        {
            CollectContacts ();
            _debugRun = false;
        }

        return;
    }

    std::lock_guard const lock ( _mutex );
    _accumulator += deltaTime * _timeSpeed;

    while ( _accumulator >= _fixedTimeStep )
    {
        CollectContacts ();
        VelocitySolver::Run ( _contactManager, _fixedTimeStepInverse );
        Integrate ();

        _accumulator -= _fixedTimeStep;
    }
}

void Physics::SweepTest ( std::vector<RigidBodyRef> &result, ShapeRef const &sweepShape, uint32_t groups ) noexcept
{
    result.clear ();

    auto check = [ &result, groups ] ( Shape const &sweep, std::unordered_set<RigidBodyRef> const &set ) noexcept {
        GJK gjk {};

        for ( auto &body : set )
        {
            Shape const &bodyShape = body->GetShape ();
            gjk.Reset ();

            if ( ( bodyShape.GetCollisionGroups () & groups ) && gjk.Run ( sweep, bodyShape ) )
            {
                result.emplace_back ( body );
            }
        }
    };

    Shape const &sweep = *sweepShape;
    check ( sweep, _kinematics );
    check ( sweep, _dynamics );
}

void Physics::OnDebugRun () noexcept
{
    _debugRun = true;
}

void Physics::CollectContacts () noexcept
{
    _contactManager.Reset ();
    auto end = _dynamics.end ();

    for ( auto i = _dynamics.begin (); i != end; ++i )
    {
        for ( auto &kinematic : _kinematics )
            _contactDetector.Check ( _contactManager, *i, kinematic );

        for ( auto j = i; ++j != end; )
        {
            _contactDetector.Check ( _contactManager, *i, *j );
        }
    }
}

void Physics::Integrate () noexcept
{
    for ( auto &kinematic : _kinematics )
        kinematic->UpdatePositionAndRotation ( _fixedTimeStep );

    for ( auto &dynamic : _dynamics )
    {
        for ( auto const &globalForce : _globalForces )
            globalForce->Apply ( dynamic );

        dynamic->Integrate ( _fixedTimeStep );
    }
}

void Physics::ResolveIntegrationType ( RigidBody &rigidBody ) noexcept
{
    auto resolve = [ & ] ( auto &targetSet, auto &otherSet ) noexcept {
        auto end = otherSet.end ();

        auto findResult = std::find_if ( otherSet.begin (),
            end,
            [ & ] ( RigidBodyRef const &body ) noexcept -> bool {
                return body.get () == &rigidBody;
            }
        );

        if ( findResult == end )
            return;

        targetSet.insert ( *findResult );
        otherSet.erase ( findResult );
    };

    if ( rigidBody.IsKinematic () )
    {
        resolve ( _kinematics, _dynamics );
        return;
    }

    resolve ( _dynamics, _kinematics );
}

} // namespace android_vulkan
