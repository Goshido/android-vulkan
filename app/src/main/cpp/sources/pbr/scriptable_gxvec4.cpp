#include <pbr/scriptable_gxvec4.hpp>
#include <logger.hpp>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

#include <cstdio>
#include <memory>

GX_RESTORE_WARNING_STATE


namespace pbr {

namespace {

// 1 Mb of data at 64bit system
constexpr size_t INITIAL_CAPACITY = 32'768U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

ScriptableGXVec4::Item* ScriptableGXVec4::_free = nullptr;
ScriptableGXVec4::Item* ScriptableGXVec4::_used = nullptr;

void ScriptableGXVec4::Init ( lua_State &vm ) noexcept
{
    for ( size_t i = 0U; i < INITIAL_CAPACITY; ++i )
        Insert ( new Item {}, _free );

    constexpr luaL_Reg const extensions[] =
    {
        {
            .name = "av_GXVec4Create",
            .func = &ScriptableGXVec4::OnCreate
        },

        {
            .name = "av_GXVec4Destroy",
            .func = &ScriptableGXVec4::OnDestroy
        },

        {
            .name = "av_GXVec4Init",
            .func = &ScriptableGXVec4::OnInit
        },

        {
            .name = "av_GXVec4ToString",
            .func = &ScriptableGXVec4::OnToString
        }
    };

    for ( auto const &extension : extensions )
    {
        lua_register ( &vm, extension.name, extension.func );
    }
}

void ScriptableGXVec4::Destroy () noexcept
{
    constexpr auto free = [] ( Item* &head ) noexcept {
        Item* item = head;

        while ( item )
        {
            Item* d = item;
            item = item->_next;
            delete d;
        }

        head = nullptr;
    };

    free ( _free );
    free ( _used );
}

GXVec4 &ScriptableGXVec4::Extract ( lua_State* state, int idx ) noexcept
{
    auto &item = *static_cast<Item*> ( lua_touserdata ( state, idx ) );
    return item._vec4;
}

void ScriptableGXVec4::Insert ( Item* item, Item* &list ) noexcept
{
    item->_previous = nullptr;
    item->_next = list;

    if ( list )
        list->_previous = item;

    list = item;
}

int ScriptableGXVec4::OnCreate ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::ScriptableGXVec4::OnCreate - Stack is too small." );
        return 0;
    }

    if ( !_free ) [[unlikely]]
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

int ScriptableGXVec4::OnDestroy ( lua_State* state )
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

int ScriptableGXVec4::OnInit ( lua_State* state )
{
    auto &item = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );

    item._vec4.Init ( static_cast<float> ( lua_tonumber ( state, 2 ) ),
        static_cast<float> ( lua_tonumber ( state, 3 ) ),
        static_cast<float> ( lua_tonumber ( state, 4 ) ),
        static_cast<float> ( lua_tonumber ( state, 5 ) )
    );

    return 0;
}

int ScriptableGXVec4::OnToString ( lua_State* state )
{
    auto const &item = *static_cast<Item const*> ( lua_touserdata ( state, 1 ) );
    GXVec4 const &v = item._vec4;

    char result[ 128U ];

    int const len = std::snprintf ( result,
        std::size ( result ),
        "%14g %14g %14g %14g",
        v._data[ 0U ],
        v._data[ 1U ],
        v._data[ 2U ],
        v._data[ 3U ]
    );

    lua_pushlstring ( state, result, static_cast<size_t> ( len ) );
    return 1;
}

} // namespace pbr
