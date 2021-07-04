#include <rigid_body.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

constexpr static float const DEFAULT_DAMPING_ANGULAR = 0.8F;
constexpr static float const DEFAULT_DAMPING_LINEAR = 0.8F;

constexpr static float const DEFAULT_FRICTION = 0.4F;

constexpr static GXVec3 const DEFAULT_LOCATION ( 0.0F, -777.777F, 0.0F );

constexpr static float const DEFAULT_MASS = 1.0F;
constexpr static float const DEFAULT_MASS_INVERSE = 1.0F / DEFAULT_MASS;

constexpr static float const DEFAULT_RESTITUTION = 0.85F;

constexpr static GXVec3 const DEFAULT_ROTATION_AXIS ( 0.0F, 0.0F, 1.0F );

constexpr static float const LOCATION_SLEEP_THRESHOLD = 2.0e-5F;
constexpr static float const ROTATION_SLEEP_THRESHOLD = 1.5e-5F;
constexpr static float const SLEEP_TIMEOUT = 0.2F;

constexpr static GXVec3 const ZERO ( 0.0F, 0.0F, 0.0F );

RigidBody::RigidBody () noexcept:
    _dampingAngular ( DEFAULT_DAMPING_ANGULAR ),
    _dampingLinear ( DEFAULT_DAMPING_LINEAR ),
    _friction ( DEFAULT_FRICTION ),
    _inertiaTensorInverse {},
    _isAwake ( true ),
    _isCanSleep ( true ),
    _isKinematic ( false ),
    _location ( DEFAULT_LOCATION ),
    _locationBefore ( DEFAULT_LOCATION ),
    _mass ( DEFAULT_MASS ),
    _massInverse ( DEFAULT_MASS_INVERSE ),
    _restitution ( DEFAULT_RESTITUTION ),
    _rotation {},
    _rotationBefore {},
    _shape {},
    _sleepTimeout ( SLEEP_TIMEOUT ),
    _totalForce {},
    _totalTorque {},
    _transform {},
    _velocityAngular {},
    _velocityLinear {}
{
    _inertiaTensorInverse.Identity ();
    _rotation.FromAxisAngle ( DEFAULT_ROTATION_AXIS, 0.0F );
    _rotationBefore = _rotation;
}

[[maybe_unused]] void RigidBody::AddVelocityAngular ( GXVec3 const &velocity ) noexcept
{
    _velocityAngular.Sum ( _velocityAngular, velocity );
}

[[maybe_unused]] GXVec3 const& RigidBody::GetVelocityAngular () const noexcept
{
    return _velocityAngular;
}

[[maybe_unused]] void RigidBody::SetVelocityAngular ( GXVec3 const &velocity ) noexcept
{
    _velocityAngular = velocity;
}

void RigidBody::SetVelocityAngular ( float wx, float wy, float wz ) noexcept
{
    _velocityAngular._data[ 0U ] = wx;
    _velocityAngular._data[ 1U ] = wy;
    _velocityAngular._data[ 2U ] = wz;
}

[[maybe_unused]] void RigidBody::AddVelocityLinear ( GXVec3 const &velocity ) noexcept
{
    _velocityLinear.Sum ( _velocityLinear, velocity );
}

[[maybe_unused]] GXVec3 const& RigidBody::GetVelocityLinear () const noexcept
{
    return _velocityLinear;
}

[[maybe_unused]] void RigidBody::SetVelocityLinear ( GXVec3 const &velocity ) noexcept
{
    _velocityLinear = velocity;
}

void RigidBody::SetVelocityLinear ( float x, float y, float z ) noexcept
{
    _velocityLinear._data[ 0U ] = x;
    _velocityLinear._data[ 1U ] = y;
    _velocityLinear._data[ 2U ] = z;
}

[[maybe_unused]] void RigidBody::AddForce ( GXVec3 const &force, GXVec3 const &point ) noexcept
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
    _isAwake = GX_TRUE;
}

[[maybe_unused]] void RigidBody::AddImpulse ( GXVec3 const &impulse, GXVec3 const &point ) noexcept
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

    _isAwake = GX_TRUE;
}

[[maybe_unused]] void RigidBody::DisableKinematic () noexcept
{
    _isKinematic = false;
}

[[maybe_unused]] void RigidBody::EnableKinematic () noexcept
{
    _isKinematic = true;
}

[[maybe_unused]] bool RigidBody::IsKinematic () const noexcept
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

[[maybe_unused]] void RigidBody::EnableSleep () noexcept
{
    _isCanSleep = true;
}

[[maybe_unused]] bool RigidBody::IsCanSleep () const noexcept
{
    return _isCanSleep;
}

[[maybe_unused]] float RigidBody::GetDampingAngular () const noexcept
{
    return _dampingAngular;
}

[[maybe_unused]] void RigidBody::SetDampingAngular ( float damping ) noexcept
{
    _dampingAngular = damping;
}

[[maybe_unused]] float RigidBody::GetDampingLinear () const noexcept
{
    return _dampingLinear;
}

[[maybe_unused]] void RigidBody::SetDampingLinear ( float damping ) noexcept
{
    _dampingLinear = damping;
}

float RigidBody::GetFriction () const noexcept
{
    return _friction;
}

[[maybe_unused]] void RigidBody::SetFriction ( float friction ) noexcept
{
    _friction = friction;
}

[[maybe_unused]] GXMat3 const& RigidBody::GetInertiaTensorInverse () const noexcept
{
    return _inertiaTensorInverse;
}

[[maybe_unused]] GXVec3 const& RigidBody::GetLocation () const noexcept
{
    return _location;
}

[[maybe_unused]] void RigidBody::SetLocation ( GXVec3 const &location ) noexcept
{
    _location = location;
    _transform.SetW ( _location );

    if ( !_shape )
        return;

    UpdateCacheData ();
}

[[maybe_unused]] void RigidBody::SetLocation ( float x, float y, float z ) noexcept
{
    _location._data[ 0U ] = x;
    _location._data[ 1U ] = y;
    _location._data[ 2U ] = z;

    _locationBefore = _location;
    _transform.SetW ( _location );

    if ( !_shape )
        return;

    UpdateCacheData ();
}

[[maybe_unused]] float RigidBody::GetMass () const noexcept
{
    return _mass;
}

float RigidBody::GetMassInverse () const noexcept
{
    return _massInverse;
}

[[maybe_unused]] void RigidBody::SetMass ( float mass ) noexcept
{
    _mass = mass;
    _massInverse = 1.0F / mass;

    if ( !_shape )
        return;

    _shape->CalculateInertiaTensor ( mass );
    UpdateCacheData ();
}

float RigidBody::GetRestitution () const noexcept
{
    return _restitution;
}

[[maybe_unused]] void RigidBody::SetRestitution ( float restitution ) noexcept
{
    _restitution = restitution;
}

[[maybe_unused]] GXQuat const& RigidBody::GetRotation () const noexcept
{
    return _rotation;
}

[[maybe_unused]] void RigidBody::SetRotation ( GXQuat const &rotation ) noexcept
{
    _rotation = rotation;
    _rotationBefore = rotation;
    _transform.From ( _rotation, _location );

    if ( !_shape )
        return;

    UpdateCacheData ();
}

[[maybe_unused]] Shape& RigidBody::GetShape () noexcept
{
    assert ( _shape );
    return *_shape;
}

[[nodiscard]] bool RigidBody::HasShape () const noexcept
{
    return static_cast<bool> ( _shape );
}

[[maybe_unused]] void RigidBody::SetShape ( ShapeRef &shape ) noexcept
{
    _shape = shape;

    if ( !_shape )
        return;

    _shape->CalculateInertiaTensor ( _mass );
    UpdateCacheData ();
}

[[maybe_unused]] GXVec3 const& RigidBody::GetTotalForce () const noexcept
{
    return _totalForce;
}

[[maybe_unused]] GXVec3 const& RigidBody::GetTotalTorque () const noexcept
{
    return _totalTorque;
}

[[maybe_unused]] GXMat4 const& RigidBody::GetTransform () const noexcept
{
    return _transform;
}

void RigidBody::Integrate ( float deltaTime ) noexcept
{
    if ( _isKinematic )
    {
        IntegrateAsKinematic ( deltaTime );
        return;
    }

    IntegrateAsDynamic ( deltaTime );
}

[[maybe_unused]] bool RigidBody::IsAwake () const noexcept
{
    return _isAwake;
}

void RigidBody::ResetAccumulators ()
{
    _totalForce = ZERO;
    _totalTorque = ZERO;
}

void RigidBody::RunSleepLogic ( float deltaTime ) noexcept
{
    if ( !_isCanSleep )
        return;

    GXVec3 locationDiff {};
    locationDiff.Subtract ( _location, _locationBefore );

    if ( locationDiff.SquaredLength () > LOCATION_SLEEP_THRESHOLD )
    {
        _sleepTimeout = 0.0F;
        return;
    }

    GXQuat rotationDiff {};
    rotationDiff.Substract ( _rotation, _rotationBefore );

    GXVec4 const alpha ( rotationDiff._data[ 0U ],
        rotationDiff._data[ 1U ],
        rotationDiff._data[ 2U ],
        rotationDiff._data[ 3U ]
    );

    if ( alpha.SquaredLength () > ROTATION_SLEEP_THRESHOLD )
    {
        _sleepTimeout = 0.0F;
        return;
    }

    _sleepTimeout += deltaTime;

    if ( _sleepTimeout < SLEEP_TIMEOUT )
        return;

    SetSleep ();
}

void RigidBody::SetAwake () noexcept
{
    _isAwake = true;
    _sleepTimeout = 0.0F;
}

void RigidBody::SetSleep () noexcept
{
    _isAwake = false;
    _velocityAngular = ZERO;
    _velocityLinear = ZERO;
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

void RigidBody::IntegrateAsDynamic ( float deltaTime ) noexcept
{
    if ( !_isAwake || !_shape )
        return;

    GXVec3 accelerationLinear {};
    accelerationLinear.Multiply ( _totalForce, _massInverse );
    _velocityLinear.Sum ( _velocityLinear, deltaTime, accelerationLinear );

    GXVec3 accelerationAngular {};
    _inertiaTensorInverse.MultiplyMatrixVector ( accelerationAngular, _totalTorque );
    _velocityAngular.Sum ( _velocityAngular, deltaTime, accelerationAngular );

    _velocityLinear.Multiply ( _velocityLinear, std::pow ( _dampingLinear, deltaTime ) );
    _velocityAngular.Multiply ( _velocityAngular, std::pow ( _dampingAngular, deltaTime ) );

    _locationBefore = _location;
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
    beta.Normalize ();

    _rotationBefore = _rotation;
    _rotation = beta;

    UpdateCacheData ();
    RunSleepLogic ( deltaTime );
}

void RigidBody::IntegrateAsKinematic ( float deltaTime ) noexcept
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
    beta.Normalize ();

    _rotationBefore = _rotation;
    _rotation = beta;

    UpdateCacheData ();
}

} // namespace android_vulkan
