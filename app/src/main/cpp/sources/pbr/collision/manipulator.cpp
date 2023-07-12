#include <pbr/collision/manipulator.h>
#include <gamepad.h>
#include <logger.h>


namespace pbr::collision {

constexpr static float const VELOCITY_ANGULAR = 1.0F;
constexpr static float const VELOCITY_LINEAR = 0.2F;

void Manipulator::Capture ( android_vulkan::RigidBodyRef &body ) noexcept
{
    _body = body;
    _pitch = 0;
    _roll = 0;

    android_vulkan::Gamepad &gamepad = android_vulkan::Gamepad::GetInstance ();

    gamepad.BindKey ( this,
        &Manipulator::OnBDown,
        android_vulkan::eGamepadKey::B,
        android_vulkan::eButtonState::Down
    );

    gamepad.BindKey ( this,
        &Manipulator::OnBUp,
        android_vulkan::eGamepadKey::B,
        android_vulkan::eButtonState::Up
    );

    gamepad.BindKey ( this,
        &Manipulator::OnDownDown,
        android_vulkan::eGamepadKey::Down,
        android_vulkan::eButtonState::Down
    );

    gamepad.BindKey ( this,
        &Manipulator::OnDownUp,
        android_vulkan::eGamepadKey::Down,
        android_vulkan::eButtonState::Up
    );

    gamepad.BindKey ( this,
        &Manipulator::OnLeftDown,
        android_vulkan::eGamepadKey::Left,
        android_vulkan::eButtonState::Down
    );

    gamepad.BindKey ( this,
        &Manipulator::OnLeftUp,
        android_vulkan::eGamepadKey::Left,
        android_vulkan::eButtonState::Up
    );

    gamepad.BindKey ( this,
        &Manipulator::OnRightDown,
        android_vulkan::eGamepadKey::Right,
        android_vulkan::eButtonState::Down
    );

    gamepad.BindKey ( this,
        &Manipulator::OnRightUp,
        android_vulkan::eGamepadKey::Right,
        android_vulkan::eButtonState::Up
    );

    gamepad.BindKey ( this,
        &Manipulator::OnUpDown,
        android_vulkan::eGamepadKey::Up,
        android_vulkan::eButtonState::Down
    );

    gamepad.BindKey ( this,
        &Manipulator::OnUpUp,
        android_vulkan::eGamepadKey::Up,
        android_vulkan::eButtonState::Up
    );

    gamepad.BindKey ( this,
        &Manipulator::OnYDown,
        android_vulkan::eGamepadKey::Y,
        android_vulkan::eButtonState::Down
    );

    gamepad.BindKey ( this,
        &Manipulator::OnYUp,
        android_vulkan::eGamepadKey::Y,
        android_vulkan::eButtonState::Up
    );
}

void Manipulator::Free () noexcept
{
    android_vulkan::Gamepad &gamepad = android_vulkan::Gamepad::GetInstance ();

    gamepad.UnbindKey ( android_vulkan::eGamepadKey::B, android_vulkan::eButtonState::Down );
    gamepad.UnbindKey ( android_vulkan::eGamepadKey::B, android_vulkan::eButtonState::Up );
    gamepad.UnbindKey ( android_vulkan::eGamepadKey::Down, android_vulkan::eButtonState::Down );
    gamepad.UnbindKey ( android_vulkan::eGamepadKey::Down, android_vulkan::eButtonState::Up );
    gamepad.UnbindKey ( android_vulkan::eGamepadKey::Left, android_vulkan::eButtonState::Down );
    gamepad.UnbindKey ( android_vulkan::eGamepadKey::Left, android_vulkan::eButtonState::Up );
    gamepad.UnbindKey ( android_vulkan::eGamepadKey::Right, android_vulkan::eButtonState::Down );
    gamepad.UnbindKey ( android_vulkan::eGamepadKey::Right, android_vulkan::eButtonState::Up );
    gamepad.UnbindKey ( android_vulkan::eGamepadKey::Up, android_vulkan::eButtonState::Down );
    gamepad.UnbindKey ( android_vulkan::eGamepadKey::Up, android_vulkan::eButtonState::Up );
    gamepad.UnbindKey ( android_vulkan::eGamepadKey::Y, android_vulkan::eButtonState::Down );
    gamepad.UnbindKey ( android_vulkan::eGamepadKey::Y, android_vulkan::eButtonState::Up );

    _body = nullptr;
    _pitch = 0;
    _roll = 0;
}

void Manipulator::Update ( GXMat4 const &cameraLocal, float deltaTime ) noexcept
{
    if ( _pitch == 0 && _roll == 0 && _height == 0 )
        return;

    float const angle = VELOCITY_ANGULAR * deltaTime;

    GXQuat pitchRotation {};
    GXVec3 pitch {};
    cameraLocal.GetX ( pitch );
    pitchRotation.FromAxisAngle ( pitch, angle * static_cast<float> ( _pitch ) );

    GXQuat rollRotation {};
    GXVec3 roll {};
    cameraLocal.GetZ ( roll );
    rollRotation.FromAxisAngle ( roll, angle * static_cast<float> ( _roll ) );

    GXQuat r {};
    r.Multiply ( rollRotation, pitchRotation );

    GXQuat finalRotation {};
    android_vulkan::RigidBody &b = *_body.get ();
    finalRotation.Multiply ( r, b.GetRotation () );
    b.SetRotation ( finalRotation, false );

    GXVec3 location = b.GetLocation ();
    location._data[ 1U ] += VELOCITY_LINEAR * deltaTime * static_cast<float> ( _height );
    b.SetLocation ( location, false );
}

void Manipulator::OnBDown ( void* context ) noexcept
{
    Manipulator &manipulator = *static_cast<Manipulator*> ( context );
    --manipulator._height;
}

void Manipulator::OnBUp ( void* context ) noexcept
{
    Manipulator &manipulator = *static_cast<Manipulator*> ( context );
    ++manipulator._height;
}

void Manipulator::OnDownDown ( void* context ) noexcept
{
    Manipulator &manipulator = *static_cast<Manipulator*> ( context );
    --manipulator._pitch;
}

void Manipulator::OnDownUp ( void* context ) noexcept
{
    Manipulator &manipulator = *static_cast<Manipulator*> ( context );
    ++manipulator._pitch;
}

void Manipulator::OnLeftDown ( void* context ) noexcept
{
    Manipulator &manipulator = *static_cast<Manipulator*> ( context );
    --manipulator._roll;
}

void Manipulator::OnLeftUp ( void* context ) noexcept
{
    Manipulator &manipulator = *static_cast<Manipulator*> ( context );
    ++manipulator._roll;
}

void Manipulator::OnRightDown ( void* context ) noexcept
{
    Manipulator &manipulator = *static_cast<Manipulator*> ( context );
    ++manipulator._roll;
}

void Manipulator::OnRightUp ( void* context ) noexcept
{
    Manipulator &manipulator = *static_cast<Manipulator*> ( context );
    --manipulator._roll;
}

void Manipulator::OnUpDown ( void* context ) noexcept
{
    Manipulator &manipulator = *static_cast<Manipulator*> ( context );
    ++manipulator._pitch;
}

void Manipulator::OnUpUp ( void* context ) noexcept
{
    Manipulator &manipulator = *static_cast<Manipulator*> ( context );
    --manipulator._pitch;
}

void Manipulator::OnYDown ( void* context ) noexcept
{
    Manipulator &manipulator = *static_cast<Manipulator*> ( context );
    ++manipulator._height;
}

void Manipulator::OnYUp ( void* context ) noexcept
{
    Manipulator &manipulator = *static_cast<Manipulator*> ( context );
    --manipulator._height;
}

} // namespace pbr::collision
