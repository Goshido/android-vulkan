#include <gamepad.h>

GX_DISABLE_COMMON_WARNINGS

#include <android/keycodes.h>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

Gamepad &Gamepad::GetInstance () noexcept
{
    static Gamepad gamepad {};
    return gamepad;
}

void Gamepad::BindKey ( void* context, KeyHandler handler, eGamepadKey key, eButtonState state ) noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );

    switch ( state )
    {
        case eButtonState::Down:
        {
            KeyBind &bind = _downKeyBinds[ static_cast<size_t> ( key ) ];
            bind._context = context;
            bind._handler = handler;
        }
        break;

        case eButtonState::Up:
        {
            KeyBind &bind = _upKeyBinds[ static_cast<size_t> ( key ) ];
            bind._context = context;
            bind._handler = handler;
        }
        break;

        default:
            // NOTHING
        break;
    }
}

void Gamepad::UnbindKey ( eGamepadKey key, eButtonState state ) noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );

    switch ( state )
    {
        case eButtonState::Down:
        {
            KeyBind &bind = _downKeyBinds[ static_cast<size_t> ( key ) ];
            bind._context = nullptr;
            bind._handler = nullptr;
        }
        break;

        case eButtonState::Up:
        {
            KeyBind &bind = _upKeyBinds[ static_cast<size_t> ( key ) ];
            bind._context = nullptr;
            bind._handler = nullptr;
        }
        break;

        default:
            // NOTHING
        break;
    }
}

void Gamepad::BindLeftStick ( void* context, StickHandler handler ) noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );
    _leftStick._context = context;
    _leftStick._handler = handler;
}

void Gamepad::UnbindLeftStick () noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );
    _leftStick._context = nullptr;
    _leftStick._handler = nullptr;
}

void Gamepad::BindRightStick ( void* context, StickHandler handler ) noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );
    _rightStick._context = context;
    _rightStick._handler = handler;
}

void Gamepad::UnbindRightStick () noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );
    _rightStick._context = nullptr;
    _rightStick._handler = nullptr;
}

[[maybe_unused]] void Gamepad::BindLeftTrigger ( void* context, TriggerHandler handler ) noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );
    _leftTrigger._context = context;
    _leftTrigger._handler = handler;
}

[[maybe_unused]] void Gamepad::UnbindLeftTrigger () noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );
    _leftTrigger._context = nullptr;
    _leftTrigger._handler = nullptr;
}

void Gamepad::BindRightTrigger ( void* context, TriggerHandler handler ) noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );
    _rightTrigger._context = context;
    _rightTrigger._handler = handler;
}

void Gamepad::UnbindRightTrigger () noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );
    _rightTrigger._context = nullptr;
    _rightTrigger._handler = nullptr;
}

bool Gamepad::OnKeyDown ( int32_t key ) const noexcept
{
    KeyBind bind {};

    {
        // Operator block is intentional because of unique lock.
        std::unique_lock<std::mutex> const lock ( _mutex );
        auto const findResult = _mapper.find ( key );

        if ( findResult == _mapper.cend () )
            return false;

        bind = _downKeyBinds[ findResult->second ];
    }

    if ( !bind._handler )
        return false;

    bind._handler ( bind._context );
    return true;
}

bool Gamepad::OnKeyUp ( int32_t key ) const noexcept
{
    KeyBind bind {};

    {
        // Operator block is intentional because of unique lock.
        std::unique_lock<std::mutex> const lock ( _mutex );
        auto const findResult = _mapper.find ( key );

        if ( findResult == _mapper.cend () )
            return false;

        bind = _upKeyBinds[ findResult->second ];
    }

    if ( !bind._handler )
        return false;

    bind._handler ( bind._context );
    return true;
}

void Gamepad::OnLeftStick ( float x, float y ) const noexcept
{
    Stick stick {};

    {
        // Operator block is intentional because of unique lock.
        std::unique_lock<std::mutex> const lock ( _mutex );
        stick = _leftStick;
    }

    if ( !stick._handler )
        return;

    stick._handler ( stick._context, x, y );
}

void Gamepad::OnRightStick ( float x, float y ) const noexcept
{
    Stick stick {};

    {
        // Operator block is intentional because of unique lock.
        std::unique_lock<std::mutex> const lock ( _mutex );
        stick = _rightStick;
    }

    if ( !stick._handler )
        return;

    stick._handler ( stick._context, x, y );
}

void Gamepad::OnLeftTrigger ( float value ) const noexcept
{
    Trigger trigger {};

    {
        // Operator block is intentional because of unique lock.
        std::unique_lock<std::mutex> const lock ( _mutex );
        trigger = _leftTrigger;
    }

    if ( !trigger._handler )
        return;

    trigger._handler ( trigger._context, value );
}

void Gamepad::OnRightTrigger ( float value ) const noexcept
{
    Trigger trigger {};

    {
        // Operator block is intentional because of unique lock.
        std::unique_lock<std::mutex> const lock ( _mutex );
        trigger = _rightTrigger;
    }

    if ( !trigger._handler )
        return;

    trigger._handler ( trigger._context, value );
}

Gamepad::Gamepad () noexcept
{
    std::unique_lock<std::mutex> const lock ( _mutex );

    _mapper.emplace ( AKEYCODE_BUTTON_A, static_cast<size_t> ( eGamepadKey::A ) );
    _mapper.emplace ( AKEYCODE_BUTTON_B, static_cast<size_t> ( eGamepadKey::B ) );
    _mapper.emplace ( AKEYCODE_BUTTON_X, static_cast<size_t> ( eGamepadKey::X ) );
    _mapper.emplace ( AKEYCODE_BUTTON_Y, static_cast<size_t> ( eGamepadKey::Y ) );
    _mapper.emplace ( AKEYCODE_DPAD_DOWN, static_cast<size_t> ( eGamepadKey::Down ) );
    _mapper.emplace ( AKEYCODE_DPAD_LEFT, static_cast<size_t> ( eGamepadKey::Left ) );
    _mapper.emplace ( AKEYCODE_DPAD_RIGHT, static_cast<size_t> ( eGamepadKey::Right ) );
    _mapper.emplace ( AKEYCODE_DPAD_UP, static_cast<size_t> ( eGamepadKey::Up ) );
    _mapper.emplace ( AKEYCODE_BUTTON_THUMBL, static_cast<size_t> ( eGamepadKey::LeftStick ) );
    _mapper.emplace ( AKEYCODE_BUTTON_THUMBR, static_cast<size_t> ( eGamepadKey::RightStick ) );
    _mapper.emplace ( AKEYCODE_BUTTON_L1, static_cast<size_t> ( eGamepadKey::LeftBumper ) );
    _mapper.emplace ( AKEYCODE_BUTTON_R1, static_cast<size_t> ( eGamepadKey::RightBumper ) );
    _mapper.emplace ( AKEYCODE_BUTTON_MODE, static_cast<size_t> ( eGamepadKey::Home ) );
    _mapper.emplace ( AKEYCODE_BUTTON_START, static_cast<size_t> ( eGamepadKey::Menu ) );
    _mapper.emplace ( AKEYCODE_BUTTON_SELECT, static_cast<size_t> ( eGamepadKey::View ) );
}

} // namespace android_vulkan
