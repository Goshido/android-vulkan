#include <pbr/script_component.hpp>
#include <pbr/script_engine.hpp>
#include <av_assert.hpp>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

namespace {

[[maybe_unused]] constexpr uint32_t SCRIPT_COMPONENT_DESC_FORMAT_VERSION = 1U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

int ScriptComponent::_registerScriptComponentIndex = std::numeric_limits<int>::max ();
std::unordered_map<Component const*, ComponentRef> ScriptComponent::_scripts {};

ScriptComponent::ScriptComponent ( ScriptComponentDesc const &desc, uint8_t const* data ) noexcept:
    Component ( ClassID::Script )
{
    // sanity check.
    AV_ASSERT ( desc._formatVersion == SCRIPT_COMPONENT_DESC_FORMAT_VERSION )

    _name = reinterpret_cast<char const*> ( data + desc._name );
    _script = reinterpret_cast<char const*> ( data + desc._script );

    if ( desc._params != android_vulkan::NO_UTF8_OFFSET )
    {
        _params = reinterpret_cast<char const*> ( data + desc._params );
    }
}

ScriptComponent::ScriptComponent ( std::string &&name ) noexcept:
    Component ( ClassID::Script, std::move ( name ) )
{
    // NOTHING
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

bool ScriptComponent::RegisterFromNative ( lua_State &vm, Actor &actor ) noexcept
{
    if ( !lua_checkstack ( &vm, 5 ) ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::ScriptComponent::RegisterFromNative - Stack is too small." );
        return false;
    }

    _actor = &actor;

    lua_pushvalue ( &vm, _registerScriptComponentIndex );
    lua_pushlightuserdata ( &vm, this );
    lua_pushlstring ( &vm, _script.c_str (), _script.size () );

    if ( _params.empty () )
        lua_pushnil ( &vm );
    else
        lua_pushlstring ( &vm, _params.c_str (), _params.size () );

    return lua_pcall ( &vm, 3, 1, ScriptEngine::GetErrorHandlerIndex () ) == LUA_OK;
}

void ScriptComponent::RegisterFromScript ( Actor &actor ) noexcept
{
    _actor = &actor;
}

bool ScriptComponent::Init ( lua_State &vm ) noexcept
{
    if ( !lua_checkstack ( &vm, 1 ) ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::ScriptComponent::Init - Stack is too small." );
        return false;
    }

    if ( lua_getglobal ( &vm, "RegisterScriptComponent" ) != LUA_TFUNCTION ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::ScriptComponent::Init - Can't find register function." );
        return false;
    }

    _registerScriptComponentIndex = lua_gettop ( &vm );


    constexpr luaL_Reg const extensions[] =
    {
        {
            .name = "av_ScriptComponentCreate",
            .func = &ScriptComponent::OnCreate
        },
        {
            .name = "av_ScriptComponentDestroy",
            .func = &ScriptComponent::OnDestroy
        },
        {
            .name = "av_ScriptComponentCollectGarbage",
            .func = &ScriptComponent::OnGarbageCollected
        }
    };

    for ( auto const &extension : extensions )
        lua_register ( &vm, extension.name, extension.func );

    return true;
}

void ScriptComponent::Destroy () noexcept
{
    if ( !_scripts.empty () ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::ScriptComponent::Destroy - Memory leak." );
        AV_ASSERT ( false )
    }

    _scripts.clear ();
}

ComponentRef &ScriptComponent::GetReference () noexcept
{
    auto findResult = _scripts.find ( this );
    AV_ASSERT ( findResult != _scripts.end () )
    return findResult->second;
}

int ScriptComponent::OnCreate ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::ScriptComponent::OnCreate - Stack is too small." );
        return 0;
    }

    char const* name = lua_tostring ( state, 1 );

    if ( !name ) [[unlikely]]
    {
        lua_pushnil ( state );
        return 1;
    }

    ComponentRef component = std::make_shared<ScriptComponent> ( name );
    Component* handle = component.get ();
    _scripts.emplace ( handle, std::move ( component ) );

    lua_pushlightuserdata ( state, handle );
    return 1;
}

int ScriptComponent::OnDestroy ( lua_State* state )
{
    auto &self = *static_cast<ScriptComponent*> ( lua_touserdata ( state, 1 ) );
    self._actor->DestroyComponent ( self );
    return 0;
}

int ScriptComponent::OnGarbageCollected ( lua_State* state )
{
    _scripts.erase ( static_cast<Component*> ( lua_touserdata ( state, 1 ) ) );
    return 0;
}

} // namespace pbr
