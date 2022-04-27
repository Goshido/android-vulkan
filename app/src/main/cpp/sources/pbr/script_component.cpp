#include <pbr/script_component.h>
#include <pbr/script_engine.h>
#include <guid_generator.h>


namespace pbr {

[[maybe_unused]] int ScriptComponent::_registerScriptComponentIndex = std::numeric_limits<int>::max ();

ScriptComponent::ScriptComponent ( std::string &&script ) noexcept:
    Component ( ClassID::Script, android_vulkan::GUID::GenerateAsString ( "Script" ) ),
    _script ( std::move ( script ) )
{
    // NOTHING
}

ScriptComponent::ScriptComponent ( std::string &&script, std::string &&params ) noexcept:
    Component ( ClassID::Script, android_vulkan::GUID::GenerateAsString ( "Script" ) ),
    _script ( std::move ( script ) ),
    _params ( std::move ( params ) )
{
    // NOTHING
}

bool ScriptComponent::Register ( lua_State &vm ) noexcept
{
    if ( !lua_checkstack ( &vm, 4 ) )
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
        android_vulkan::LogError ( "pbr::Scene::OnInitDevice - Can't find register function." );
        return false;
    }

    _registerScriptComponentIndex = lua_gettop ( &vm );

    lua_register ( &vm, "av_ScriptComponentCreate", &ScriptComponent::OnCreate );
    return true;
}

int ScriptComponent::OnCreate ( lua_State* /*state*/ )
{
    // TODO
    return 0;
}

} // namespace pbr
