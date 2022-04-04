#include <pbr/scriptable_gxmat3.h>
#include <pbr/scriptable_gxvec3.h>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

#include <cstdio>
#include <memory>

GX_RESTORE_WARNING_STATE


namespace pbr {

// ~2 Mb of data at 64bit system
constexpr static size_t INITIAL_CAPACITY = 40'329U;

//----------------------------------------------------------------------------------------------------------------------

ScriptableGXMat3::Item* ScriptableGXMat3::_free = nullptr;
ScriptableGXMat3::Item* ScriptableGXMat3::_used = nullptr;

void ScriptableGXMat3::Init ( lua_State* vm ) noexcept
{
    for ( size_t i = 0U; i < INITIAL_CAPACITY; ++i )
        Insert ( new Item {}, _free );

    luaL_Reg const extentions[] =
    {
        {
            .name = "av_GXMat3Create",
            .func = &ScriptableGXMat3::OnCreate
        },

        {
            .name = "av_GXMat3Destroy",
            .func = &ScriptableGXMat3::OnDestroy
        },

        {
            .name = "av_GXMat3GetX",
            .func = &ScriptableGXMat3::OnGetX
        },

        {
            .name = "av_GXMat3GetY",
            .func = &ScriptableGXMat3::OnGetY
        },

        {
            .name = "av_GXMat3GetZ",
            .func = &ScriptableGXMat3::OnGetZ
        },

        {
            .name = "av_GXMat3Identity",
            .func = &ScriptableGXMat3::OnIdentity
        },

        {
            .name = "av_GXMat3ToString",
            .func = &ScriptableGXMat3::OnToString
        }
    };

    for ( auto const& extension : extentions )
    {
        lua_register ( vm, extension.name, extension.func );
    }
}

void ScriptableGXMat3::Destroy () noexcept
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

void ScriptableGXMat3::Insert ( Item* item, Item*& list ) noexcept
{
    item->_previous = nullptr;
    item->_next = list;

    if ( list )
        list->_previous = item;

    list = item;
}

int ScriptableGXMat3::OnCreate ( lua_State* state )
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

int ScriptableGXMat3::OnDestroy ( lua_State* state )
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

int ScriptableGXMat3::OnGetX ( lua_State* state )
{
    auto const& self = *static_cast<Item const*> ( lua_touserdata ( state, 1 ) );
    self._matrix.GetX ( ScriptableGXVec3::Extract ( state, 2 ) );
    return 0;
}

int ScriptableGXMat3::OnGetY ( lua_State* state )
{
    auto const& self = *static_cast<Item const*> ( lua_touserdata ( state, 1 ) );
    self._matrix.GetY ( ScriptableGXVec3::Extract ( state, 2 ) );
    return 0;
}

int ScriptableGXMat3::OnGetZ ( lua_State* state )
{
    auto const& self = *static_cast<Item const*> ( lua_touserdata ( state, 1 ) );
    self._matrix.GetZ ( ScriptableGXVec3::Extract ( state, 2 ) );
    return 0;
}

int ScriptableGXMat3::OnIdentity ( lua_State* state )
{
    auto& item = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    item._matrix.Identity ();
    return 0;
}

int ScriptableGXMat3::OnToString ( lua_State* state )
{
    auto const* item = static_cast<Item const*> ( lua_touserdata ( state, 1 ) );
    GXMat3 const& m = item->_matrix;

    constexpr char const format[] =
R"__(%14g %14g %14g
%14g %14g %14g
%14g %14g %14g)__";

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
        m._data[ 8U ]
    );

    lua_pushlstring ( state, result, static_cast<size_t> ( len ) );
    return 1;
}

} // namespace pbr
