#include <pbr/scriptable_penetration.h>
#include <pbr/script_engine.h>
#include <logger.h>


namespace pbr {

constexpr static std::string_view FIELD_BODY = "_body";
constexpr static std::string_view FIELD_COUNT = "_count";
constexpr static std::string_view FIELD_DEPTH = "_depth";
constexpr static std::string_view FIELD_PENETRATIONS = "_penetrations";

constexpr static char const GLOBAL_FUNCTION[] = "FindRigidBodyComponent";
constexpr static char const GLOBAL_TABLE[] = "av_scriptablePenetration";

constexpr static int INITIAL_CAPACITY = 128;

//----------------------------------------------------------------------------------------------------------------------

bool ScriptablePenetration::Init ( lua_State &vm ) noexcept
{
    if ( !lua_checkstack ( &vm, 5 ) )
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

    if ( !FindVec3Constructor ( vm ) )
    {
        lua_pop ( &vm, 2 );
        return false;
    }

    int const vec3Constructor = lua_gettop ( &vm );
    constexpr GXVec3 normal ( 0.0F, 1.0F, 0.0F );

    _normals.reserve ( static_cast<size_t> ( INITIAL_CAPACITY ) );

    // Don't care about actual value. Main task is to allocate internal Lua array with something.
    constexpr lua_Integer proxyRigidBody = 777;
    lua_pushinteger ( &vm, proxyRigidBody );
    int const rigidBodyComponentStack = lua_gettop ( &vm );

    for ( int i = 1; i <= INITIAL_CAPACITY; ++i )
    {
        if ( !Append ( vm, vec3Constructor, penetrationIndex, 0.0, normal, rigidBodyComponentStack ) )
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

    if ( !lua_checkstack ( &vm, 1 ) )
    {
        android_vulkan::LogError ( "pbr::ScriptablePenetration::Init - Stack is too small." );
        return;
    }

    lua_pushnil ( &vm );
    lua_setglobal ( &vm, GLOBAL_TABLE );
}

bool ScriptablePenetration::PublishResult ( lua_State &vm,
    std::vector<android_vulkan::Penetration> const &penetrations
) noexcept
{
    if ( !lua_checkstack ( &vm, 7 ) )
    {
        android_vulkan::LogError ( "pbr::ScriptablePenetration::PublishResult - Stack is too small." );
        return false;
    }

    size_t const count = penetrations.size ();

    lua_getglobal ( &vm, GLOBAL_TABLE );
    lua_pushlstring ( &vm, FIELD_COUNT.data (), FIELD_COUNT.size () );
    lua_pushinteger ( &vm, static_cast<int> ( count ) );
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

    lua_getglobal ( &vm, GLOBAL_FUNCTION );

    for ( size_t i = 0U; i < available; ++i )
    {
        android_vulkan::Penetration const& p = pens[ i ];
        _normals[ i ].get ()._vec3 = p._normal;

        lua_pushinteger ( &vm, static_cast<int> ( i ) + 1 );

        if ( lua_rawget ( &vm, -3 ) != LUA_TTABLE )
        {
            android_vulkan::LogError ( "pbr::ScriptablePenetration::PublishResult - Can't find array item %zu.",
                i + 1U
            );

            lua_pop ( &vm, 4 );
            return false;
        }

        lua_pushlstring ( &vm, FIELD_DEPTH.data (), FIELD_DEPTH.size () );
        lua_pushnumber ( &vm, p._depth );
        lua_rawset ( &vm, -3 );

        lua_pushlstring ( &vm, FIELD_BODY.data (), FIELD_BODY.size () );

        lua_pushvalue ( &vm, -3 );
        lua_pushlightuserdata ( &vm, p._body.get () );

        if ( lua_pcall ( &vm, 1, 1, ScriptEngine::GetErrorHandlerIndex () ) != LUA_OK )
        {
            lua_pop ( &vm, 6 );
            return false;
        }

        lua_rawset ( &vm, -3 );
        lua_pop ( &vm, 1 );
    }

    if ( count - available == 0U )
    {
        lua_pop ( &vm, 2 );
        return true;
    }

    if ( !FindVec3Constructor ( vm ) )
    {
        lua_pop ( &vm, 3 );
        return false;
    }

    int const vec3Constructor = lua_gettop ( &vm );

    for ( size_t i = available; i < count; ++i )
    {
        android_vulkan::Penetration const& p = pens[ i ];

        lua_pushvalue ( &vm, -2 );
        lua_pushlightuserdata ( &vm, p._body.get () );

        if ( lua_pcall ( &vm, 1, 1, ScriptEngine::GetErrorHandlerIndex () ) != LUA_OK )
        {
            lua_pop ( &vm, 5 );
            return false;
        }

        if ( !Append ( vm, vec3Constructor, penetrationIndex, p._depth, p._normal, lua_gettop ( &vm ) ) )
        {
            lua_pop ( &vm, 5 );
            return false;
        }

        lua_pop ( &vm, 1 );
    }

    lua_pop ( &vm, 3 );
    return true;
}

bool ScriptablePenetration::Append ( lua_State &vm,
    int vec3Constructor,
    int penetrationIndex,
    lua_Number depth,
    GXVec3 const &normal,
    int rigidBodyComponentStack
) noexcept
{
    if ( !lua_checkstack ( &vm, 5 ) )
    {
        android_vulkan::LogError ( "pbr::ScriptablePenetration::Append - Stack is too small." );
        return false;
    }

    lua_pushinteger ( &vm, static_cast<int> ( _normals.size () ) + 1 );
    lua_createtable ( &vm, 0, 2 );

    constexpr std::string_view fieldNormal = "_normal";
    constexpr std::string_view fieldHandle = "_handle";

    // Depth field append.
    lua_pushlstring ( &vm, FIELD_DEPTH.data (), FIELD_DEPTH.size () );
    lua_pushnumber ( &vm, depth );
    lua_rawset ( &vm, -3 );

    // Normal field init.
    lua_pushlstring ( &vm, fieldNormal.data (), fieldNormal.size () );

    lua_pushvalue ( &vm, vec3Constructor );

    if ( lua_pcall ( &vm, 0, 1, ScriptEngine::GetErrorHandlerIndex () ) != LUA_OK )
    {
        lua_pop ( &vm, 4 );
        return false;
    }

    lua_pushlstring ( &vm, fieldHandle.data (), fieldHandle.size () );

    if ( lua_rawget ( &vm, -2 ) != LUA_TLIGHTUSERDATA )
    {
        android_vulkan::LogError ( "pbr::ScriptablePenetration::Append - Can't find GXVec3 handle." );
        lua_pop ( &vm, 5 );
        return false;
    }

    auto& n = *static_cast<ScriptableGXVec3::Item*> ( lua_touserdata ( &vm, -1 ) );
    n._vec3 = normal;
    _normals.emplace_back ( std::ref ( n ) );
    lua_pop ( &vm, 1 );

    // Normal field append.
    lua_rawset ( &vm, -3 );

    // Rigid body component append.
    lua_pushlstring ( &vm, FIELD_BODY.data (), FIELD_BODY.size () );
    lua_pushvalue ( &vm, rigidBodyComponentStack );
    lua_rawset ( &vm, -3 );

    // Depth + Normal + Rigid body component append to _penetrations.
    lua_rawset ( &vm, penetrationIndex );
    return true;
}

bool ScriptablePenetration::FindVec3Constructor ( lua_State &vm ) noexcept
{
    if ( !lua_checkstack ( &vm, 3 ) )
    {
        android_vulkan::LogError ( "pbr::ScriptablePenetration::FindVec3Constructor - Stack is too small." );
        return false;
    }

    if ( lua_getglobal ( &vm, "GXVec3" ) != LUA_TTABLE )
    {
        android_vulkan::LogError ( "pbr::ScriptablePenetration::FindVec3Constructor - Can't find GXVec3 table." );
        lua_pop ( &vm, 1 );
        return false;
    }

    if ( lua_getmetatable ( &vm, -1 ) != 1 )
    {
        android_vulkan::LogError ( "pbr::ScriptablePenetration::FindVec3Constructor - Can't find GXVec3 metatable." );
        lua_pop ( &vm, 2 );
        return false;
    }

    constexpr std::string_view metamethodCall = "__call";
    lua_pushlstring ( &vm, metamethodCall.data (), metamethodCall.size () );

    if ( lua_rawget ( &vm, -2 ) != LUA_TFUNCTION )
    {
        android_vulkan::LogError ( "pbr::ScriptablePenetration::FindVec3Constructor - Can't find GXVec3 constructor." );
        lua_pop ( &vm, 3 );
        return false;
    }

    lua_replace ( &vm, -3 );
    lua_pop ( &vm, 1 );

    return true;
}

} // namespace pbr
