#include <rigid_body.h>
#include <logger.h>
#include <physics.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

constexpr static float const DEFAULT_DAMPING_ANGULAR = 0.85F;
constexpr static float const DEFAULT_DAMPING_LINEAR = 0.85F;

constexpr static GXVec3 const DEFAULT_LOCATION ( 0.0F, -777.777F, 0.0F );

constexpr static float const DEFAULT_MASS = 1.0F;
constexpr static float const DEFAULT_MASS_INVERSE = 1.0F / DEFAULT_MASS;

constexpr static GXVec3 const DEFAULT_ROTATION_AXIS ( 0.0F, 0.0F, 1.0F );

constexpr static float const LOCATION_SLEEP_THRESHOLD = 8.0e-1F;
constexpr static float const ROTATION_SLEEP_THRESHOLD = 1.0e+0F;
constexpr static float const SLEEP_TIMEOUT = 2.0F;

constexpr static GXVec3 const ZERO ( 0.0F, 0.0F, 0.0F );

//----------------------------------------------------------------------------------------------------------------------

std::mutex RigidBody::_mutex {};

RigidBody::RigidBody () noexcept:
    _context ( nullptr ),
    _dampingAngular ( DEFAULT_DAMPING_ANGULAR ),
    _dampingLinear ( DEFAULT_DAMPING_LINEAR ),
    _forceAwake ( false ),
    _inertiaTensorInverse {},
    _isAwake ( true ),
    _isCanSleep ( true ),
    _isKinematic ( false ),
    _location ( DEFAULT_LOCATION ),
    _mass ( DEFAULT_MASS ),
    _massInverse ( DEFAULT_MASS_INVERSE ),
    _physics ( nullptr ),
    _rotation {},
    _shape {},
    _sleepTimeout ( 0.0F ),
    _totalForce {},
    _totalTorque {},
    _tag {},
    _transform {},
    _velocityAngular {},
    _velocityLinear {}
{
    _inertiaTensorInverse.Identity ();
    _rotation.FromAxisAngle ( DEFAULT_ROTATION_AXIS, 0.0F );
}

[[maybe_unused]] void RigidBody::AddVelocityAngular ( GXVec3 const &velocity, bool forceAwake ) noexcept
{
    _velocityAngular.Sum ( _velocityAngular, velocity );
    _forceAwake |= forceAwake;
}

GXVec3 const& RigidBody::GetVelocityAngular () const noexcept
{
    return _velocityAngular;
}

[[maybe_unused]] void RigidBody::SetVelocityAngular ( GXVec3 const &velocity, bool forceAwake ) noexcept
{
    _velocityAngular = velocity;
    _forceAwake |= forceAwake;
}

[[maybe_unused]] void RigidBody::SetVelocityAngular ( float wx, float wy, float wz, bool forceAwake ) noexcept
{
    _velocityAngular._data[ 0U ] = wx;
    _velocityAngular._data[ 1U ] = wy;
    _velocityAngular._data[ 2U ] = wz;

    _forceAwake |= forceAwake;
}

[[maybe_unused]] void RigidBody::AddVelocityLinear ( GXVec3 const &velocity, bool forceAwake ) noexcept
{
    _velocityLinear.Sum ( _velocityLinear, velocity );
    _forceAwake |= forceAwake;
}

GXVec3 const& RigidBody::GetVelocityLinear () const noexcept
{
    return _velocityLinear;
}

void RigidBody::SetVelocityLinear ( GXVec3 const &velocity, bool forceAwake ) noexcept
{
    _velocityLinear = velocity;
    _forceAwake |= forceAwake;
}

[[maybe_unused]] void RigidBody::SetVelocityLinear ( float x, float y, float z, bool forceAwake ) noexcept
{
    _velocityLinear._data[ 0U ] = x;
    _velocityLinear._data[ 1U ] = y;
    _velocityLinear._data[ 2U ] = z;

    _forceAwake |= forceAwake;
}

void RigidBody::SetVelocities ( GXVec6 const &velocities ) noexcept
{
    std::memcpy ( _velocityLinear._data, velocities._data, sizeof ( _velocityLinear ) );
    std::memcpy ( _velocityAngular._data, velocities._data + 3U, sizeof ( _velocityAngular ) );
}

GXVec6 RigidBody::GetVelocities () const noexcept
{
    return GXVec6 ( _velocityLinear, _velocityAngular );
}

void RigidBody::AddForce ( GXVec3 const &force, GXVec3 const &point, bool forceAwake ) noexcept
{
    if ( _isKinematic )
        return;

    // Note: Force is applied for prolonged period of time. It's not the same as impulse which is simultaneous event.
    // So from this point of view it makes sense to accumulate total force independent from apply point location.
    // The math principles is described here:
    // https://www.bzarg.com/p/3d-rotational-dynamics-1-the-basics/
    _totalForce.Sum ( _totalForce, force );

    GXVec3 leverArm {};
    leverArm.Subtract ( point, _location );

    GXVec3 torque {};
    torque.CrossProduct ( leverArm, force );

    _totalTorque.Sum ( _totalTorque, torque );
    _forceAwake |= forceAwake;
}

[[maybe_unused]] void RigidBody::AddImpulse ( GXVec3 const &impulse, GXVec3 const &point, bool forceAwake ) noexcept
{
    if ( _isKinematic )
        return;

    // Note: The implementation follows the same ideas as
    // PhysX:
    // https://github.com/NVIDIAGameWorks/PhysX/blob/93c6dd21b545605185f2febc8eeacebe49a99479/physx/source/physxextensions/src/ExtRigidBodyExt.cpp#L474
    // and Bullet:
    // https://github.com/bulletphysics/bullet3/blob/0e124cb2f103c40de4afac6c100b7e8e1f5d9e15/src/BulletDynamics/Dynamics/btRigidBody.h#L335
    //
    // So for linear velocity there is no any difference where hit point was. This is a counterintuitively.
    // But from other hand there is no any sources which disprove that.
    GXVec3 deltaVelocityLinear {};
    deltaVelocityLinear.Multiply ( impulse, _massInverse );
    _velocityLinear.Sum ( _velocityLinear, deltaVelocityLinear );

    GXVec3 levelArm {};
    levelArm.Subtract ( point, _location );

    GXVec3 angularImpulse {};
    angularImpulse.CrossProduct ( levelArm, impulse );

    GXVec3 deltaVelocityAngular {};
    _inertiaTensorInverse.MultiplyMatrixVector ( deltaVelocityAngular, angularImpulse );
    _velocityAngular.Sum ( _velocityAngular, deltaVelocityAngular );

    _forceAwake |= forceAwake;
}

void RigidBody::DisableKinematic ( bool forceAwake ) noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );

    _isKinematic = false;
    _forceAwake |= forceAwake;

    if ( _physics )
    {
        _physics->OnIntegrationTypeChanged ( *this );
    }
}

void RigidBody::EnableKinematic () noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );
    _isKinematic = true;

    if ( _physics )
    {
        _physics->OnIntegrationTypeChanged ( *this );
    }
}

bool RigidBody::IsKinematic () const noexcept
{
    return _isKinematic;
}

[[maybe_unused]] void RigidBody::DisableSleep () noexcept
{
    _isCanSleep = false;

    if ( _isAwake )
        return;

    SetAwake ();
}

void RigidBody::EnableSleep () noexcept
{
    _isCanSleep = true;
}

[[maybe_unused]] bool RigidBody::IsCanSleep () const noexcept
{
    return _isCanSleep;
}

[[nodiscard]] RigidBody::Context RigidBody::GetContext () const noexcept
{
    return _context;
}

void RigidBody::SetContext ( Context context ) noexcept
{
    _context = context;
}

[[maybe_unused]] float RigidBody::GetDampingAngular () const noexcept
{
    return _dampingAngular;
}

void RigidBody::SetDampingAngular ( float damping ) noexcept
{
    _dampingAngular = damping;
}

[[maybe_unused]] float RigidBody::GetDampingLinear () const noexcept
{
    return _dampingLinear;
}

void RigidBody::SetDampingLinear ( float damping ) noexcept
{
    _dampingLinear = damping;
}

GXMat3 const& RigidBody::GetInertiaTensorInverse () const noexcept
{
    return _inertiaTensorInverse;
}

GXVec3 const& RigidBody::GetLocation () const noexcept
{
    return _location;
}

void RigidBody::SetLocation ( GXVec3 const &location, bool forceAwake ) noexcept
{
    _location = location;
    _transform.SetW ( _location );
    _forceAwake |= forceAwake;

    if ( !_shape )
        return;

    UpdateCacheData ();
}

void RigidBody::SetLocation ( float x, float y, float z, bool forceAwake ) noexcept
{
    _location._data[ 0U ] = x;
    _location._data[ 1U ] = y;
    _location._data[ 2U ] = z;

    _transform.SetW ( _location );
    _forceAwake |= forceAwake;

    if ( !_shape )
        return;

    UpdateCacheData ();
}

float RigidBody::GetMass () const noexcept
{
    return _mass;
}

float RigidBody::GetMassInverse () const noexcept
{
    return _massInverse;
}

void RigidBody::SetMass ( float mass, bool forceAwake ) noexcept
{
    _mass = mass;
    _massInverse = 1.0F / mass;
    _forceAwake |= forceAwake;

    if ( !_shape )
        return;

    _shape->CalculateInertiaTensor ( mass );
    UpdateCacheData ();
}

GXQuat const& RigidBody::GetRotation () const noexcept
{
    return _rotation;
}

void RigidBody::SetRotation ( GXQuat const &rotation, bool forceAwake ) noexcept
{
    _rotation = rotation;
    _transform.From ( _rotation, _location );
    _forceAwake |= forceAwake;

    if ( !_shape )
        return;

    UpdateCacheData ();
}

Shape& RigidBody::GetShape () noexcept
{
    assert ( _shape );
    return *_shape;
}

bool RigidBody::HasShape () const noexcept
{
    return static_cast<bool> ( _shape );
}

[[maybe_unused]] void RigidBody::SetShape ( ShapeRef &shape, bool forceAwake ) noexcept
{
    _shape = shape;
    _forceAwake |= forceAwake;

    if ( !_shape )
        return;

    _shape->CalculateInertiaTensor ( _mass );
    UpdateCacheData ();
}

[[maybe_unused]] std::string const& RigidBody::GetTag () const noexcept
{
    return _tag;
}

void RigidBody::SetTag ( std::string &&tag ) noexcept
{
    _tag = std::move ( tag );
}

[[maybe_unused]] GXVec3 const& RigidBody::GetTotalForce () const noexcept
{
    return _totalForce;
}

[[maybe_unused]] GXVec3 const& RigidBody::GetTotalTorque () const noexcept
{
    return _totalTorque;
}

GXMat4 const& RigidBody::GetTransform () const noexcept
{
    return _transform;
}

void RigidBody::Integrate ( float deltaTime ) noexcept
{
    if ( !_shape )
    {
        ResetAccumulators ();
        _forceAwake = false;
        return;
    }

    RunSleepLogic ( deltaTime );

    if ( !_isAwake && !_forceAwake )
    {
        ResetAccumulators ();
        return;
    }

    GXVec3 accelerationLinear {};
    accelerationLinear.Multiply ( _totalForce, _massInverse );
    _velocityLinear.Sum ( _velocityLinear, deltaTime, accelerationLinear );

    GXVec3 accelerationAngular {};
    _inertiaTensorInverse.MultiplyMatrixVector ( accelerationAngular, _totalTorque );
    _velocityAngular.Sum ( _velocityAngular, deltaTime, accelerationAngular );

    _velocityLinear.Multiply ( _velocityLinear, std::pow ( _dampingLinear, deltaTime ) );
    _velocityAngular.Multiply ( _velocityAngular, std::pow ( _dampingAngular, deltaTime ) );

    UpdatePositionAndRotation ( deltaTime );
    ResetAccumulators ();

    _forceAwake = false;
}

[[maybe_unused]] bool RigidBody::IsAwake () const noexcept
{
    return _isAwake;
}

void RigidBody::SetAwake () noexcept
{
    _isAwake = true;
    _sleepTimeout = 0.0F;
}

void RigidBody::OnRegister ( Physics &physics ) noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );
    _physics = &physics;
}

void RigidBody::OnUnregister () noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );
    _physics = nullptr;
}

void RigidBody::UpdatePositionAndRotation ( float deltaTime ) noexcept
{
    _location.Sum ( _location, deltaTime, _velocityLinear );

    // omega: angular velocity (direction is axis, magnitude is angle)
    // https://fgiesen.wordpress.com/2012/08/24/quaternion-differentiation/
    // https://www.ashwinnarayan.com/post/how-to-integrate-quaternions/
    // https://gafferongames.com/post/physics_in_3d/
    GXQuat alpha ( 0.0F, _velocityAngular._data[ 0U ], _velocityAngular._data[ 1U ], _velocityAngular._data[ 2U ] );
    alpha.Multiply ( alpha, 0.5F * deltaTime );
    alpha._data[ 0U ] = 1.0F;

    GXQuat beta {};
    beta.Multiply ( alpha, _rotation );
    _rotation = beta;

    UpdateCacheData ();
}

void RigidBody::ResetAccumulators () noexcept
{
    _totalForce = ZERO;
    _totalTorque = ZERO;
}

void RigidBody::RunSleepLogic ( float deltaTime ) noexcept
{
    if ( !_isCanSleep )
        return;

    constexpr float const linearThreshold = LOCATION_SLEEP_THRESHOLD * LOCATION_SLEEP_THRESHOLD;

    if ( _velocityLinear.SquaredLength () >= linearThreshold )
    {
        _isAwake = true;
        _sleepTimeout = 0.0F;
        return;
    }

    constexpr float const angularThreshold = ROTATION_SLEEP_THRESHOLD * ROTATION_SLEEP_THRESHOLD;

    if ( _velocityAngular.SquaredLength () >= angularThreshold )
    {
        _isAwake = true;
        _sleepTimeout = 0.0F;
        return;
    }

    _sleepTimeout += deltaTime;

    if ( _sleepTimeout > SLEEP_TIMEOUT )
    {
        _isAwake = false;
    }
}

void RigidBody::UpdateCacheData () noexcept
{
    _rotation.Normalize ();
    _transform.FromFast ( _rotation, _location );

    GXMat3 const alpha ( _transform );
    GXMat3 beta {};
    beta.Transpose ( alpha );

    GXMat3 gamma {};
    gamma.Multiply ( beta, _shape->GetInertiaTensorInverse () );
    _inertiaTensorInverse.Multiply ( gamma, alpha );

    _shape->UpdateCacheData ( _transform );
}

} // namespace android_vulkan
