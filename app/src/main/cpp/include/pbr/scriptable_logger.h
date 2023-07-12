#ifndef PBR_SCRIPTABLE_LOGGER_H
#define PBR_SCRIPTABLE_LOGGER_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lstate.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

class ScriptableLogger final
{
    public:
        ScriptableLogger () = delete;

        ScriptableLogger ( ScriptableLogger const & ) = delete;
        ScriptableLogger &operator = ( ScriptableLogger const & ) = delete;

        ScriptableLogger ( ScriptableLogger && ) = delete;
        ScriptableLogger &operator = ( ScriptableLogger && ) = delete;

        ~ScriptableLogger () = delete;

        static void Register ( lua_State &vm ) noexcept;

    private:
        [[nodiscard]] static int OnLogDebug ( lua_State* vm );
        [[nodiscard]] static int OnLogError ( lua_State* vm );
        [[nodiscard]] static int OnLogInfo ( lua_State* vm );
        [[nodiscard]] static int OnLogWarning ( lua_State* vm );
};

} // namespace pbr


#endif // PBR_SCRIPTABLE_LOGGER_H
