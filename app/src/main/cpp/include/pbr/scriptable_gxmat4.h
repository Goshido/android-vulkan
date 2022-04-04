#ifndef PBR_SCRIPTABLE_GXMAT4_H
#define PBR_SCRIPTABLE_GXMAT4_H


#include <GXCommon/GXMath.h>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lstate.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

class ScriptableGXMat4 final
{
    private:
        struct Item final
        {
            Item*       _previous;
            Item*       _next;
            GXMat4      _matrix;
        };

    private:
        static Item*    _free;
        static Item*    _used;

    public:
        ScriptableGXMat4 () = delete;

        ScriptableGXMat4 ( ScriptableGXMat4 const & ) = delete;
        ScriptableGXMat4& operator = ( ScriptableGXMat4 const & ) = delete;

        ScriptableGXMat4 ( ScriptableGXMat4 && ) = delete;
        ScriptableGXMat4& operator = ( ScriptableGXMat4 && ) = delete;

        ~ScriptableGXMat4 () = delete;

        static void Init ( lua_State* vm ) noexcept;
        static void Destroy () noexcept;

    private:
        static void Insert ( Item* item, Item*& list ) noexcept;

        [[nodiscard]] static int OnCreate ( lua_State* state );
        [[nodiscard]] static int OnDestroy ( lua_State* state );
        [[nodiscard]] static int OnGetX ( lua_State* state );
        [[nodiscard]] static int OnGetY ( lua_State* state );
        [[nodiscard]] static int OnGetZ ( lua_State* state );
        [[nodiscard]] static int OnGetW ( lua_State* state );
        [[nodiscard]] static int OnIdentity ( lua_State* state );
        [[nodiscard]] static int OnInverse ( lua_State* state );
        [[nodiscard]] static int OnMultiply ( lua_State* state );
        [[nodiscard]] static int OnMultiplyMatrixVector ( lua_State* state );
        [[nodiscard]] static int OnMultiplyAsNormal ( lua_State* state );
        [[nodiscard]] static int OnMultiplyAsPoint ( lua_State* state );
        [[nodiscard]] static int OnMultiplyVectorMatrix ( lua_State* state );
        [[nodiscard]] static int OnPerspective ( lua_State* state );
        [[nodiscard]] static int OnRotationX ( lua_State* state );
        [[nodiscard]] static int OnRotationY ( lua_State* state );
        [[nodiscard]] static int OnRotationZ ( lua_State* state );
        [[nodiscard]] static int OnScale ( lua_State* state );
        [[nodiscard]] static int OnSetX ( lua_State* state );
        [[nodiscard]] static int OnSetY ( lua_State* state );
        [[nodiscard]] static int OnSetZ ( lua_State* state );
        [[nodiscard]] static int OnSetW ( lua_State* state );
        [[nodiscard]] static int OnToString ( lua_State* state );
        [[nodiscard]] static int OnTranslation ( lua_State* state );
};

} // namespace pbr


#endif // PBR_SCRIPTABLE_GXMAT4_H
