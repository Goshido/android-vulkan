#include <physics.h>
#include <contact_detector.h>
#include <logger.h>
#include <velocity_solver.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

constexpr static uint16_t const STEPS_PER_SECOND = 60U;
constexpr static float const DEFAULT_TIME_SPEED = 1.0F;
constexpr static float const FIXED_TIME_STEP = DEFAULT_TIME_SPEED / static_cast<float> ( STEPS_PER_SECOND );
constexpr static float const FIXED_TIME_STEP_INVERSE = 1.0F / FIXED_TIME_STEP;

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
    std::unique_lock<std::mutex> const lock ( _mutex );
    auto const result = _globalForces.insert ( globalForce );

    if ( result.second )
        return true;

    LogError ( "Physics::AddGlobalForce - Can't insert global force. Same one presents already." );
    return false;
}

[[maybe_unused]] bool Physics::RemoveGlobalForce ( GlobalForceRef const &globalForce ) noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );
    auto const result = _globalForces.erase ( globalForce );

    if ( result > 0U )
        return true;

    LogError ( "Physics::RemoveGlobalForce - Can't find global force." );
    return false;
}

[[maybe_unused]] bool Physics::AddRigidBody ( RigidBodyRef const &rigidBody ) noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );
    RigidBody& body = *rigidBody.get ();

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
    std::unique_lock<std::mutex> const lock ( _mutex );

    RigidBody& body = *rigidBody.get ();
    auto const result = body.IsKinematic () ? _kinematics.erase ( rigidBody ) : _dynamics.erase ( rigidBody );

    if ( result > 0U )
    {
        body.OnUnregister ();
        return true;
    }

    LogError ( "Physics::RemoveRigidBody - Can't find the rigid body." );
    return false;
}

std::vector<ContactManifold> const& Physics::GetContactManifolds () const noexcept
{
    return _contactManager.GetContactManifolds ();
}

[[maybe_unused]] float Physics::GetTimeSpeed () const noexcept
{
    return _timeSpeed;
}

[[maybe_unused]] void Physics::SetTimeSpeed ( float speed ) noexcept
{
    assert ( speed != 0.0F );
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
    std::unique_lock<std::mutex> const lock ( _mutex );
    ResolveIntegrationType ( rigidBody );
}

void Physics::Pause () noexcept
{
    _isPause = true;
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
    if ( _isPause )
    {
        if ( _debugRun )
        {
            CollectContacts ();
            _debugRun = false;
        }

        return;
    }

    std::unique_lock<std::mutex> const lock ( _mutex );
    _accumulator += deltaTime * _timeSpeed;

    while ( _accumulator >= _fixedTimeStep )
    {
        CollectContacts ();
        VelocitySolver::Run ( _contactManager, _fixedTimeStepInverse );
        Integrate ();

        _accumulator -= _fixedTimeStep;
    }
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
        for ( auto& kinematic : _kinematics )
            _contactDetector.Check ( _contactManager, *i, kinematic );

        for ( auto j = i; ++j != end; )
        {
            _contactDetector.Check ( _contactManager, *i, *j );
        }
    }
}

void Physics::Integrate () noexcept
{
    for ( auto& kinematic : _kinematics )
        kinematic->UpdatePositionAndRotation ( _fixedTimeStep );

    for ( auto& dynamic : _dynamics )
    {
        for ( auto const& globalForce : _globalForces )
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
