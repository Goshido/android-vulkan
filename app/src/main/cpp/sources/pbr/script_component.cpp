#include <pbr/script_component.h>
#include <pbr/script_engine.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace pbr {

[[maybe_unused]] constexpr static uint32_t SCRIPT_COMPONENT_DESC_FORMAT_VERSION = 1U;

//----------------------------------------------------------------------------------------------------------------------

int ScriptComponent::_registerScriptComponentIndex = std::numeric_limits<int>::max ();

ScriptComponent::ScriptComponent ( ScriptComponentDesc const &desc, uint8_t const* data ) noexcept:
    Component ( ClassID::Script )
{
    // sanity check.
    assert ( desc._formatVersion == SCRIPT_COMPONENT_DESC_FORMAT_VERSION );

    _name = reinterpret_cast<char const*> ( data + desc._name );
    _script = reinterpret_cast<char const*> ( data + desc._script );

    if ( desc._params != android_vulkan::NO_UTF8_OFFSET )
    {
        _params = reinterpret_cast<char const*> ( data + desc._params );
    }
}

ScriptComponent::ScriptComponent ( std::string &&script, std::string &&name ) noexcept:
    Component ( ClassID::Script, std::move ( name ) ),
    _script ( std::move ( script ) )
{
    // NOTHING
}

ScriptComponent::ScriptComponent ( std::string &&script, std::string &&params, std::string &&name ) noexcept:
    Component ( ClassID::Script, std::move ( name ) ),
    _script ( std::move ( script ) ),
    _params ( std::move ( params ) )
{
    // NOTHING
}

bool ScriptComponent::Register ( lua_State &vm ) noexcept
{
    if ( !lua_checkstack ( &vm, 5 ) )
    {
        android_vulkan::LogError ( "pbr::ScriptComponent::Register - Stack too small." );
        return false;
    }

    lua_pushvalue ( &vm, _registerScriptComponentIndex );
    lua_pushlightuserdata ( &vm, this );
    lua_pushlstring ( &vm, _script.c_str (), _script.size () );

    if ( _params.empty () )
        lua_pushnil ( &vm );
    else
        lua_pushlstring ( &vm, _params.c_str (), _params.size () );

    return lua_pcall ( &vm, 3, 1, ScriptEngine::GetErrorHandlerIndex () ) == LUA_OK;
}

bool ScriptComponent::Init ( lua_State &vm ) noexcept
{
    if ( !lua_checkstack ( &vm, 1 ) )
    {
        android_vulkan::LogError ( "pbr::ScriptComponent::Init - Stack too small." );
        return false;
    }

    if ( lua_getglobal ( &vm, "RegisterScriptComponent" ) != LUA_TFUNCTION )
    {
        android_vulkan::LogError ( "pbr::ScriptComponent::Init - Can't find register function." );
        return false;
    }

    _registerScriptComponentIndex = lua_gettop ( &vm );

    lua_register ( &vm, "av_ScriptComponentCreate", &ScriptComponent::OnCreate );
    return true;
}

ComponentRef& ScriptComponent::GetReference () noexcept
{
    // TODO
    static ComponentRef dummy {};
    return dummy;
}

int ScriptComponent::OnCreate ( lua_State* /*state*/ )
{
    // TODO
    return 0;
}

} // namespace pbr
