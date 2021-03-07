#include <gamepad.h>

GX_DISABLE_COMMON_WARNINGS

#include <map>
#include <unistd.h>
#include <android/keycodes.h>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

constexpr static useconds_t const TIMEOUT_MICROSECONDS = 10000U;
constexpr static int32_t const EVENT_HANDLED = 1;
constexpr static int32_t const EVENT_IGNORED = 0;

//----------------------------------------------------------------------------------------------------------------------

Gamepad& Gamepad::GetInstance ()
{
    static Gamepad gamepad;
    return gamepad;
}

void Gamepad::BindKey ( void* context, KeyHandler handler, eGamepadKey key, eButtonState state )
{
    std::unique_lock<std::shared_timed_mutex> lock ( _mutex );

    switch ( state )
    {
        case eButtonState::Down:
            _downKeyBinds[ static_cast<size_t> ( static_cast<uint8_t> ( key ) ) ].Init ( context, handler );
        break;

        case eButtonState::Up:
            _upKeyBinds[ static_cast<size_t> ( static_cast<uint8_t> ( key ) ) ].Init ( context, handler );
        break;
    }
}

void Gamepad::UnbindKey ( eGamepadKey key, eButtonState state )
{
    std::unique_lock<std::shared_timed_mutex> lock ( _mutex );

    switch ( state )
    {
        case eButtonState::Down:
            _downKeyBinds[ static_cast<size_t> ( static_cast<uint8_t> ( key ) ) ].Reset ();
        break;

        case eButtonState::Up:
            _upKeyBinds[ static_cast<size_t> ( static_cast<uint8_t> ( key ) ) ].Reset ();
        break;
    }
}

void Gamepad::BindLeftStick ( void* context, StickHandler handler )
{
    std::unique_lock<std::shared_timed_mutex> lock ( _mutex );
    _leftStick.Bind ( context, handler );
}

void Gamepad::UnbindLeftStick ()
{
    std::unique_lock<std::shared_timed_mutex> lock ( _mutex );
    _leftStick.Unbind ();
}

void Gamepad::BindRightStick ( void* context, StickHandler handler )
{
    std::unique_lock<std::shared_timed_mutex> lock ( _mutex );
    _rightStick.Bind ( context, handler );
}

void Gamepad::UnbindRightStick ()
{
    std::unique_lock<std::shared_timed_mutex> lock ( _mutex );
    _rightStick.Unbind ();
}

void Gamepad::BindLeftTrigger ( void* context, TriggerHandler handler )
{
    std::unique_lock<std::shared_timed_mutex> lock ( _mutex );
    _leftTrigger.Bind ( context, handler );
}

void Gamepad::UnbindLeftTrigger ()
{
    std::unique_lock<std::shared_timed_mutex> lock ( _mutex );
    _leftTrigger.Unbind ();
}

void Gamepad::BindRightTrigger ( void* context, TriggerHandler handler )
{
    std::unique_lock<std::shared_timed_mutex> lock ( _mutex );
    _rightTrigger.Bind ( context, handler );
}

void Gamepad::UnbindRightTrigger ()
{
    std::unique_lock<std::shared_timed_mutex> lock ( _mutex );
    _rightTrigger.Unbind ();
}

int32_t Gamepad::OnOSInputEvent ( AInputEvent* event )
{
    if ( !_loop )
        return EVENT_IGNORED;

    auto const source = static_cast<uint32_t> ( AInputEvent_getSource ( event ) );

    if ( ( source & AINPUT_SOURCE_GAMEPAD ) == AINPUT_SOURCE_GAMEPAD )
        return AInputEvent_getType ( event ) == AINPUT_EVENT_TYPE_KEY ? HandleKey ( event ) : EVENT_IGNORED;

    if ( ( source & AINPUT_SOURCE_JOYSTICK ) == AINPUT_SOURCE_JOYSTICK )
        return AInputEvent_getType ( event ) == AINPUT_EVENT_TYPE_MOTION ? HandleMotion ( event ) : EVENT_IGNORED;

    return EVENT_IGNORED;
}

void Gamepad::Start ()
{
    auto job = [ & ] () {
        while ( _loop )
        {
            std::unique_lock<std::shared_timed_mutex> lock ( _mutex );

            ResolveDPad ();

            ExecuteKeyEvents ();
            ExecuteStickEvents ();
            ExecuteTriggerEvents ();

            usleep ( TIMEOUT_MICROSECONDS );
        }
    };

    _loop = true;
    _thread = std::thread ( job );
}

void Gamepad::Stop ()
{
    _loop = false;
    _thread.join ();
}

Gamepad::Gamepad ():
    _downKeyBinds {},
    _upKeyBinds {},
    _dPadCurrent {},
    _dPadOld {},
    _loop ( false ),
    _keyActionPool {},
    _freeKeyActions ( nullptr ),
    _readyKeyActions ( nullptr ),
    _leftStick {},
    _rightStick {},
    _leftTrigger {},
    _rightTrigger {},
    _mutex {},
    _thread {}
{
    InitActionPool ();
}

void Gamepad::AddAction ( KeyBind const &bind )
{
    // Note actions will be placed in reverse order.
    // It is assumed that order does not matter.

    KeyAction* newAction = _freeKeyActions;
    _freeKeyActions = _freeKeyActions->_next;

    newAction->_bind = bind;
    newAction->_next = _readyKeyActions;
    _readyKeyActions = newAction;
}

void Gamepad::ExecuteKeyEvents ()
{
    KeyAction* tail = nullptr;

    while ( _readyKeyActions )
    {
        tail = _readyKeyActions;

        KeyBind const& bind = _readyKeyActions->_bind;
        bind._handler ( bind._context );

        _readyKeyActions = _readyKeyActions->_next;
    }

    if ( !tail )
        return;

    // Returning key actions to event pool.
    tail->_next = _freeKeyActions;
    _freeKeyActions = tail;
}

void Gamepad::ExecuteStickEvents ()
{
    _leftStick.Execute ();
    _rightStick.Execute ();
}

void Gamepad::ExecuteTriggerEvents ()
{
    _leftTrigger.Execute ();
    _rightTrigger.Execute ();
}

void Gamepad::HandleDPad ( AInputEvent* event )
{
    GXVec2 const raw ( AMotionEvent_getAxisValue ( event, AMOTION_EVENT_AXIS_HAT_X, 0U ),
        AMotionEvent_getAxisValue ( event, AMOTION_EVENT_AXIS_HAT_Y, 0U )
    );

    if ( raw._data[ 0U ] < 0.0F )
    {
        _dPadCurrent._left = true;
        _dPadCurrent._right = false;
    }
    else
    {
        _dPadCurrent._left = false;
        _dPadCurrent._right = raw._data[ 0U ] > 0.0F;
    }

    if ( raw._data[ 1U ] < 0.0F )
    {
        _dPadCurrent._up = true;
        _dPadCurrent._down = false;
        return;
    }

    _dPadCurrent._up = false;
    _dPadCurrent._down = raw._data[ 1U ] > 0.0F;
}

int32_t Gamepad::HandleKey ( AInputEvent* event )
{
    if ( AKeyEvent_getRepeatCount ( event ) > 0 )
    {
        // auto repeated events are filtered.
        return EVENT_IGNORED;
    }

    // see here https://developer.android.com/reference/android/view/KeyEvent
    static std::map<int32_t, size_t> const mapper =
    {
        { AKEYCODE_BUTTON_A, static_cast<size_t> ( static_cast<uint8_t> ( eGamepadKey::A ) ) },
        { AKEYCODE_BUTTON_B, static_cast<size_t> ( static_cast<uint8_t> ( eGamepadKey::B ) ) },
        { AKEYCODE_BUTTON_X, static_cast<size_t> ( static_cast<uint8_t> ( eGamepadKey::X ) ) },
        { AKEYCODE_BUTTON_Y, static_cast<size_t> ( static_cast<uint8_t> ( eGamepadKey::Y ) ) },
        { AKEYCODE_DPAD_DOWN, static_cast<size_t> ( static_cast<uint8_t> ( eGamepadKey::Down ) ) },
        { AKEYCODE_DPAD_LEFT, static_cast<size_t> ( static_cast<uint8_t> ( eGamepadKey::Left ) ) },
        { AKEYCODE_DPAD_RIGHT, static_cast<size_t> ( static_cast<uint8_t> ( eGamepadKey::Right ) ) },
        { AKEYCODE_DPAD_UP, static_cast<size_t> ( static_cast<uint8_t> ( eGamepadKey::Up ) ) },
        { AKEYCODE_BUTTON_THUMBL, static_cast<size_t> ( static_cast<uint8_t> ( eGamepadKey::LeftStick ) ) },
        { AKEYCODE_BUTTON_THUMBR, static_cast<size_t> ( static_cast<uint8_t> ( eGamepadKey::RightStick ) ) },
        { AKEYCODE_BUTTON_L1, static_cast<size_t> ( static_cast<uint8_t> ( eGamepadKey::LeftBumper ) ) },
        { AKEYCODE_BUTTON_R1, static_cast<size_t> ( static_cast<uint8_t> ( eGamepadKey::RightBumper ) ) },
        { AKEYCODE_BUTTON_MODE, static_cast<size_t> ( static_cast<uint8_t> ( eGamepadKey::Home ) ) },
        { AKEYCODE_BUTTON_START, static_cast<size_t> ( static_cast<uint8_t> ( eGamepadKey::Menu ) ) },
        { AKEYCODE_BUTTON_SELECT, static_cast<size_t> ( static_cast<uint8_t> ( eGamepadKey::View ) ) }
    };

    auto const findResult = mapper.find ( AKeyEvent_getKeyCode ( event ) );

    if ( findResult == mapper.cend () )
        return EVENT_IGNORED;

    KeyBind* target = nullptr;

    switch ( AKeyEvent_getAction ( event ) )
    {
        case AKEY_EVENT_ACTION_DOWN:
            target = _downKeyBinds + findResult->second;
        break;

        case AKEY_EVENT_ACTION_UP:
            target = _upKeyBinds + findResult->second;
        break;

        default:
            // NOTHING
        break;
    }

    if ( !target || !target->_handler )
        return EVENT_IGNORED;

    AddAction ( *target );
    return EVENT_HANDLED;
}

int32_t Gamepad::HandleMotion ( AInputEvent* event )
{
    HandleDPad ( event );
    HandleSticks ( event );
    HandleTriggers ( event );

    return EVENT_HANDLED;
}

void Gamepad::HandleSticks ( AInputEvent* event )
{
    _leftStick.Update ( event, AMOTION_EVENT_AXIS_X, AMOTION_EVENT_AXIS_Y );
    _rightStick.Update ( event, AMOTION_EVENT_AXIS_Z, AMOTION_EVENT_AXIS_RZ );
}

void Gamepad::HandleTriggers ( AInputEvent* event )
{
    _leftTrigger.Update ( event, AMOTION_EVENT_AXIS_LTRIGGER );
    _rightTrigger.Update ( event, AMOTION_EVENT_AXIS_RTRIGGER );
}

void Gamepad::InitActionPool ()
{
    constexpr size_t const lastKey = ACTION_POOL_ELEMENT_COUNT - 1U;

    for ( size_t i = 0U; i < lastKey; ++i )
        _keyActionPool[ i ]._next = _keyActionPool + ( i + 1U );

    _keyActionPool[ lastKey ]._next = nullptr;
    _freeKeyActions = _keyActionPool;
}

void Gamepad::ResolveDPad ()
{
    auto resolve = [ & ] ( bool current, bool old, eGamepadKey key ) {
        if ( current == old )
            return;

        KeyBind& target = current ?
            _downKeyBinds[ static_cast<size_t> ( static_cast<uint8_t> ( key ) ) ]:
            _upKeyBinds[ static_cast<size_t> ( static_cast<uint8_t> ( key ) ) ];

        if ( !target._handler )
            return;

        AddAction ( target );
    };

    resolve ( _dPadCurrent._down, _dPadOld._down, eGamepadKey::Down );
    resolve ( _dPadCurrent._left, _dPadOld._left, eGamepadKey::Left );
    resolve ( _dPadCurrent._right, _dPadOld._right, eGamepadKey::Right );
    resolve ( _dPadCurrent._up, _dPadOld._up, eGamepadKey::Up );

    _dPadOld = _dPadCurrent;
}

} // namespace android_vulkan
