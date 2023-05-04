#ifndef PBR_UI_LAYER_H
#define PBR_UI_LAYER_H


#include "html5_parser.h"
#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <string>
#include <unordered_set>

extern "C" {

#include <lua/lstate.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

class UILayer final
{
    private:
        HTML5Parser                             _html {};
        static std::unordered_set<UILayer*>     _uiLayers;

    public:
        UILayer () = delete;

        UILayer ( UILayer const & ) = delete;
        UILayer& operator = ( UILayer const & ) = delete;

        UILayer ( UILayer && ) = delete;
        UILayer& operator = ( UILayer && ) = delete;

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
