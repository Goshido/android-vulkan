#include <precompiled_headers.hpp>
#include <pbr/rigid_body_component.hpp>
#include <pbr/script_engine.hpp>
#include <pbr/scriptable_gxvec3.hpp>
#include <pbr/scriptable_penetration.hpp>
#include <logger.hpp>


namespace pbr {

namespace {

constexpr std::string_view FIELD_BODY = "_body";
constexpr std::string_view FIELD_COUNT = "_count";
constexpr std::string_view FIELD_DEPTH = "_depth";
constexpr std::string_view FIELD_PENETRATIONS = "_penetrations";
constexpr std::string_view FIELD_NORMAL = "_normal";

constexpr char const GLOBAL_TABLE[] = "av_scriptablePenetration";

constexpr int INITIAL_CAPACITY = 128;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

bool ScriptablePenetration::Init ( lua_State &vm ) noexcept
{
    if ( !lua_checkstack ( &vm, 5 ) ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::ScriptablePenetration::Init - Stack is too small." );
        return false;
    }

    lua_createtable ( &vm, 0, 2 );
    lua_setglobal ( &vm, GLOBAL_TABLE );
    lua_getglobal ( &vm, GLOBAL_TABLE );

    lua_pushlstring ( &vm, FIELD_COUNT.data (), FIELD_COUNT.size () );
    lua_pushinteger ( &vm, 0 );
    lua_rawset ( &vm, -3 );

    lua_pushlstring ( &vm, FIELD_PENETRATIONS.data (), FIELD_PENETRATIONS.size () );
    lua_createtable ( &vm, INITIAL_CAPACITY, 0 );
    lua_rawset ( &vm, -3 );

    lua_pushlstring ( &vm, FIELD_PENETRATIONS.data (), FIELD_PENETRATIONS.size () );
    lua_rawget ( &vm, -2 );
    int const penetrationIndex = lua_gettop ( &vm );

    if ( !ScriptableGXVec3::PrepareLuaConstructor ( vm ) ) [[unlikely]]
    {
        lua_pop ( &vm, 2 );
        return false;
    }

    int const vec3Constructor = lua_gettop ( &vm );
    constexpr int errorHandlerIdx = ScriptEngine::GetErrorHandlerIndex ();
    constexpr GXVec3 normal ( 0.0F, 1.0F, 0.0F );

    _normals.reserve ( static_cast<size_t> ( INITIAL_CAPACITY ) );

    // Don't care about actual value. Main task is to allocate internal Lua array with something.
    constexpr lua_Integer proxyRigidBody = 777;
    lua_pushinteger ( &vm, proxyRigidBody );
    int const rigidBodyComponentIdx = lua_gettop ( &vm );

    for ( int i = 1; i <= INITIAL_CAPACITY; ++i )
    {
        bool const result = Append ( vm,
            errorHandlerIdx,
            vec3Constructor,
            penetrationIndex,
            rigidBodyComponentIdx,
            0.0F,
            normal
        );

        if ( !result ) [[unlikely]]
        {
            lua_pop ( &vm, 4 );
            return false;
        }
    }

    lua_pop ( &vm, 4 );
    return true;
}

void ScriptablePenetration::Destroy ( lua_State &vm ) noexcept
{
    _normals.clear ();
    _normals.shrink_to_fit ();

    if ( !lua_checkstack ( &vm, 1 ) ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::ScriptablePenetration::Destroy - Stack is too small." );
        return;
    }

    lua_pushnil ( &vm );
    lua_setglobal ( &vm, GLOBAL_TABLE );
}

bool ScriptablePenetration::PublishResult ( lua_State &vm,
    std::vector<android_vulkan::Penetration> const &penetrations
) noexcept
{
    if ( !lua_checkstack ( &vm, 7 ) ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::ScriptablePenetration::PublishResult - Stack is too small." );
        return false;
    }

    size_t const count = penetrations.size ();

    lua_getglobal ( &vm, GLOBAL_TABLE );
    lua_pushlstring ( &vm, FIELD_COUNT.data (), FIELD_COUNT.size () );
    lua_pushinteger ( &vm, static_cast<lua_Integer> ( count ) );
    lua_rawset ( &vm, -3 );

    if ( count == 0U )
        return true;

    lua_pushlstring ( &vm, FIELD_PENETRATIONS.data (), FIELD_PENETRATIONS.size () );
    lua_rawget ( &vm, -2 );
    int const penetrationIndex = lua_gettop ( &vm );

    size_t const capacity = _normals.size ();
    size_t const cases[] = { capacity, count };
    size_t const available = cases[ static_cast<size_t> ( count <= capacity ) ];
    android_vulkan::Penetration const* pens = penetrations.data ();

    if ( !RigidBodyComponent::PrepareLuaFindRigidBodyComponent ( vm ) ) [[unlikely]]
    {
        lua_pop ( &vm, 2 );
        return false;
    }

    int const findRigidBodyIdx = lua_gettop ( &vm );
    int const errorHandlerIdx = ScriptEngine::PushErrorHandlerToStack ( vm );

    for ( size_t i = 0U; i < available; ++i )
    {
        android_vulkan::Penetration const &p = pens[ i ];
        *( _normals[ i ] ) = p._normal;

        lua_pushinteger ( &vm, static_cast<int> ( i ) + 1 );

        if ( lua_rawget ( &vm, penetrationIndex ) != LUA_TTABLE ) [[unlikely]]
        {
            android_vulkan::LogError ( "pbr::ScriptablePenetration::PublishResult - Can't find array item %zu.",
                i + 1U
            );

            lua_pop ( &vm, 5 );
            return false;
        }

        lua_pushlstring ( &vm, FIELD_DEPTH.data (), FIELD_DEPTH.size () );
        lua_pushnumber ( &vm, p._depth );
        lua_rawset ( &vm, -3 );

        lua_pushlstring ( &vm, FIELD_BODY.data (), FIELD_BODY.size () );

        lua_pushvalue ( &vm, findRigidBodyIdx );
        lua_pushlightuserdata ( &vm, p._body.get () );

        if ( lua_pcall ( &vm, 1, 1, errorHandlerIdx ) != LUA_OK ) [[unlikely]]
        {
            lua_pop ( &vm, 6 );
            return false;
        }

        lua_rawset ( &vm, -3 );
        lua_pop ( &vm, 1 );
    }

    if ( count - available == 0U )
    {
        lua_pop ( &vm, 3 );
        return true;
    }

    if ( !ScriptableGXVec3::PrepareLuaConstructor ( vm ) ) [[unlikely]]
    {
        lua_pop ( &vm, 4 );
        return false;
    }

    int const vec3Constructor = lua_gettop ( &vm );

    for ( size_t i = available; i < count; ++i )
    {
        android_vulkan::Penetration const &p = pens[ i ];

        lua_pushvalue ( &vm, findRigidBodyIdx );
        lua_pushlightuserdata ( &vm, p._body.get () );

        if ( lua_pcall ( &vm, 1, 1, errorHandlerIdx ) != LUA_OK ) [[unlikely]]
        {
            lua_pop ( &vm, 6 );
            return false;
        }

        bool const result = Append ( vm,
            errorHandlerIdx,
            vec3Constructor,
            penetrationIndex,
            lua_gettop ( &vm ),
            p._depth,
            p._normal
        );

        if ( !result ) [[unlikely]]
        {
            lua_pop ( &vm, 6 );
            return false;
        }

        lua_pop ( &vm, 1 );
    }

    lua_pop ( &vm, 4 );
    return true;
}

bool ScriptablePenetration::Append ( lua_State &vm,
    int errorHandlerIdx,
    int vec3Constructor,
    int penetrationIndex,
    int rigidBodyComponentIdx,
    lua_Number depth,
    GXVec3 const &normal
) noexcept
{
    if ( !lua_checkstack ( &vm, 5 ) ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::ScriptablePenetration::Append - Stack is too small." );
        return false;
    }

    lua_pushinteger ( &vm, static_cast<int> ( _normals.size () ) + 1 );
    lua_createtable ( &vm, 0, 2 );

    // Depth field append.
    lua_pushlstring ( &vm, FIELD_DEPTH.data (), FIELD_DEPTH.size () );
    lua_pushnumber ( &vm, depth );
    lua_rawset ( &vm, -3 );

    // Normal field init.
    lua_pushlstring ( &vm, FIELD_NORMAL.data (), FIELD_NORMAL.size () );

    lua_pushvalue ( &vm, vec3Constructor );

    if ( lua_pcall ( &vm, 0, 1, errorHandlerIdx ) != LUA_OK ) [[unlikely]]
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

    GXVec3* n = *probe;
    *n = normal;
    _normals.emplace_back ( n );

    // Normal field append.
    lua_rawset ( &vm, -3 );

    // Rigid body component append.
    lua_pushlstring ( &vm, FIELD_BODY.data (), FIELD_BODY.size () );
    lua_pushvalue ( &vm, rigidBodyComponentIdx );
    lua_rawset ( &vm, -3 );

    // Depth + Normal + Rigid body component append to _penetrations.
    lua_rawset ( &vm, penetrationIndex );
    return true;
}

} // namespace pbr
