#include <physics.h>
#include <contact_detector.h>
#include <logger.h>


namespace android_vulkan {

constexpr static uint16_t const STEPS_PER_SECOND = 120U;
constexpr static float const FIXED_TIME_STEP = 1.0F / static_cast<float> ( STEPS_PER_SECOND );

Physics::Physics () noexcept:
    _accumulator ( 0.0F ),
    _contactDetector {},
    _contactManager {},
    _globalForces {},
    _isPause ( true ),
    _rigidBodies {}
{
    // NOTHING
}

[[maybe_unused]] bool Physics::AddGlobalForce ( GlobalForceRef const &globalForce ) noexcept
{
    auto const result = _globalForces.insert ( globalForce );

    if ( result.second )
        return true;

    LogError ( "Physics::AddGlobalForce - Can't insert global force. Same one presents already." );
    return false;
}

[[maybe_unused]] bool Physics::RemoveGlobalForce ( GlobalForceRef const &globalForce ) noexcept
{
    auto const result = _globalForces.erase ( globalForce );

    if ( result > 0U )
        return true;

    LogError ( "Physics::RemoveGlobalForce - Can't find global force." );
    return false;
}

[[maybe_unused]] bool Physics::AddRigidBody ( RigidBodyRef const &rigidBody ) noexcept
{
    if ( !rigidBody->HasShape() )
    {
        LogError ( "Physics::AddRigidBody - Can't insert rigid body. The rigid body does not have a shape." );
        return false;
    }

    auto const result = _rigidBodies.insert ( rigidBody );

    if ( result.second )
        return true;

    LogError ( "Physics::AddRigidBody - Can't insert rigid body. Same one presents already." );
    return false;
}

[[maybe_unused]] bool Physics::RemoveRigidBody ( RigidBodyRef const &rigidBody ) noexcept
{
    auto const result = _rigidBodies.erase ( rigidBody );

    if ( result > 0U )
        return true;

    LogError ( "Physics::RemoveRigidBody - Can't find the rigid body." );
    return false;
}

bool Physics::IsPaused () const noexcept
{
    return _isPause;
}

void Physics::Pause () noexcept
{
    _isPause = true;
}

void Physics::Resume () noexcept
{
    _isPause = false;
}

void Physics::Simulate ( float deltaTime ) noexcept
{
    if ( _isPause )
        return;

    _accumulator += deltaTime;

    while ( _accumulator >= FIXED_TIME_STEP )
    {
        Integrate ();
        CollectContacts ();

        // TODO collision response

        Prepare ();
        _accumulator -= FIXED_TIME_STEP;
    }

    // TODO
}

void Physics::CollectContacts () noexcept
{
    auto end = _rigidBodies.end ();

    for ( auto i = _rigidBodies.begin (); i != end; ++i )
    {
        for ( auto j = i; ++j != end; )
        {
            _contactDetector.Check ( _contactManager, *i, *j );
        }
    }
}

void Physics::Integrate () noexcept
{
    for ( auto& rigidBody : _rigidBodies )
    {
        for ( auto const& globalForce : _globalForces )
            globalForce->Apply ( rigidBody );

        rigidBody->Integrate ( FIXED_TIME_STEP );
    }
}

void Physics::Prepare () noexcept
{
    for ( auto& rigidBody : _rigidBodies )
        rigidBody->ResetAccumulators ();

    _contactManager.Reset ();
}

} // namespace android_vulkan
