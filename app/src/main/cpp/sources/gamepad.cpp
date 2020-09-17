#include <gamepad.h>

GX_DISABLE_COMMON_WARNINGS

#include <map>
#include <unistd.h>
#include <android/keycodes.h>

GX_RESTORE_WARNING_STATE

#include <logger.h>


namespace android_vulkan {

constexpr static useconds_t const TIMEOUT_MICROSECONDS = 20000U;
constexpr static int32_t const EVENT_HANDLED = 1;
constexpr static int32_t const EVENT_IGNORED = 0;

//----------------------------------------------------------------------------------------------------------------------

Gamepad::KeyBind::KeyBind ():
    _context ( nullptr ),
    _handler ( nullptr )
{
    // NOTHING
}

void Gamepad::KeyBind::Init ( void* context, KeyHandler handler )
{
    _context = context;
    _handler = handler;
}

void Gamepad::KeyBind::Reset ()
{
    _context = nullptr;
    _handler = nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

Gamepad::KeyAction::KeyAction ():
    _bind {},
    _next ( nullptr )
{
    // NOTHING
}

//----------------------------------------------------------------------------------------------------------------------

Gamepad::Stick::Stick ():
    _isEvent ( false ),
    _state ( 0.0F, 0.0F ),
    _context ( nullptr ),
    _handler ( nullptr )
{
    // NOTHING
}

void Gamepad::Stick::Bind ( void* context, StickHandler handler )
{
    _isEvent = false;
    _state = GXVec2 ( 0.0F, 0.0F );
    _context = context;
    _handler = handler;
}

void Gamepad::Stick::Unbind ()
{
    _isEvent = false;
    _context = nullptr;
    _handler = nullptr;
}

void Gamepad::Stick::Execute ()
{
    if ( !_isEvent )
        return;

    _handler ( _context, _state._data[ 0U ], _state._data[ 1U ] );
    _isEvent = false;
}

//----------------------------------------------------------------------------------------------------------------------

Gamepad::Trigger::Trigger ():
    _isEvent ( false ),
    _push ( 0.0F ),
    _context ( nullptr ),
    _handler ( nullptr )
{
    // TODO
}

void Gamepad::Trigger::Bind ( void* context, TriggerHandler handler )
{
    _isEvent = false;
    _push = 0.0F;
    _context = context;
    _handler = handler;
}

void Gamepad::Trigger::Unbind ()
{
    _isEvent = false;
    _context = nullptr;
    _handler = nullptr;
}

void Gamepad::Trigger::Execute ()
{
    if ( !_isEvent )
        return;

    _handler ( _context, _push );
    _isEvent = false;
}

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

    if ( ( source & AINPUT_SOURCE_GAMEPAD ) != AINPUT_SOURCE_GAMEPAD )
        return EVENT_IGNORED;

    switch ( AInputEvent_getType ( event ) )
    {
        case AINPUT_EVENT_TYPE_KEY:
        return HandleKey ( event );

        default:
            // NOTHING
        break;
    }

    // TODO
    return EVENT_IGNORED;
}

void Gamepad::Start ()
{
    auto job = [ & ] () {
        while ( _loop )
        {
            std::unique_lock<std::shared_timed_mutex> lock ( _mutex );

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
        { AKEYCODE_BUTTON_R1, static_cast<size_t> ( static_cast<uint8_t> ( eGamepadKey::RightBumper ) ) }
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

void Gamepad::ExecuteTriggerEvents ()
{
    _leftTrigger.Execute ();
    _rightTrigger.Execute ();
}

void Gamepad::InitActionPool ()
{
    constexpr size_t const lastKey = ACTION_POOL_ELEMENT_COUNT - 1U;

    for ( size_t i = 0U; i < lastKey; ++i )
        _keyActionPool[ i ]._next = _keyActionPool + ( i + 1U );

    _keyActionPool[ lastKey ]._next = nullptr;
    _freeKeyActions = _keyActionPool;
}

} // namespace android_vulkan
