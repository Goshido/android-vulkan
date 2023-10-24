#ifndef PBR_BIT_FIELD_HPP
#define PBR_BIT_FIELD_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lstate.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

class BitField final
{
    private:
        struct Item final
        {
            Item*               _previous;
            Item*               _next;
            uint32_t            _bitField;
        };

    private:
        static Item*            _free;
        static Item*            _used;

    public:
        BitField () = delete;

        BitField ( BitField const & ) = delete;
        BitField &operator = ( BitField const & ) = delete;

        BitField ( BitField && ) = delete;
        BitField &operator = ( BitField && ) = delete;

        ~BitField () = delete;

        static void Init ( lua_State &vm ) noexcept;
        static void Destroy () noexcept;

        [[nodiscard]] static uint32_t Extract ( lua_State* state, int idx ) noexcept;

    private:
        [[nodiscard]] static Item &AllocateResult () noexcept;
        static void Insert ( Item* item, Item* &list ) noexcept;

        [[nodiscard]] static int OnAnd ( lua_State* state );
        [[nodiscard]] static int OnClone ( lua_State* state );
        [[nodiscard]] static int OnCreate ( lua_State* state );
        [[nodiscard]] static int OnDestroy ( lua_State* state );
        [[nodiscard]] static int OnEqual ( lua_State* state );
        [[nodiscard]] static int OnOr ( lua_State* state );
        [[nodiscard]] static int OnNot ( lua_State* state );
        [[nodiscard]] static int OnResetAllBits ( lua_State* state );
        [[nodiscard]] static int OnResetBit ( lua_State* state );
        [[nodiscard]] static int OnSetAllBits ( lua_State* state );
        [[nodiscard]] static int OnSetBit ( lua_State* state );
        [[nodiscard]] static int OnXor ( lua_State* state );
};

} // namespace pbr


#endif // PBR_BIT_FIELD_HPP
