#ifndef PBR_UI_LAYER_H
#define PBR_UI_LAYER_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <string>

extern "C" {

#include <lua/lstate.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

class UILayer final
{
    public:
        UILayer () = delete;

        UILayer ( UILayer const & ) = default;
        UILayer& operator = ( UILayer const & ) = default;

        UILayer ( UILayer && ) = default;
        UILayer& operator = ( UILayer && ) = default;

        [[maybe_unused]] explicit UILayer ( bool &success, std::string &&uiAsset ) noexcept;

        ~UILayer () = default;

        [[nodiscard]] static bool Init ( lua_State &vm ) noexcept;
        static void Destroy () noexcept;

    private:
        [[nodiscard]] static int OnCreate ( lua_State* state );
        [[nodiscard]] static int OnFind ( lua_State* state );
        [[nodiscard]] static int OnGarbageCollected ( lua_State* state );
        [[nodiscard]] static int OnHide ( lua_State* state );
        [[nodiscard]] static int OnIsVisible ( lua_State* state );
        [[nodiscard]] static int OnShow ( lua_State* state );
};

} // namespace pbr


#endif // PBR_UI_LAYER_H
