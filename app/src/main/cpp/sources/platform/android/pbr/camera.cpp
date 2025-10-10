#include <precompiled_headers.hpp>
#include <gamepad.hpp>
#include <platform/android/pbr/camera.hpp>


namespace pbr {

namespace {

constexpr float ANGULAR_SPEED = 0.87F * GX_MATH_PI;
constexpr float MOVE_BOOST = 10.0F;
constexpr float MOVING_SPEED = 50.0F;
constexpr float STICK_DEAD_ZONE = 0.2F;
constexpr float TRIGGER_DEAD_ZONE = 0.2F;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

Camera::Camera () noexcept:
    _moveBoost ( 0.0F ),
    _movingSpeed ( MOVING_SPEED ),
    _pitch ( 0.0F ),
    _yaw ( 0.0F ),
    _moveSpeed ( 0.0F, 0.0F, 0.0F ),
    _angularSpeed ( 0.0F, 0.0F ),
    _local {},
    _projection {}
{
    _local.Identity ();
    _projection.Identity ();
}

void Camera::CaptureInput () noexcept
{
    android_vulkan::Gamepad &gamepad = android_vulkan::Gamepad::GetInstance ();

    gamepad.BindKey ( this, &Camera::OnADown, android_vulkan::eGamepadKey::A, android_vulkan::eButtonState::Down );
    gamepad.BindKey ( this, &Camera::OnAUp, android_vulkan::eGamepadKey::A, android_vulkan::eButtonState::Up );
    gamepad.BindKey ( this, &Camera::OnXDown, android_vulkan::eGamepadKey::X, android_vulkan::eButtonState::Down );
    gamepad.BindKey ( this, &Camera::OnXUp, android_vulkan::eGamepadKey::X, android_vulkan::eButtonState::Up );

    gamepad.BindLeftStick ( this, &Camera::OnLeftStick );
    gamepad.BindRightStick ( this, &Camera::OnRightStick );
    gamepad.BindRightTrigger ( this, &Camera::OnRightTrigger );
}

void Camera::ReleaseInput () noexcept
{
    android_vulkan::Gamepad &gamepad = android_vulkan::Gamepad::GetInstance ();

    gamepad.UnbindRightTrigger ();
    gamepad.UnbindRightStick ();
    gamepad.UnbindLeftStick ();

    gamepad.UnbindKey ( android_vulkan::eGamepadKey::X, android_vulkan::eButtonState::Up );
    gamepad.UnbindKey ( android_vulkan::eGamepadKey::X, android_vulkan::eButtonState::Down );
    gamepad.UnbindKey ( android_vulkan::eGamepadKey::A, android_vulkan::eButtonState::Up );
    gamepad.UnbindKey ( android_vulkan::eGamepadKey::A, android_vulkan::eButtonState::Down );

    constexpr GXVec3 stopMove ( 0.0F, 0.0F, 0.0F );
    _moveSpeed = stopMove;

    constexpr GXVec2 stopRotate ( 0.0F, 0.0F );
    _angularSpeed = stopRotate;
}

GXMat4 const &Camera::GetLocalMatrix() const noexcept
{
    return _local;
}

GXMat4 const &Camera::GetProjectionMatrix () const noexcept
{
    return _projection;
}

void Camera::SetLocation ( GXVec3 const &location ) noexcept
{
    _local._data[ 3U ][ 0U ] = location._data[ 0U ];
    _local._data[ 3U ][ 1U ] = location._data[ 1U ];
    _local._data[ 3U ][ 2U ] = location._data[ 2U ];
    _local.SetW ( location );
}

void Camera::SetMovingSpeed ( float speed ) noexcept
{
    _movingSpeed = speed;
}

void Camera::SetProjection ( float fieldOfViewRadians, float aspectRatio, float zNear, float zFar ) noexcept
{
    _projection.Perspective ( fieldOfViewRadians, aspectRatio, zNear, zFar );
}

void Camera::SetRotation ( float pitch, float yaw ) noexcept
{
    _pitch = pitch;
    _yaw = yaw;
}

void Camera::Update ( float deltaTime ) noexcept
{
    float const boost = _moveBoost;

    GXVec2 angular = _angularSpeed;
    GXVec3 move = _moveSpeed;

    constexpr auto deadZoneHandler = [] ( float value ) -> float {
        if ( std::abs ( value ) < STICK_DEAD_ZONE )
            return 0.0F;

        constexpr float scale = 1.0F / ( 1.0F - STICK_DEAD_ZONE );
        float const threshold = value > 0.0F ? -STICK_DEAD_ZONE : STICK_DEAD_ZONE;
        return scale * ( value + threshold );
    };

    angular._data[ 0U ] = deadZoneHandler ( angular._data[ 0U ] );
    angular._data[ 1U ] = deadZoneHandler ( angular._data[ 1U ] );
    move._data[ 0U ] = deadZoneHandler ( move._data[ 0U ] );
    move._data[ 1U ] = deadZoneHandler ( move._data[ 1U ] );

    float const deltaAngular = ANGULAR_SPEED * deltaTime;

    _yaw += deltaAngular * angular._data[ 0U ];

    while ( _yaw < -GX_MATH_PI )
        _yaw += GX_MATH_DOUBLE_PI;

    while ( _yaw > GX_MATH_PI )
        _yaw -= GX_MATH_DOUBLE_PI;

    _pitch = GXClampf ( _pitch + deltaAngular * angular._data[ 1U ], -GX_MATH_HALF_PI, GX_MATH_HALF_PI );

    GXMat4 rot {};
    rot.RotationXY ( _pitch, _yaw );

    GXVec3 right {};
    rot.GetX ( right );
    _local.SetX ( right );

    GXVec3 up {};
    rot.GetY ( up );
    _local.SetY ( up );

    GXVec3 forward {};
    rot.GetZ ( forward );
    _local.SetZ ( forward );

    constexpr float scale = 1.0F / ( 1.0F - TRIGGER_DEAD_ZONE );
    float const boostFactor = MOVE_BOOST * _movingSpeed;

    float const deltaLinear = boost > TRIGGER_DEAD_ZONE ?
        deltaTime * boostFactor * scale * ( boost - TRIGGER_DEAD_ZONE ):
        deltaTime * _movingSpeed;

    GXVec3 location {};
    _local.GetW ( location );

    location.Sum ( location, deltaLinear * move._data[ 0U ], forward );
    location.Sum ( location, deltaLinear * move._data[ 1U ], right );
    location.Sum ( location, deltaLinear * move._data[ 2U ], up );
    _local.SetW ( location );
}

void Camera::OnADown ( void* context ) noexcept
{
    auto &camera = *static_cast<Camera*> ( context );
    camera._moveSpeed._data[ 2U ] -= 1.0F;
}

void Camera::OnAUp ( void* context ) noexcept
{
    auto &camera = *static_cast<Camera*> ( context );
    camera._moveSpeed._data[ 2U ] += 1.0F;
}

void Camera::OnXDown ( void* context ) noexcept
{
    auto &camera = *static_cast<Camera*> ( context );
    camera._moveSpeed._data[ 2U ] += 1.0F;
}

void Camera::OnXUp ( void* context ) noexcept
{
    auto &camera = *static_cast<Camera*> ( context );
    camera._moveSpeed._data[ 2U ] -= 1.0F;
}

void Camera::OnLeftStick ( void* context, float horizontal, float vertical ) noexcept
{
    auto &camera = *static_cast<Camera*> ( context );
    camera._moveSpeed._data[ 0U ] = vertical;
    camera._moveSpeed._data[ 1U ] = horizontal;
}

void Camera::OnRightStick ( void* context, float horizontal, float vertical ) noexcept
{
    auto &camera = *static_cast<Camera*> ( context );
    camera._angularSpeed._data[ 0U ] = horizontal;
    camera._angularSpeed._data[ 1U ] = -vertical;
}

void Camera::OnRightTrigger ( void* context, float push ) noexcept
{
    auto &camera = *static_cast<Camera*> ( context );
    camera._moveBoost = push;
}

} // namespace pbr
