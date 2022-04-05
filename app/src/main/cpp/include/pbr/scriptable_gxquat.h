#ifndef PBR_SCRIPTABLE_GXQUAT_H
#define PBR_SCRIPTABLE_GXQUAT_H


#include <GXCommon/GXMath.h>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lstate.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

class ScriptableGXQuat final
{
    private:
        struct Item final
        {
            Item*       _previous;
            Item*       _next;
            GXQuat      _quaternion;
        };

    private:
        static Item*    _free;
        static Item*    _used;

    public:
        ScriptableGXQuat () = delete;

        ScriptableGXQuat ( ScriptableGXQuat const & ) = delete;
        ScriptableGXQuat& operator = ( ScriptableGXQuat const & ) = delete;

        ScriptableGXQuat ( ScriptableGXQuat && ) = delete;
        ScriptableGXQuat& operator = ( ScriptableGXQuat && ) = delete;

        ~ScriptableGXQuat () = delete;

        static void Init ( lua_State* vm ) noexcept;
        static void Destroy () noexcept;

    private:
        static void Insert ( Item* item, Item*& list ) noexcept;

        [[nodiscard]] static int OnCreate ( lua_State* state );
        [[nodiscard]] static int OnDestroy ( lua_State* state );
        [[nodiscard]] static int OnFromAxisAngle ( lua_State* state );
        [[nodiscard]] static int OnMultiply ( lua_State* state );
        [[nodiscard]] static int OnNormalize ( lua_State* state );
        [[nodiscard]] static int OnToString ( lua_State* state );
        [[nodiscard]] static int OnTransformFast ( lua_State* state );
};

} // namespace pbr


#endif // PBR_SCRIPTABLE_GXQUAT_H
