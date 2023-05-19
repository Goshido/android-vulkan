#ifndef PBR_UI_LAYER_H
#define PBR_UI_LAYER_H


#include "div_ui_element.h"
#include "html5_element.h"

GX_DISABLE_COMMON_WARNINGS

#include <string>
#include <unordered_set>

GX_RESTORE_WARNING_STATE


namespace pbr {

class UILayer final
{
    public:
        struct CSSUnitToDevicePixel final
        {
            [[maybe_unused]] float              _fromMM = 3.7795F;
            [[maybe_unused]] float              _fromPT = 1.333F;
            float                               _fromPX = 1.0F;
        };

    private:
        DIVUIElement*                           _body = nullptr;
        CSSParser                               _css {};

        static CSSUnitToDevicePixel             _cssUnitToDevicePixel;
        static std::unordered_set<UILayer*>     _uiLayers;

    public:
        UILayer () = delete;

        UILayer ( UILayer const & ) = delete;
        UILayer& operator = ( UILayer const & ) = delete;

        UILayer ( UILayer && ) = delete;
        UILayer& operator = ( UILayer && ) = delete;

        explicit UILayer ( bool &success, lua_State &vm ) noexcept;

        ~UILayer () = default;

        void Submit () noexcept;

        static void InitCSSUnitConverter ( float dpi, float comfortableViewDistanceMeters ) noexcept;
        static void InitLuaFrontend ( lua_State &vm ) noexcept;
        static void Destroy () noexcept;

    private:
        [[nodiscard]] static bool AppendChild ( lua_State &vm,
            int errorHandlerIdx,
            int appendChildElementIdx,
            int uiLayerIdx,
            int registerNamedElementIdx,
            DIVUIElement &parent,
            HTML5Element &htmlChild
        ) noexcept;

        [[nodiscard]] static bool RegisterNamedElement ( lua_State &vm,
            int errorHandlerIdx,
            int uiLayerIdx,
            int registerNamedElementIdx,
            std::u32string const &id
        ) noexcept;

        [[nodiscard]] static int OnCreate ( lua_State* state );
        [[nodiscard]] static int OnGarbageCollected ( lua_State* state );
        [[nodiscard]] static int OnHide ( lua_State* state );
        [[nodiscard]] static int OnIsVisible ( lua_State* state );
        [[nodiscard]] static int OnShow ( lua_State* state );
};

} // namespace pbr


#endif // PBR_UI_LAYER_H
