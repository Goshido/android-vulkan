#include <precompiled_headers.hpp>
#include <pbr/bit_field.hpp>
#include <logger.hpp>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

#include <limits>

GX_RESTORE_WARNING_STATE


namespace pbr {

namespace {

constexpr uint32_t BIT_COUNT = 32U;

// ~2 Mb of data at 64bit system
constexpr size_t INITIAL_CAPACITY = 104'857U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

BitField::Item* BitField::_free = nullptr;
BitField::Item* BitField::_used = nullptr;

void BitField::Init ( lua_State &vm ) noexcept
{
    for ( size_t i = 0U; i < INITIAL_CAPACITY; ++i )
        Insert ( new Item {}, _free );

    constexpr luaL_Reg const extensions[] =
    {
        {
            .name = "av_BitFieldAnd",
            .func = &BitField::OnAnd
        },
        {
            .name = "av_BitFieldClone",
            .func = &BitField::OnClone
        },
        {
            .name = "av_BitFieldCreate",
            .func = &BitField::OnCreate
        },
        {
            .name = "av_BitFieldDestroy",
            .func = &BitField::OnDestroy
        },
        {
            .name = "av_BitFieldEqual",
            .func = &BitField::OnEqual
        },
        {
            .name = "av_BitFieldNot",
            .func = &BitField::OnNot
        },
        {
            .name = "av_BitFieldOr",
            .func = &BitField::OnOr
        },
        {
            .name = "av_BitFieldResetAllBits",
            .func = &BitField::OnResetAllBits
        },
        {
            .name = "av_BitFieldResetBit",
            .func = &BitField::OnResetBit
        },
        {
            .name = "av_BitFieldSetAllBits",
            .func = &BitField::OnSetAllBits
        },
        {
            .name = "av_BitFieldSetBit",
            .func = &BitField::OnSetBit
        },
        {
            .name = "av_BitFieldXor",
            .func = &BitField::OnXor
        }
    };

    for ( auto const &extension : extensions )
    {
        lua_register ( &vm, extension.name, extension.func );
    }
}

void BitField::Destroy () noexcept
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

uint32_t BitField::Extract ( lua_State* state, int idx ) noexcept
{
    auto const &item = *static_cast<Item const*> ( lua_touserdata ( state, idx ) );
    return item._bitField;
}

BitField::Item &BitField::AllocateResult () noexcept
{
    Item* result = _free;

    if ( !result ) [[unlikely]]
    {
        Insert ( new Item {}, _used );
        return *_used;
    }

    _free = _free->_next;

    if ( _free )
        _free->_previous = nullptr;

    Insert ( result, _used );
    return *result;
}

void BitField::Insert ( Item* item, Item* &list ) noexcept
{
    item->_previous = nullptr;
    item->_next = list;

    if ( list )
        list->_previous = item;

    list = item;
}

int BitField::OnAnd ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::BitField::OnAnd - Stack is too small." );
        return 0;
    }

    auto const &self = *static_cast<Item const*> ( lua_touserdata ( state, 1 ) );
    auto const &other = *static_cast<Item const*> ( lua_touserdata ( state, 2 ) );
    Item &result = AllocateResult ();

    result._bitField = self._bitField & other._bitField;
    lua_pushlightuserdata ( state, &result );
    return 1;
}

int BitField::OnClone ( lua_State* state )
{
    auto &self = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    auto const &source = *static_cast<Item const*> ( lua_touserdata ( state, 2 ) );
    self._bitField = source._bitField;
    return 0;
}

int BitField::OnCreate ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::BitField::OnCreate - Stack is too small." );
        return 0;
    }

    Item &item = AllocateResult ();
    item._bitField = 0U;
    lua_pushlightuserdata ( state, &item );
    return 1;
}

int BitField::OnDestroy ( lua_State* state )
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

int BitField::OnEqual ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::BitField::OnEqual - Stack is too small." );
        return 0;
    }

    auto const &self = *static_cast<Item const*> ( lua_touserdata ( state, 1 ) );
    auto const &other = *static_cast<Item const*> ( lua_touserdata ( state, 2 ) );
    lua_pushboolean ( state, self._bitField == other._bitField );
    return 1;
}

int BitField::OnNot ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::BitField::OnNot - Stack is too small." );
        return 0;
    }

    auto &self = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    Item &result = AllocateResult ();

    result._bitField = ~self._bitField;
    lua_pushlightuserdata ( state, &result );
    return 1;
}

int BitField::OnOr ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::BitField::OnOr - Stack is too small." );
        return 0;
    }

    auto const &self = *static_cast<Item const*> ( lua_touserdata ( state, 1 ) );
    auto const &other = *static_cast<Item const*> ( lua_touserdata ( state, 2 ) );
    Item &result = AllocateResult ();

    result._bitField = self._bitField | other._bitField;
    lua_pushlightuserdata ( state, &result );
    return 1;
}

int BitField::OnResetAllBits ( lua_State* state )
{
    auto &self = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    self._bitField = 0U;
    return 0;
}

int BitField::OnResetBit ( lua_State* state )
{
    auto &self = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    auto const bit = static_cast<uint32_t> ( lua_tointeger ( state, 2 ) );

    if ( bit < BIT_COUNT ) [[likely]]
    {
        self._bitField &= ~( 1U << bit );
        return 0;
    }

    android_vulkan::LogWarning ( "pbr::BitField::OnResetBit - Trying to reset bit %u [bit count %u]. Aborting...",
        bit,
        BIT_COUNT
    );

    return 0;
}

int BitField::OnSetAllBits ( lua_State* state )
{
    auto &self = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    self._bitField = std::numeric_limits<uint32_t>::max ();
    return 0;
}

int BitField::OnSetBit ( lua_State* state )
{
    auto &self = *static_cast<Item*> ( lua_touserdata ( state, 1 ) );
    auto const bit = static_cast<uint32_t> ( lua_tointeger ( state, 2 ) );

    if ( bit < BIT_COUNT ) [[likely]]
    {
        self._bitField |= 1U << bit;
        return 0;
    }

    android_vulkan::LogWarning ( "pbr::BitField::OnSetBit - Trying to set bit %u [bit count %u]. Aborting...",
        bit,
        BIT_COUNT
    );

    return 0;
}

int BitField::OnXor ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::BitField::OnXor - Stack is too small." );
        return 0;
    }

    auto const &self = *static_cast<Item const*> ( lua_touserdata ( state, 1 ) );
    auto const &other = *static_cast<Item const*> ( lua_touserdata ( state, 2 ) );
    Item &result = AllocateResult ();

    result._bitField = self._bitField ^ other._bitField;
    lua_pushlightuserdata ( state, &result );
    return 1;
}

} // namespace pbr
