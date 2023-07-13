#include <pbr/scriptable_gxquat.hpp>
#include <pbr/scriptable_gxvec3.hpp>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

#include <cstdio>
#include <memory>

GX_RESTORE_WARNING_STATE


namespace pbr {

// 1 Mb of data at 64bit system
constexpr static size_t INITIAL_CAPACITY = 32'768U;

//----------------------------------------------------------------------------------------------------------------------

ScriptableGXQuat::Item* ScriptableGXQuat::_free = nullptr;
ScriptableGXQuat::Item* ScriptableGXQuat::_used = nullptr;

void ScriptableGXQuat::Init ( lua_State &vm ) noexcept
{
    for ( size_t i = 0U; i < INITIAL_CAPACITY; ++i )
        Insert ( new Item {}, _free );

    constexpr luaL_Reg const extensions[] =
    {
        {
            .name = "av_GXQuatCreate",
            .func = &ScriptableGXQuat::OnCreate
        },

        {
            .name = "av_GXQuatDestroy",
            .func = &ScriptableGXQuat::OnDestroy
        },

        {
            .name = "av_GXQuatFromAxisAngle",
            .func = &ScriptableGXQuat::OnFromAxisAngle
        },

        {
            .name = "av_GXQuatInverse",
            .func = &ScriptableGXQuat::OnInverse
        },

        {
            .name = "av_GXQuatInverseFast",
            .func = &ScriptableGXQuat::OnInverseFast
        },

        {
            .name = "av_GXQuatMultiply",
            .func = &ScriptableGXQuat::OnMultiply
        },

        {
            .name = "av_GXQuatNormalize",
            .func = &ScriptableGXQuat::OnNormalize
        },

        {
            .name = "av_GXQuatToString",
            .func = &ScriptableGXQuat::OnToString
        },

        {
            .name = "av_GXQuatSphericalLinearInterpolation",
            .func = &ScriptableGXQuat::OnSphericalLinearInterpolation
        },

        {
            .name = "av_GXQuatTransformFast",
            .func = &ScriptableGXQuat::OnTransformFast
        }
    };

    for ( auto const &extension : extensions )
    {
        lua_register ( &vm, extension.name, extension.func );
    }
}

void ScriptableGXQuat::Destroy () noexcept
{
    auto free = [] ( Item* &head ) noexcept {
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

GXQuat &ScriptableGXQuat::Extract ( lua_State* state, int idx ) noexcept
{
    auto &item = *static_cast<Item*> ( lua_touserdata ( state, idx ) );
    return item._quaternion;
}

void ScriptableGXQuat::Insert ( Item* item, Item* &list ) noexcept
{
    item->_previous = nullptr;
    item->_next = list;

    if ( list )
        list->_previous = item;

    list = item;
}

int ScriptableGXQuat::OnCreate ( lua_State* state )
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

int ScriptableGXQuat::OnDestroy ( lua_State* state )
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

int ScriptableGXQuat::OnFromAxisAngle ( lua_State* state )
{
    auto &item = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );

    item._quaternion.FromAxisAngle ( ScriptableGXVec3::Extract ( state, 2 ),
        static_cast<float> ( lua_tonumber ( state, 3 ) )
    );

    return 0;
}

int ScriptableGXQuat::OnInverse ( lua_State* state )
{
    auto &self = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    auto const &q = *static_cast<Item const*> ( lua_touserdata ( state, 2 ) );
    self._quaternion.Inverse ( q._quaternion );

    return 0;
}

int ScriptableGXQuat::OnInverseFast ( lua_State* state )
{
    auto &self = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    auto const &unitQuaternion = *static_cast<Item const*> ( lua_touserdata ( state, 2 ) );
    self._quaternion.InverseFast ( unitQuaternion._quaternion );

    return 0;
}

int ScriptableGXQuat::OnMultiply ( lua_State* state )
{
    auto &self = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    auto const &a = *static_cast<Item const*> ( lua_touserdata ( state, 2 ) );
    auto const &b = *static_cast<Item const*> ( lua_touserdata ( state, 3 ) );
    self._quaternion.Multiply ( a._quaternion, b._quaternion );

    return 0;
}

int ScriptableGXQuat::OnNormalize ( lua_State* state )
{
    auto &item = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    item._quaternion.Normalize ();
    return 0;
}

int ScriptableGXQuat::OnSphericalLinearInterpolation ( lua_State* state )
{
    auto &self = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    auto const &start = *static_cast<Item const*> ( lua_touserdata ( state, 2 ) );
    auto const &finish = *static_cast<Item const*> ( lua_touserdata ( state, 3 ) );

    self._quaternion.SphericalLinearInterpolation ( start._quaternion,
        finish._quaternion,
        static_cast<float> ( lua_tonumber ( state, 4 ) )
    );

    return 0;
}

int ScriptableGXQuat::OnToString ( lua_State* state )
{
    auto const &item = *static_cast<Item const*> ( lua_touserdata ( state, 1 ) );
    GXQuat const &q = item._quaternion;

    char result[ 128U ];

    int const len = std::snprintf ( result,
        std::size ( result ),
        "%14g %14g %14g %14g",
        q._data[ 0U ],
        q._data[ 1U ],
        q._data[ 2U ],
        q._data[ 3U ]
    );

    lua_pushlstring ( state, result, static_cast<size_t> ( len ) );
    return 1;
}

int ScriptableGXQuat::OnTransformFast ( lua_State* state )
{
    auto const &self = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    self._quaternion.TransformFast ( ScriptableGXVec3::Extract ( state, 2 ), ScriptableGXVec3::Extract ( state, 3 ) );
    return 0;
}

} // namespace pbr
