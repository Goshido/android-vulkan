#ifndef PBR_SCRIPTABLE_GXVEC4_H
#define PBR_SCRIPTABLE_GXVEC4_H


#include <GXCommon/GXMath.h>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lstate.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

class ScriptableGXVec4 final
{
    private:
        struct Item final
        {
            Item*       _previous;
            Item*       _next;
            GXVec4      _vec4;
        };

    private:
        static Item*    _free;
        static Item*    _used;

    public:
        ScriptableGXVec4 () = delete;

        ScriptableGXVec4 ( ScriptableGXVec4 const & ) = delete;
        ScriptableGXVec4 &operator = ( ScriptableGXVec4 const & ) = delete;

        ScriptableGXVec4 ( ScriptableGXVec4 && ) = delete;
        ScriptableGXVec4 &operator = ( ScriptableGXVec4 && ) = delete;

        ~ScriptableGXVec4 () = delete;

        static void Init ( lua_State &vm ) noexcept;
        static void Destroy () noexcept;

        [[nodiscard]] static GXVec4 &Extract ( lua_State* state, int idx ) noexcept;

    private:
        static void Insert ( Item* item, Item* &list ) noexcept;

        [[nodiscard]] static int OnCreate ( lua_State* state );
        [[nodiscard]] static int OnDestroy ( lua_State* state );
        [[nodiscard]] static int OnInit ( lua_State* state );
        [[nodiscard]] static int OnToString ( lua_State* state );
};

} // namespace pbr


#endif // PBR_SCRIPTABLE_GXVEC4_H
