#include <pbr/scriptable_gxvec3.h>
#include <logger.h>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

#include <cstdio>
#include <memory>

GX_RESTORE_WARNING_STATE


namespace pbr {

// ~2 Mb of data at 64bit system
constexpr static size_t INITIAL_CAPACITY = 74'898U;

//----------------------------------------------------------------------------------------------------------------------

ScriptableGXVec3::Item* ScriptableGXVec3::_free = nullptr;
ScriptableGXVec3::Item* ScriptableGXVec3::_used = nullptr;

void ScriptableGXVec3::Init ( lua_State &vm ) noexcept
{
    for ( size_t i = 0U; i < INITIAL_CAPACITY; ++i )
        Insert ( new Item {}, _free );

    constexpr luaL_Reg const extentions[] =
    {
        {
            .name = "av_GXVec3Create",
            .func = &ScriptableGXVec3::OnCreate
        },

        {
            .name = "av_GXVec3CrossProduct",
            .func = &ScriptableGXVec3::OnCrossProduct
        },

        {
            .name = "av_GXVec3Destroy",
            .func = &ScriptableGXVec3::OnDestroy
        },

        {
            .name = "av_GXVec3Distance",
            .func = &ScriptableGXVec3::OnDistance
        },

        {
            .name = "av_GXVec3DotProduct",
            .func = &ScriptableGXVec3::OnDotProduct
        },

        {
            .name = "av_GXVec3Init",
            .func = &ScriptableGXVec3::OnInit
        },

        {
            .name = "av_GXVec3Length",
            .func = &ScriptableGXVec3::OnLength
        },

        {
            .name = "av_GXVec3MultiplyScalar",
            .func = &ScriptableGXVec3::OnMultiplyScalar
        },

        {
            .name = "av_GXVec3Normalize",
            .func = &ScriptableGXVec3::OnNormalize
        },

        {
            .name = "av_GXVec3Reverse",
            .func = &ScriptableGXVec3::OnReverse
        },

        {
            .name = "av_GXVec3SquaredDistance",
            .func = &ScriptableGXVec3::OnSquaredDistance
        },

        {
            .name = "av_GXVec3SquaredLength",
            .func = &ScriptableGXVec3::OnSquaredLength
        },

        {
            .name = "av_GXVec3Subtract",
            .func = &ScriptableGXVec3::OnSubtract
        },

        {
            .name = "av_GXVec3Sum",
            .func = &ScriptableGXVec3::OnSum
        },

        {
            .name = "av_GXVec3SumScaled",
            .func = &ScriptableGXVec3::OnSumScaled
        },

        {
            .name = "av_GXVec3ToString",
            .func = &ScriptableGXVec3::OnToString
        }
    };

    for ( auto const& extension : extentions )
    {
        lua_register ( &vm, extension.name, extension.func );
    }
}

void ScriptableGXVec3::Destroy () noexcept
{
    auto free = [] ( Item*& head ) noexcept {
        Item* item = head;

        while ( item )
        {
            Item* d = item;
            item = item->_next;
            delete ( d );
        }

        head = nullptr;
    };

    free ( _free );
    free ( _used );
}

GXVec3& ScriptableGXVec3::Extract ( lua_State* state, int idx ) noexcept
{
    auto& item = *static_cast<Item*> ( lua_touserdata ( state, idx ) );
    return item._vec3;
}

void ScriptableGXVec3::Insert ( Item* item, Item*& list ) noexcept
{
    item->_previous = nullptr;
    item->_next = list;

    if ( list )
        list->_previous = item;

    list = item;
}

int ScriptableGXVec3::OnCreate ( lua_State* state )
{
    if ( !_free )
    {
        Insert ( new Item {}, _used );
        lua_pushlightuserdata ( state, _used );
        return 1;
    }

    Item* item = _free;
    _free = _free->_next;

    if ( _free )
        _free->_previous = nullptr;

    Insert ( item, _used );
    lua_pushlightuserdata ( state, item );
    return 1;
}

int ScriptableGXVec3::OnCrossProduct ( lua_State* state )
{
    auto& self = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    auto const& a = *static_cast<Item const*> ( lua_touserdata ( state, 2 ) );
    auto const& b = *static_cast<Item const*> ( lua_touserdata ( state, 3 ) );

    self._vec3.CrossProduct ( a._vec3, b._vec3 );
    return 0;
}

int ScriptableGXVec3::OnDestroy ( lua_State* state )
{
    auto* item = static_cast<Item*> ( lua_touserdata ( state, 1 ) );

    Item* p = item->_previous;
    Item* n = item->_next;

    if ( p )
        p->_next = n;

    if ( n )
        n->_previous = p;

    // Branchless optimization.
    Item* cases[ 2U ] = { _used, n };
    _used = cases[ static_cast<size_t> ( item == _used ) ];

    Insert ( item, _free );
    return 0;
}

int ScriptableGXVec3::OnDistance ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) )
    {
        android_vulkan::LogWarning ( "pbr::ScriptableGXVec3::OnDistance - Stack too small." );
        return 0;
    }

    auto const& self = *static_cast<Item const*> ( lua_touserdata ( state, 1 ) );
    auto const& other = *static_cast<Item const*> ( lua_touserdata ( state, 2 ) );
    lua_pushnumber ( state, static_cast<lua_Number> ( self._vec3.Distance ( other._vec3 ) ) );

    return 1;
}

int ScriptableGXVec3::OnDotProduct ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) )
    {
        android_vulkan::LogWarning ( "pbr::ScriptableGXVec3::OnDotProduct - Stack too small." );
        return 0;
    }

    auto const& self = *static_cast<Item const*> ( lua_touserdata ( state, 1 ) );
    auto const& other = *static_cast<Item const*> ( lua_touserdata ( state, 2 ) );
    lua_pushnumber ( state, static_cast<lua_Number> ( self._vec3.DotProduct ( other._vec3 ) ) );

    return 1;
}

int ScriptableGXVec3::OnInit ( lua_State* state )
{
    auto& item = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );

    item._vec3.Init ( static_cast<float> ( lua_tonumber ( state, 2 ) ),
        static_cast<float> ( lua_tonumber ( state, 3 ) ),
        static_cast<float> ( lua_tonumber ( state, 4 ) )
    );

    return 0;
}

int ScriptableGXVec3::OnLength ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) )
    {
        android_vulkan::LogWarning ( "pbr::ScriptableGXVec3::OnLength - Stack too small." );
        return 0;
    }

    auto const& item = *static_cast<Item const*> ( lua_touserdata ( state, 1 ) );
    lua_pushnumber ( state, static_cast<lua_Number> ( item._vec3.Length () ) );
    return 1;
}

int ScriptableGXVec3::OnMultiplyScalar ( lua_State* state )
{
    auto& self = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    auto const& a = *static_cast<Item const*> ( lua_touserdata ( state, 2 ) );

    self._vec3.Multiply ( a._vec3, static_cast<GXFloat> ( lua_tonumber ( state, 3 ) ) );
    return 0;
}

int ScriptableGXVec3::OnNormalize ( lua_State* state )
{
    auto& item = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    item._vec3.Normalize ();
    return 0;
}

int ScriptableGXVec3::OnReverse ( lua_State* state )
{
    auto& item = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    item._vec3.Reverse ();
    return 0;
}

int ScriptableGXVec3::OnSquaredDistance ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) )
    {
        android_vulkan::LogWarning ( "pbr::ScriptableGXVec3::OnSquaredDistance - Stack too small." );
        return 0;
    }

    auto const& self = *static_cast<Item const*> ( lua_touserdata ( state, 1 ) );
    auto const& other = *static_cast<Item const*> ( lua_touserdata ( state, 2 ) );
    lua_pushnumber ( state, static_cast<lua_Number> ( self._vec3.SquaredDistance ( other._vec3 ) ) );
    return 1;
}

int ScriptableGXVec3::OnSquaredLength ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) )
    {
        android_vulkan::LogWarning ( "pbr::ScriptableGXVec3::OnSquaredLength - Stack too small." );
        return 0;
    }

    auto const& item = *static_cast<Item const*> ( lua_touserdata ( state, 1 ) );
    lua_pushnumber ( state, static_cast<lua_Number> ( item._vec3.SquaredLength () ) );
    return 1;
}

int ScriptableGXVec3::OnSubtract ( lua_State* state )
{
    auto& self = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    auto const& a = *static_cast<Item const*> ( lua_touserdata ( state, 2 ) );
    auto const& b = *static_cast<Item const*> ( lua_touserdata ( state, 3 ) );

    self._vec3.Subtract ( a._vec3, b._vec3 );
    return 0;
}

int ScriptableGXVec3::OnSum ( lua_State* state )
{
    auto& self = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    auto const& a = *static_cast<Item const*> ( lua_touserdata ( state, 2 ) );
    auto const& b = *static_cast<Item const*> ( lua_touserdata ( state, 3 ) );

    self._vec3.Sum ( a._vec3, b._vec3 );
    return 0;
}

int ScriptableGXVec3::OnSumScaled ( lua_State* state )
{
    auto& self = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    auto const& a = *static_cast<Item const*> ( lua_touserdata ( state, 2 ) );
    auto const& b = *static_cast<Item const*> ( lua_touserdata ( state, 4 ) );

    self._vec3.Sum ( a._vec3, static_cast<float> ( lua_tonumber ( state, 3 ) ), b._vec3 );
    return 0;
}

int ScriptableGXVec3::OnToString ( lua_State* state )
{
    auto const& item = *static_cast<Item const*> ( lua_touserdata ( state, 1 ) );
    GXVec3 const& v = item._vec3;

    char result[ 128U ];

    int const len = std::snprintf ( result,
        std::size ( result ),
        "%14g %14g %14g",
        v._data[ 0U ],
        v._data[ 1U ],
        v._data[ 2U ]
    );

    lua_pushlstring ( state, result, static_cast<size_t> ( len ) );
    return 1;
}

} // namespace pbr
