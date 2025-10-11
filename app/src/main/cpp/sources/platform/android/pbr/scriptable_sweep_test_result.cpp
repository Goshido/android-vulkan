#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <platform/android/pbr/scriptable_sweep_test_result.hpp>
#include <platform/android/pbr/script_engine.hpp>


namespace pbr {

namespace {

constexpr std::string_view FIELD_COUNT = "_count";
constexpr std::string_view FIELD_BODIES = "_bodies";

constexpr char const GLOBAL_FUNCTION[] = "FindRigidBodyComponent";
constexpr char const GLOBAL_TABLE[] = "av_scriptableSweepTestResult";

constexpr int INITIAL_CAPACITY = 128;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

bool ScriptableSweepTestResult::Init ( lua_State &vm ) noexcept
{
    if ( !lua_checkstack ( &vm, 4 ) ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::ScriptableSweepTestResult::Init - Stack is too small." );
        return false;
    }

    lua_createtable ( &vm, 0, 2 );
    lua_setglobal ( &vm, GLOBAL_TABLE );
    lua_getglobal ( &vm, GLOBAL_TABLE );

    lua_pushlstring ( &vm, FIELD_COUNT.data (), FIELD_COUNT.size () );
    lua_pushinteger ( &vm, 0 );
    lua_rawset ( &vm, -3 );

    lua_pushlstring ( &vm, FIELD_BODIES.data (), FIELD_BODIES.size () );
    lua_createtable ( &vm, INITIAL_CAPACITY, 0 );
    lua_rawset ( &vm, -3 );

    lua_pushlstring ( &vm, FIELD_BODIES.data (), FIELD_BODIES.size () );
    lua_rawget ( &vm, -2 );

    // Don't care about actual value. Main task is to allocate internal Lua array with something.
    constexpr lua_Integer proxyRigidBody = 777;

    for ( int i = 1; i <= INITIAL_CAPACITY; ++i )
    {
        lua_pushinteger ( &vm, i );
        lua_pushinteger ( &vm, proxyRigidBody );
        lua_rawset ( &vm, -3 );
    }

    lua_pop ( &vm, 2 );
    return true;
}

void ScriptableSweepTestResult::Destroy ( lua_State &vm ) noexcept
{
    if ( !lua_checkstack ( &vm, 1 ) ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::ScriptableSweepTestResult::Init - Stack is too small." );
        return;
    }

    lua_pushnil ( &vm );
    lua_setglobal ( &vm, GLOBAL_TABLE );
}

bool ScriptableSweepTestResult::PublishResult ( lua_State &vm,
    std::vector<android_vulkan::RigidBodyRef> const &sweepTestResult
) noexcept
{
    if ( !lua_checkstack ( &vm, 6 ) ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::ScriptableSweepTestResult::PublishResult - Stack is too small." );
        return false;
    }

    size_t const count = sweepTestResult.size ();

    lua_getglobal ( &vm, GLOBAL_TABLE );
    lua_pushlstring ( &vm, FIELD_COUNT.data (), FIELD_COUNT.size () );
    lua_pushinteger ( &vm, static_cast<int> ( count ) );
    lua_rawset ( &vm, -3 );

    if ( count == 0U )
        return true;

    lua_pushlstring ( &vm, FIELD_BODIES.data (), FIELD_BODIES.size () );
    lua_rawget ( &vm, -2 );

    lua_getglobal ( &vm, GLOBAL_FUNCTION );

    for ( size_t i = 0U; i < count; ++i )
    {
        android_vulkan::RigidBodyRef const &body = sweepTestResult[ i ];
        lua_pushinteger ( &vm, static_cast<int> ( i + 1U ) );

        lua_pushvalue ( &vm, -2 );
        lua_pushlightuserdata ( &vm, body.get () );

        if ( lua_pcall ( &vm, 1, 1, ScriptEngine::GetErrorHandlerIndex () ) != LUA_OK ) [[unlikely]]
        {
            lua_pop ( &vm, 5 );
            return false;
        }

        lua_rawset ( &vm, -4 );
    }

    lua_pop ( &vm, 2 );
    return true;
}

} // namespace pbr
