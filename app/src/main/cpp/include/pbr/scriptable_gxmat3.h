#ifndef PBR_SCRIPTABLE_GXMAT3_H
#define PBR_SCRIPTABLE_GXMAT3_H


#include <GXCommon/GXMath.h>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lstate.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

class ScriptableGXMat3 final
{
    private:
        struct Item final
        {
            Item*       _previous;
            Item*       _next;
            GXMat3      _matrix;
        };

    private:
        static Item*    _free;
        static Item*    _used;

    public:
        ScriptableGXMat3 () = delete;

        ScriptableGXMat3 ( ScriptableGXMat3 const & ) = delete;
        ScriptableGXMat3 &operator = ( ScriptableGXMat3 const & ) = delete;

        ScriptableGXMat3 ( ScriptableGXMat3 && ) = delete;
        ScriptableGXMat3 &operator = ( ScriptableGXMat3 && ) = delete;

        ~ScriptableGXMat3 () = delete;

        static void Init ( lua_State &vm ) noexcept;
        static void Destroy () noexcept;

    private:
        static void Insert ( Item* item, Item* &list ) noexcept;

        [[nodiscard]] static int OnCreate ( lua_State* state );
        [[nodiscard]] static int OnDestroy ( lua_State* state );
        [[nodiscard]] static int OnGetX ( lua_State* state );
        [[nodiscard]] static int OnGetY ( lua_State* state );
        [[nodiscard]] static int OnGetZ ( lua_State* state );
        [[nodiscard]] static int OnIdentity ( lua_State* state );
        [[nodiscard]] static int OnInverse ( lua_State* state );
        [[nodiscard]] static int OnMultiply ( lua_State* state );
        [[nodiscard]] static int OnMultiplyMatrixVector ( lua_State* state );
        [[nodiscard]] static int OnMultiplyVectorMatrix ( lua_State* state );
        [[nodiscard]] static int OnSetX ( lua_State* state );
        [[nodiscard]] static int OnSetY ( lua_State* state );
        [[nodiscard]] static int OnSetZ ( lua_State* state );
        [[nodiscard]] static int OnToString ( lua_State* state );
        [[nodiscard]] static int OnTranspose ( lua_State* state );
};

} // namespace pbr


#endif // PBR_SCRIPTABLE_GXMAT3_H
