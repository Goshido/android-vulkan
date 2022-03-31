﻿#include <pbr/scriptable_gxmat4.h>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

#include <cstdio>
#include <memory>

GX_RESTORE_WARNING_STATE


namespace pbr {

// ~10 Mb of data at 64bit system
constexpr static size_t INITIAL_CAPACITY = 131072;

//----------------------------------------------------------------------------------------------------------------------

ScriptableGXMat4::Item* ScriptableGXMat4::_free = nullptr;
ScriptableGXMat4::Item* ScriptableGXMat4::_used = nullptr;

void ScriptableGXMat4::Init ( lua_State* vm ) noexcept
{
    for ( size_t i = 0U; i < INITIAL_CAPACITY; ++i )
        Insert ( new Item {}, _free );

    luaL_Reg const extentions[] =
    {
        {
            .name = "av_GXMat4Create",
            .func = &ScriptableGXMat4::OnCreate
        },

        {
            .name = "av_GXMat4Destroy",
            .func = &ScriptableGXMat4::OnDestroy
        },

        {
            .name = "av_GXMat4Identity",
            .func = &ScriptableGXMat4::OnIdentity
        },

        {
            .name = "av_GXMat4Perspective",
            .func = &ScriptableGXMat4::OnPerspective
        },

        {
            .name = "av_GXMat4RotationX",
            .func = &ScriptableGXMat4::OnRotationX
        },

        {
            .name = "av_GXMat4Scale",
            .func = &ScriptableGXMat4::OnScale
        },

        {
            .name = "av_GXMat4ToString",
            .func = &ScriptableGXMat4::OnToString
        }
    };

    for ( auto const& extension : extentions )
    {
        lua_register ( vm, extension.name, extension.func );
    }
}

void ScriptableGXMat4::Destroy () noexcept
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

void ScriptableGXMat4::Insert ( Item* item, Item*& list ) noexcept
{
    item->_previous = nullptr;
    item->_next = list;

    if ( list )
        list->_previous = item;

    list = item;
}

int ScriptableGXMat4::OnCreate ( lua_State* state )
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

int ScriptableGXMat4::OnDestroy ( lua_State* state )
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

int ScriptableGXMat4::OnIdentity ( lua_State* state )
{
    auto& item = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    item._matrix.Identity ();
    return 0;
}

int ScriptableGXMat4::OnPerspective ( lua_State* state )
{
    auto& item = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );

    item._matrix.Perspective ( static_cast<float> ( lua_tonumber ( state, 2 ) ),
        static_cast<float> ( lua_tonumber ( state, 3 ) ),
        static_cast<float> ( lua_tonumber ( state, 4 ) ),
        static_cast<float> ( lua_tonumber ( state, 5 ) )
    );

    return 0;
}

int ScriptableGXMat4::OnRotationX ( lua_State* state )
{
    auto& item = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    item._matrix.RotationX ( static_cast<float> ( lua_tonumber ( state, 2 ) ) );
    return 0;
}

int ScriptableGXMat4::OnScale ( lua_State* state )
{
    auto& item = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );

    item._matrix.Scale ( static_cast<float> ( lua_tonumber ( state, 2 ) ),
        static_cast<float> ( lua_tonumber ( state, 3 ) ),
        static_cast<float> ( lua_tonumber ( state, 4 ) )
    );

    return 0;
}

int ScriptableGXMat4::OnToString ( lua_State* state )
{
    auto const* item = static_cast<Item const*> ( lua_touserdata ( state, 1 ) );
    GXMat4 const& m = item->_matrix;

    constexpr char const format[] =
R"__(%14g %14g %14g %14g
%14g %14g %14g %14g
%14g %14g %14g %14g
%14g %14g %14g %14g)__";

    char result[ 512U ];

    int const len = std::snprintf ( result,
        std::size ( result ),
        format,
        m._data[ 0U ],
        m._data[ 1U ],
        m._data[ 2U ],
        m._data[ 3U ],
        m._data[ 4U ],
        m._data[ 5U ],
        m._data[ 6U ],
        m._data[ 7U ],
        m._data[ 8U ],
        m._data[ 9U ],
        m._data[ 10U ],
        m._data[ 11U ],
        m._data[ 12U ],
        m._data[ 13U ],
        m._data[ 14U ],
        m._data[ 15U ]
    );

    lua_pushlstring ( state, result, static_cast<size_t> ( len ) );
    return 1;
}

} // namespace pbr
