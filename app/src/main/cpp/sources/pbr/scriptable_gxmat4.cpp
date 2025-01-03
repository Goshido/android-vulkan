﻿#include <precompiled_headers.hpp>
#include <pbr/scriptable_gxmat4.hpp>
#include <pbr/scriptable_gxquat.hpp>
#include <pbr/scriptable_gxvec3.hpp>
#include <pbr/scriptable_gxvec4.hpp>
#include <logger.hpp>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

namespace {

// ~10 Mb of data at 64bit system
constexpr size_t INITIAL_CAPACITY = 131'072U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

ScriptableGXMat4::Item* ScriptableGXMat4::_free = nullptr;
ScriptableGXMat4::Item* ScriptableGXMat4::_used = nullptr;

void ScriptableGXMat4::Init ( lua_State &vm ) noexcept
{
    for ( size_t i = 0U; i < INITIAL_CAPACITY; ++i )
        Insert ( new Item {}, _free );

    constexpr luaL_Reg const extensions[] =
    {
        {
            .name = "av_GXMat4Clone",
            .func = &ScriptableGXMat4::OnClone
        },
        {
            .name = "av_GXMat4Create",
            .func = &ScriptableGXMat4::OnCreate
        },
        {
            .name = "av_GXMat4Destroy",
            .func = &ScriptableGXMat4::OnDestroy
        },
        {
            .name = "av_GXMat4FromFast",
            .func = &ScriptableGXMat4::OnFromFast
        },
        {
            .name = "av_GXMat4GetX",
            .func = &ScriptableGXMat4::OnGetX
        },
        {
            .name = "av_GXMat4GetY",
            .func = &ScriptableGXMat4::OnGetY
        },
        {
            .name = "av_GXMat4GetZ",
            .func = &ScriptableGXMat4::OnGetZ
        },
        {
            .name = "av_GXMat4GetW",
            .func = &ScriptableGXMat4::OnGetW
        },
        {
            .name = "av_GXMat4Identity",
            .func = &ScriptableGXMat4::OnIdentity
        },
        {
            .name = "av_GXMat4Inverse",
            .func = &ScriptableGXMat4::OnInverse
        },
        {
            .name = "av_GXMat4Multiply",
            .func = &ScriptableGXMat4::OnMultiply
        },
        {
            .name = "av_GXMat4MultiplyMatrixVector",
            .func = &ScriptableGXMat4::OnMultiplyMatrixVector
        },
        {
            .name = "av_GXMat4MultiplyAsNormal",
            .func = &ScriptableGXMat4::OnMultiplyAsNormal
        },
        {
            .name = "av_GXMat4MultiplyAsPoint",
            .func = &ScriptableGXMat4::OnMultiplyAsPoint
        },
        {
            .name = "av_GXMat4MultiplyVectorMatrix",
            .func = &ScriptableGXMat4::OnMultiplyVectorMatrix
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
            .name = "av_GXMat4RotationY",
            .func = &ScriptableGXMat4::OnRotationY
        },
        {
            .name = "av_GXMat4RotationZ",
            .func = &ScriptableGXMat4::OnRotationZ
        },
        {
            .name = "av_GXMat4Scale",
            .func = &ScriptableGXMat4::OnScale
        },
        {
            .name = "av_GXMat4SetX",
            .func = &ScriptableGXMat4::OnSetX
        },
        {
            .name = "av_GXMat4SetY",
            .func = &ScriptableGXMat4::OnSetY
        },
        {
            .name = "av_GXMat4SetZ",
            .func = &ScriptableGXMat4::OnSetZ
        },
        {
            .name = "av_GXMat4SetW",
            .func = &ScriptableGXMat4::OnSetW
        },
        {
            .name = "av_GXMat4ToString",
            .func = &ScriptableGXMat4::OnToString
        },
        {
            .name = "av_GXMat4Translation",
            .func = &ScriptableGXMat4::OnTranslation
        }
    };

    for ( auto const &extension : extensions )
    {
        lua_register ( &vm, extension.name, extension.func );
    }
}

void ScriptableGXMat4::Destroy () noexcept
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

GXMat4 &ScriptableGXMat4::Extract ( lua_State* state, int idx ) noexcept
{
    auto &item = *static_cast<Item*> ( lua_touserdata ( state, idx ) );
    return item._matrix;
}

void ScriptableGXMat4::Insert ( Item* item, Item* &list ) noexcept
{
    item->_previous = nullptr;
    item->_next = list;

    if ( list )
        list->_previous = item;

    list = item;
}

int ScriptableGXMat4::OnClone ( lua_State* state )
{
    auto &self = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    self._matrix = ScriptableGXMat4::Extract ( state, 2 );
    return 0;
}

int ScriptableGXMat4::OnCreate ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::ScriptableGXMat4::OnCreate - Stack is too small." );
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

int ScriptableGXMat4::OnFromFast ( lua_State* state )
{
    auto &self = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    self._matrix.FromFast ( ScriptableGXQuat::Extract ( state, 2 ), ScriptableGXVec3::Extract ( state, 3 ) );
    return 0;
}

int ScriptableGXMat4::OnGetX ( lua_State* state )
{
    auto const &self = *static_cast<Item const*> ( lua_touserdata ( state, 1 ) );
    self._matrix.GetX ( ScriptableGXVec3::Extract ( state, 2 ) );
    return 0;
}

int ScriptableGXMat4::OnGetY ( lua_State* state )
{
    auto const &self = *static_cast<Item const*> ( lua_touserdata ( state, 1 ) );
    self._matrix.GetY ( ScriptableGXVec3::Extract ( state, 2 ) );
    return 0;
}

int ScriptableGXMat4::OnGetZ ( lua_State* state )
{
    auto const &self = *static_cast<Item const*> ( lua_touserdata ( state, 1 ) );
    self._matrix.GetZ ( ScriptableGXVec3::Extract ( state, 2 ) );
    return 0;
}

int ScriptableGXMat4::OnGetW ( lua_State* state )
{
    auto const &self = *static_cast<Item const*> ( lua_touserdata ( state, 1 ) );
    self._matrix.GetW ( ScriptableGXVec3::Extract ( state, 2 ) );
    return 0;
}

int ScriptableGXMat4::OnIdentity ( lua_State* state )
{
    auto &item = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    item._matrix.Identity ();
    return 0;
}

int ScriptableGXMat4::OnInverse ( lua_State* state )
{
    auto &self = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    auto const &sourceMatrix = *static_cast<Item const*> ( lua_touserdata ( state, 2 ) );
    self._matrix.Inverse ( sourceMatrix._matrix );

    return 0;
}

int ScriptableGXMat4::OnMultiply ( lua_State* state )
{
    auto &self = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    auto const &a = *static_cast<Item const*> ( lua_touserdata ( state, 2 ) );
    auto const &b = *static_cast<Item const*> ( lua_touserdata ( state, 3 ) );
    self._matrix.Multiply ( a._matrix, b._matrix );

    return 0;
}

int ScriptableGXMat4::OnMultiplyMatrixVector ( lua_State* state )
{
    auto &self = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );

    self._matrix.MultiplyMatrixVector ( ScriptableGXVec4::Extract ( state, 2 ),
        ScriptableGXVec4::Extract ( state, 3 )
    );

    return 0;
}

int ScriptableGXMat4::OnMultiplyAsNormal ( lua_State* state )
{
    auto &self = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    self._matrix.MultiplyAsNormal ( ScriptableGXVec3::Extract ( state, 2 ), ScriptableGXVec3::Extract ( state, 3 ) );
    return 0;
}

int ScriptableGXMat4::OnMultiplyAsPoint ( lua_State* state )
{
    auto &self = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    self._matrix.MultiplyAsPoint ( ScriptableGXVec3::Extract ( state, 2 ), ScriptableGXVec3::Extract ( state, 3 ) );
    return 0;
}

int ScriptableGXMat4::OnMultiplyVectorMatrix ( lua_State* state )
{
    auto &self = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );

    self._matrix.MultiplyVectorMatrix ( ScriptableGXVec4::Extract ( state, 2 ),
        ScriptableGXVec4::Extract ( state, 3 )
    );

    return 0;
}

int ScriptableGXMat4::OnPerspective ( lua_State* state )
{
    auto &item = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );

    item._matrix.Perspective ( static_cast<float> ( lua_tonumber ( state, 2 ) ),
        static_cast<float> ( lua_tonumber ( state, 3 ) ),
        static_cast<float> ( lua_tonumber ( state, 4 ) ),
        static_cast<float> ( lua_tonumber ( state, 5 ) )
    );

    return 0;
}

int ScriptableGXMat4::OnRotationX ( lua_State* state )
{
    auto &item = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    item._matrix.RotationX ( static_cast<float> ( lua_tonumber ( state, 2 ) ) );
    return 0;
}

int ScriptableGXMat4::OnRotationY ( lua_State* state )
{
    auto &item = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    item._matrix.RotationY ( static_cast<float> ( lua_tonumber ( state, 2 ) ) );
    return 0;
}

int ScriptableGXMat4::OnRotationZ ( lua_State* state )
{
    auto &item = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    item._matrix.RotationZ ( static_cast<float> ( lua_tonumber ( state, 2 ) ) );
    return 0;
}

int ScriptableGXMat4::OnScale ( lua_State* state )
{
    auto &item = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );

    item._matrix.Scale ( static_cast<float> ( lua_tonumber ( state, 2 ) ),
        static_cast<float> ( lua_tonumber ( state, 3 ) ),
        static_cast<float> ( lua_tonumber ( state, 4 ) )
    );

    return 0;
}

int ScriptableGXMat4::OnSetX ( lua_State* state )
{
    auto &self = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    self._matrix.SetX ( ScriptableGXVec3::Extract ( state, 2 ) );
    return 0;
}

int ScriptableGXMat4::OnSetY ( lua_State* state )
{
    auto &self = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    self._matrix.SetY ( ScriptableGXVec3::Extract ( state, 2 ) );
    return 0;
}

int ScriptableGXMat4::OnSetZ ( lua_State* state )
{
    auto &self = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    self._matrix.SetZ ( ScriptableGXVec3::Extract ( state, 2 ) );
    return 0;
}

int ScriptableGXMat4::OnSetW ( lua_State* state )
{
    auto &self = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    self._matrix.SetW ( ScriptableGXVec3::Extract ( state, 2 ) );
    return 0;
}

int ScriptableGXMat4::OnToString ( lua_State* state )
{
    auto const &item = *static_cast<Item const*> ( lua_touserdata ( state, 1 ) );
    auto const &m = item._matrix._data;

    constexpr char const format[] =
R"__(%14g %14g %14g %14g
%14g %14g %14g %14g
%14g %14g %14g %14g
%14g %14g %14g %14g)__";

    char result[ 512U ];

    int const len = std::snprintf ( result,
        std::size ( result ),
        format,
        m[ 0U ][ 0U ],
        m[ 0U ][ 1U ],
        m[ 0U ][ 2U ],
        m[ 0U ][ 3U ],
        m[ 1U ][ 0U ],
        m[ 1U ][ 1U ],
        m[ 1U ][ 2U ],
        m[ 1U ][ 3U ],
        m[ 2U ][ 0U ],
        m[ 2U ][ 1U ],
        m[ 2U ][ 2U ],
        m[ 2U ][ 3U ],
        m[ 3U ][ 0U ],
        m[ 3U ][ 1U ],
        m[ 3U ][ 2U ],
        m[ 3U ][ 3U ]
    );

    lua_pushlstring ( state, result, static_cast<size_t> ( len ) );
    return 1;
}

int ScriptableGXMat4::OnTranslation ( lua_State* state )
{
    auto &item = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );

    item._matrix.Translation ( static_cast<float> ( lua_tonumber ( state, 2 ) ),
        static_cast<float> ( lua_tonumber ( state, 3 ) ),
        static_cast<float> ( lua_tonumber ( state, 4 ) )
    );

    return 0;
}

} // namespace pbr
