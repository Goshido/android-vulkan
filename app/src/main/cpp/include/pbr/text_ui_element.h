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

        explicit TextUIElement ( std::u32string &&text ) noexcept;

        ~TextUIElement () override = default;

    private:
        void ApplyLayout () noexcept override;
        void Render () noexcept override;

        [[nodiscard]] static int OnSetColor ( lua_State* state );
        [[nodiscard]] static int OnSetText ( lua_State* state );
};

} // namespace pbr

#endif // PBR_TEXT_UI_ELEMENT_H
