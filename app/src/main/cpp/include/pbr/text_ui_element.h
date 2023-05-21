#ifndef PBR_TEXT_UI_ELEMENT_H
#define PBR_TEXT_UI_ELEMENT_H


#include "ui_element.h"
#include <GXCommon/GXMath.h>

GX_DISABLE_COMMON_WARNINGS

#include <optional>
#include <string>

GX_RESTORE_WARNING_STATE


namespace pbr {

class TextUIElement : public UIElement
{
    private:
        // Way the user could override color which arrived from CSS.
        std::optional<GXColorRGB>       _color {};

        std::u32string                  _text {};

    public:
        TextUIElement () = delete;

        TextUIElement ( TextUIElement const & ) = delete;
        TextUIElement& operator = ( TextUIElement const & ) = delete;

        TextUIElement ( TextUIElement && ) = delete;
        TextUIElement& operator = ( TextUIElement && ) = delete;

        explicit TextUIElement ( bool &success,
            UIElement const* parent,
            lua_State &vm,
            int errorHandlerIdx,
            std::u32string &&text
        ) noexcept;

        ~TextUIElement () override = default;

        static void Init ( lua_State &vm ) noexcept;

    private:
        void ApplyLayout ( android_vulkan::Renderer &renderer,
            FontStorage &fontStorage,
            CSSUnitToDevicePixel const &cssUnits,
            GXVec2 &penLocation,
            float &lineHeight,
            GXVec2 const &canvasSize,
            float parentLeft,
            float parentWidth
        ) noexcept override;

        void Render () noexcept override;

        [[maybe_unused, nodiscard]] GXColorRGB const* ResolveColor () const noexcept;
        [[nodiscard]] std::string const* ResolveFont () const noexcept;
        [[nodiscard]] uint32_t ResolveFontSize ( CSSUnitToDevicePixel const &cssUnits ) const noexcept;

        [[nodiscard]] static int OnSetColorHSV ( lua_State* state );
        [[nodiscard]] static int OnSetColorRGB ( lua_State* state );
        [[nodiscard]] static int OnSetText ( lua_State* state );
};

} // namespace pbr


#endif // PBR_TEXT_UI_ELEMENT_H
