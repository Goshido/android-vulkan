#ifndef PBR_SCRIPTABLE_GXVEC3_H
#define PBR_SCRIPTABLE_GXVEC3_H


#include <GXCommon/GXMath.h>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lstate.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

class ScriptableGXVec3 final
{
    public:
        struct Item final
        {
            Item*       _previous;
            Item*       _next;
            GXVec3      _vec3;
        };

    private:
        static Item*    _free;
        static Item*    _used;

    public:
        ScriptableGXVec3 () = delete;

        ScriptableGXVec3 ( ScriptableGXVec3 const & ) = delete;
        ScriptableGXVec3& operator = ( ScriptableGXVec3 const & ) = delete;

        ScriptableGXVec3 ( ScriptableGXVec3 && ) = delete;
        ScriptableGXVec3& operator = ( ScriptableGXVec3 && ) = delete;

        ~ScriptableGXVec3 () = delete;

        static void Init ( lua_State &vm ) noexcept;
        static void Destroy () noexcept;

        [[nodiscard]] static GXVec3& Extract ( lua_State* state, int idx ) noexcept;

    private:
        static void Insert ( Item* item, Item*& list ) noexcept;

        [[nodiscard]] static int OnCreate ( lua_State* state );
        [[nodiscard]] static int OnCrossProduct ( lua_State* state );
        [[nodiscard]] static int OnDestroy ( lua_State* state );
        [[nodiscard]] static int OnDistance ( lua_State* state );
        [[nodiscard]] static int OnDotProduct ( lua_State* state );
        [[nodiscard]] static int OnGetX ( lua_State* state );
        [[nodiscard]] static int OnGetY ( lua_State* state );
        [[nodiscard]] static int OnGetZ ( lua_State* state );
        [[nodiscard]] static int OnInit ( lua_State* state );
        [[nodiscard]] static int OnLength ( lua_State* state );
        [[nodiscard]] static int OnMultiplyScalar ( lua_State* state );
        [[nodiscard]] static int OnNormalize ( lua_State* state );
        [[nodiscard]] static int OnReverse ( lua_State* state );
        [[nodiscard]] static int OnSetX ( lua_State* state );
        [[nodiscard]] static int OnSetY ( lua_State* state );
        [[nodiscard]] static int OnSetZ ( lua_State* state );
        [[nodiscard]] static int OnSquaredDistance ( lua_State* state );
        [[nodiscard]] static int OnSquaredLength ( lua_State* state );
        [[nodiscard]] static int OnSubtract ( lua_State* state );
        [[nodiscard]] static int OnSum ( lua_State* state );
        [[nodiscard]] static int OnSumScaled ( lua_State* state );
        [[nodiscard]] static int OnToString ( lua_State* state );
};

} // namespace pbr


#endif // PBR_SCRIPTABLE_GXVEC3_H
