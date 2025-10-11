#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <platform/android/pbr/rigid_body_component.hpp>
#include <platform/android/pbr/script_engine.hpp>
#include <platform/android/pbr/scriptable_gxvec3.hpp>
#include <platform/android/pbr/scriptable_raycast_result.hpp>


namespace pbr {

namespace {

constexpr std::string_view FIELD_BODY = "_body";
constexpr std::string_view FIELD_POINT = "_point";
constexpr std::string_view FIELD_NORMAL = "_normal";

constexpr char const GLOBAL_TABLE[] = "av_scriptableRaycastResult";

} // end of anonymous namespace

bool ScriptableRaycastResult::Init ( lua_State &vm ) noexcept
{
    if ( !lua_checkstack ( &vm, 4 ) ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::ScriptableRaycastResult::Init - Stack is too small." );
        return false;
    }

    lua_createtable ( &vm, 0, 3 );
    lua_setglobal ( &vm, GLOBAL_TABLE );
    lua_getglobal ( &vm, GLOBAL_TABLE );
    int const globalTableIdx = lua_gettop ( &vm );

    // Don't care about actual value. Main task is to allocate internal Lua table with something.
    lua_pushlstring ( &vm, FIELD_BODY.data (), FIELD_BODY.size () );
    constexpr lua_Integer proxyRigidBody = 777;
    lua_pushinteger ( &vm, proxyRigidBody );
    lua_rawset ( &vm, -3 );

    if ( !ScriptableGXVec3::PrepareLuaConstructor ( vm ) ) [[unlikely]]
    {
        lua_pop ( &vm, 1 );
        return false;
    }

    int const vec3ConstructorIdx = lua_gettop ( &vm );

    auto const connect = [ & ] ( GXVec3* &target, std::string_view const &field ) noexcept -> bool {
        lua_pushlstring ( &vm, field.data (), field.size () );
        lua_pushvalue ( &vm, vec3ConstructorIdx );

        if ( lua_pcall ( &vm, 0, 1, ScriptEngine::GetErrorHandlerIndex () ) != LUA_OK ) [[unlikely]]
        {
            lua_pop ( &vm, 4 );
            return false;
        }

        auto probe = ScriptableGXVec3::ExtractFromLua ( vm, -1 );

        if ( !probe ) [[unlikely]]
        {
            lua_pop ( &vm, 4 );
            return false;
        }

        target = *probe;
        lua_rawset ( &vm, globalTableIdx );
        return true;
    };

    if ( !connect ( _normal, FIELD_NORMAL ) || !connect ( _point, FIELD_POINT ) ) [[unlikely]]
        return false;

    lua_pop ( &vm, 2 );
    return true;
}

void ScriptableRaycastResult::Destroy ( lua_State &vm ) noexcept
{
    _normal = nullptr;
    _point = nullptr;

    if ( !lua_checkstack ( &vm, 1 ) ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::ScriptableRaycastResult::Destroy - Stack is too small." );
        return;
    }

    lua_pushnil ( &vm );
    lua_setglobal ( &vm, GLOBAL_TABLE );
}

bool ScriptableRaycastResult::PublishResult ( lua_State &vm, android_vulkan::RaycastResult const &result ) noexcept
{
    if ( !lua_checkstack ( &vm, 4 ) ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::ScriptableRaycastResult::PublishResult - Stack is too small." );
        return false;
    }

    int const errorHandlerIdx = ScriptEngine::PushErrorHandlerToStack ( vm );

    lua_getglobal ( &vm, GLOBAL_TABLE );
    lua_pushlstring ( &vm, FIELD_BODY.data (), FIELD_BODY.size () );

    if ( !RigidBodyComponent::PrepareLuaFindRigidBodyComponent ( vm ) ) [[unlikely]]
    {
        lua_pop ( &vm, 3 );
        return false;
    }

    lua_pushlightuserdata ( &vm, result._body.get () );

    if ( lua_pcall ( &vm, 1, 1, errorHandlerIdx ) != LUA_OK ) [[unlikely]]
    {
        lua_pop ( &vm, 4 );
        return false;
    }

    lua_rawset ( &vm, -3 );
    lua_replace ( &vm, -2 );

    *_point = result._point;
    *_normal = result._normal;
    return true;
}

} // namespace pbr
