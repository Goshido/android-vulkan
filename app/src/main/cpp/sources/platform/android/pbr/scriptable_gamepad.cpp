#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <platform/android/pbr/script_engine.hpp>
#include <platform/android/pbr/scriptable_gamepad.hpp>


namespace pbr {

namespace {

constexpr android_vulkan::eButtonState const BUTTON_STATE_CASES[]
{
    android_vulkan::eButtonState::Down,
    android_vulkan::eButtonState::Up
};

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

ScriptableGamepad::ScriptableGamepad () noexcept
{
    _actionHandlers[ static_cast<size_t> ( eType::KeyDown ) ] = &ScriptableGamepad::KeyAction;
    _actionHandlers[ static_cast<size_t> ( eType::KeyUp ) ] = &ScriptableGamepad::KeyAction;
    _actionHandlers[ static_cast<size_t> ( eType::LeftStick ) ] = &ScriptableGamepad::StickAction;
    _actionHandlers[ static_cast<size_t> ( eType::RightStick ) ] = &ScriptableGamepad::StickAction;
    _actionHandlers[ static_cast<size_t> ( eType::LeftTrigger ) ] = &ScriptableGamepad::TriggerAction;
    _actionHandlers[ static_cast<size_t> ( eType::RightTrigger ) ] = &ScriptableGamepad::TriggerAction;

    KeyContext* keyContext = _keyContexts;

    auto setupKey = [ & ] ( android_vulkan::eGamepadKey key ) noexcept {
        keyContext->_instance = this;
        keyContext->_key = key;
        keyContext->_type = eType::KeyDown;
        ++keyContext;

        keyContext->_instance = this;
        keyContext->_key = key;
        keyContext->_type = eType::KeyUp;
        ++keyContext;
    };

    setupKey ( android_vulkan::eGamepadKey::A );
    setupKey ( android_vulkan::eGamepadKey::B );
    setupKey ( android_vulkan::eGamepadKey::Down );
    setupKey ( android_vulkan::eGamepadKey::Home );
    setupKey ( android_vulkan::eGamepadKey::Left );
    setupKey ( android_vulkan::eGamepadKey::LeftBumper );
    setupKey ( android_vulkan::eGamepadKey::LeftStick );
    setupKey ( android_vulkan::eGamepadKey::Menu );
    setupKey ( android_vulkan::eGamepadKey::Right );
    setupKey ( android_vulkan::eGamepadKey::RightBumper );
    setupKey ( android_vulkan::eGamepadKey::RightStick );
    setupKey ( android_vulkan::eGamepadKey::Up );
    setupKey ( android_vulkan::eGamepadKey::View );
    setupKey ( android_vulkan::eGamepadKey::X );
    setupKey ( android_vulkan::eGamepadKey::Y );

    auto setupAnalog = [ this ] ( eType type, AnalogContext* context, size_t idx ) noexcept {
        AnalogContext &c = context[ idx ];
        c._instance = this;
        c._type = type;
    };

    setupAnalog ( eType::LeftStick, _stickContexts, LEFT_INDEX );
    setupAnalog ( eType::RightStick, _stickContexts, RIGHT_INDEX );

    setupAnalog ( eType::LeftTrigger, _triggerContexts, LEFT_INDEX );
    setupAnalog ( eType::RightTrigger, _triggerContexts, RIGHT_INDEX );
}

void ScriptableGamepad::CaptureInput () noexcept
{
    android_vulkan::Gamepad &gamepad = android_vulkan::Gamepad::GetInstance ();

    for ( auto &c : _keyContexts )
    {
        gamepad.BindKey ( &c,
            &ScriptableGamepad::OnKey,
            c._key,
            BUTTON_STATE_CASES[ static_cast<size_t> ( c._type == eType::KeyUp ) ]
        );
    }

    gamepad.BindLeftStick ( &_stickContexts[ LEFT_INDEX ], &ScriptableGamepad::OnStick );
    gamepad.BindRightStick ( &_stickContexts[ RIGHT_INDEX ], &ScriptableGamepad::OnStick );

    gamepad.BindLeftTrigger ( &_triggerContexts[ LEFT_INDEX ], &ScriptableGamepad::OnTrigger );
    gamepad.BindRightTrigger ( &_triggerContexts[ RIGHT_INDEX ], &ScriptableGamepad::OnTrigger );
}

void ScriptableGamepad::ReleaseInput () const noexcept
{
    android_vulkan::Gamepad &gamepad = android_vulkan::Gamepad::GetInstance ();

    for ( auto const &c : _keyContexts )
        gamepad.UnbindKey ( c._key,  BUTTON_STATE_CASES[ static_cast<size_t> ( c._type == eType::KeyUp ) ] );

    gamepad.UnbindLeftStick ();
    gamepad.UnbindRightStick ();

    gamepad.UnbindLeftTrigger ();
    gamepad.UnbindRightTrigger ();
}

bool ScriptableGamepad::Execute ( lua_State &vm, int sceneHandleIndex, int onInputIndex ) noexcept
{
    if ( !lua_checkstack ( &vm, 3 ) ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::ScriptableGamepad::Execute - Stack is too small." );
        return false;
    }

    {
        std::lock_guard const lock ( _mutex );
        _readQueue.swap ( _writeQueue );
    }

    for ( auto const &action : _readQueue )
    {
        ActionHandler const handler = _actionHandlers[ static_cast<size_t> ( action._type ) ];

        // C++ calling method by pointer syntax.
        ( this->*handler ) ( action, vm, sceneHandleIndex, onInputIndex );
    }

    _readQueue.clear ();
    return true;
}

bool ScriptableGamepad::Init ( lua_State &vm ) noexcept
{
    return AllocateKeyInputEvent ( _keyDownIndex, vm, eType::KeyDown ) &&
        AllocateKeyInputEvent ( _keyUpIndex, vm, eType::KeyUp ) &&
        AllocateStickInputEvent ( _leftStickIndex, vm, eType::LeftStick ) &&
        AllocateStickInputEvent ( _rightStickIndex, vm, eType::RightStick ) &&
        AllocateTriggerInputEvent ( _leftTriggerIndex, vm, eType::LeftTrigger ) &&
        AllocateTriggerInputEvent ( _rightTriggerIndex, vm, eType::RightTrigger );
}

void ScriptableGamepad::Destroy () noexcept
{
    _keyDownIndex = std::numeric_limits<int>::max ();
    _keyUpIndex = std::numeric_limits<int>::max ();

    _leftStickIndex = std::numeric_limits<int>::max ();
    _rightStickIndex = std::numeric_limits<int>::max ();

    _leftTriggerIndex = std::numeric_limits<int>::max ();
    _rightTriggerIndex = std::numeric_limits<int>::max ();
}

void ScriptableGamepad::KeyAction ( Action const &action,
    lua_State &vm,
    int sceneHandleIndex,
    int onInputIndex
) noexcept
{
    lua_pushstring ( &vm, "_key" );
    lua_pushinteger ( &vm, static_cast<lua_Integer> ( std::get<android_vulkan::eGamepadKey> ( action._variant ) ) );

    // Branch-less selection.
    int const cases[] = { _keyUpIndex, _keyDownIndex };
    int const idx = cases[ static_cast<size_t> ( action._type == eType::KeyDown ) ];
    lua_rawset ( &vm, idx );

    lua_pushvalue ( &vm, onInputIndex );
    lua_pushvalue ( &vm, sceneHandleIndex );
    lua_pushvalue ( &vm, idx );

    if ( lua_pcall ( &vm, 2, 0, ScriptEngine::GetErrorHandlerIndex () ) != LUA_OK ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::ScriptableGamepad::KeyAction - Can't send input event to Lua VM." );
    }
}

void ScriptableGamepad::StickAction ( Action const &action,
    lua_State &vm,
    int sceneHandleIndex,
    int onInputIndex
) noexcept
{
    // Branch-less selection.
    int const cases[] = { _rightStickIndex, _leftStickIndex };
    int const idx = cases[ static_cast<size_t> ( action._type == eType::LeftStick ) ];

    lua_pushstring ( &vm, "_x" );
    lua_pushnumber ( &vm, static_cast<lua_Number> ( std::get<Stick> ( action._variant )._x ) );
    lua_rawset ( &vm, idx );

    lua_pushstring ( &vm, "_y" );
    lua_pushnumber ( &vm, static_cast<lua_Number> ( std::get<Stick> ( action._variant )._y ) );
    lua_rawset ( &vm, idx );

    lua_pushvalue ( &vm, onInputIndex );
    lua_pushvalue ( &vm, sceneHandleIndex );
    lua_pushvalue ( &vm, idx );

    if ( lua_pcall ( &vm, 2, 0, ScriptEngine::GetErrorHandlerIndex () ) != LUA_OK ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::ScriptableGamepad::StickAction - Can't send input event to Lua VM." );
    }
}

void ScriptableGamepad::TriggerAction ( Action const &action,
    lua_State &vm,
    int sceneHandleIndex,
    int onInputIndex
) noexcept
{
    // Branch-less selection.
    int const cases[] = { _rightTriggerIndex, _leftTriggerIndex };
    int const idx = cases[ static_cast<size_t> ( action._type == eType::LeftTrigger ) ];

    lua_pushstring ( &vm, "_value" );
    lua_pushnumber ( &vm, static_cast<lua_Number> ( std::get<float> ( action._variant ) ) );
    lua_rawset ( &vm, idx );

    lua_pushvalue ( &vm, onInputIndex );
    lua_pushvalue ( &vm, sceneHandleIndex );
    lua_pushvalue ( &vm, idx );

    if ( lua_pcall ( &vm, 2, 0, ScriptEngine::GetErrorHandlerIndex () ) != LUA_OK ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::ScriptableGamepad::TriggerAction - Can't send input event to Lua VM." );
    }
}

bool ScriptableGamepad::AllocateKeyInputEvent ( int &eventIndex, lua_State &vm, eType type ) noexcept
{
    if ( !lua_checkstack ( &vm, 3 ) ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::ScriptableGamepad::AllocateKeyInputEvent - Stack is too small." );
        return false;
    }

    lua_createtable ( &vm, 0, 2 );
    eventIndex = lua_gettop ( &vm );

    lua_pushstring ( &vm, "_type" );
    lua_pushinteger ( &vm, static_cast<lua_Integer> ( type ) );
    lua_rawset ( &vm, eventIndex );

    lua_pushstring ( &vm, "_key" );
    lua_pushinteger ( &vm, std::numeric_limits<lua_Integer>::max () );
    lua_rawset ( &vm, eventIndex );

    return true;
}

bool ScriptableGamepad::AllocateStickInputEvent ( int &eventIndex, lua_State &vm, eType type ) noexcept
{
    if ( !lua_checkstack ( &vm, 3 ) ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::ScriptableGamepad::AllocateStickInputEvent - Stack is too small." );
        return false;
    }

    lua_createtable ( &vm, 0, 3 );
    eventIndex = lua_gettop ( &vm );

    lua_pushstring ( &vm, "_type" );
    lua_pushinteger ( &vm, static_cast<lua_Integer> ( type ) );
    lua_rawset ( &vm, eventIndex );

    lua_pushstring ( &vm, "_x" );
    lua_pushnumber ( &vm, std::numeric_limits<lua_Number>::max () );
    lua_rawset ( &vm, eventIndex );

    lua_pushstring ( &vm, "_y" );
    lua_pushnumber ( &vm, std::numeric_limits<lua_Number>::max () );
    lua_rawset ( &vm, eventIndex );

    return true;
}

bool ScriptableGamepad::AllocateTriggerInputEvent ( int &eventIndex, lua_State &vm, eType type ) noexcept
{
    if ( !lua_checkstack ( &vm, 3 ) ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::ScriptableGamepad::AllocateTriggerInputEvent - Stack is too small." );
        return false;
    }

    lua_createtable ( &vm, 0, 2 );
    eventIndex = lua_gettop ( &vm );

    lua_pushstring ( &vm, "_type" );
    lua_pushinteger ( &vm, static_cast<lua_Integer> ( type ) );
    lua_rawset ( &vm, eventIndex );

    lua_pushstring ( &vm, "_value" );
    lua_pushnumber ( &vm, std::numeric_limits<lua_Number>::max () );
    lua_rawset ( &vm, eventIndex );

    return true;
}

void ScriptableGamepad::OnKey ( void* context ) noexcept
{
    auto const &ctx = *static_cast<KeyContext const*> ( context );

    ScriptableGamepad &gamepad = *ctx._instance;
    std::lock_guard const lock ( gamepad._mutex );

    gamepad._writeQueue.push_back (
        {
            ._type = ctx._type,
            ._variant = ctx._key
        }
    );
}

void ScriptableGamepad::OnStick ( void* context, float x, float y ) noexcept
{
    auto const &ctx = *static_cast<AnalogContext const*> ( context );

    ScriptableGamepad &gamepad = *ctx._instance;
    std::lock_guard const lock ( gamepad._mutex );

    gamepad._writeQueue.push_back (
        {
            ._type = ctx._type,

            ._variant = Stick
            {
                ._x = x,
                ._y = y
            }
        }
    );
}

void ScriptableGamepad::OnTrigger ( void* context, float value ) noexcept
{
    auto const &ctx = *static_cast<AnalogContext const*> ( context );

    ScriptableGamepad &gamepad = *ctx._instance;
    std::lock_guard const lock ( gamepad._mutex );

    gamepad._writeQueue.push_back (
        {
            ._type = ctx._type,
            ._variant = value
        }
    );
}

} // namespace pbr
