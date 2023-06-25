#ifndef PBR_TEXT_UI_ELEMENT_H
#define PBR_TEXT_UI_ELEMENT_H


#include "ui_element.h"
#include <GXCommon/GXMath.h>

GX_DISABLE_COMMON_WARNINGS

#include <optional>
#include <string>

GX_RESTORE_WARNING_STATE


namespace pbr {

class TextUIElement final : public UIElement
{
    private:
        struct Glyph final
        {
            float                       _advance;
            float                       _offsetY;
            size_t                      _parentLine;
            GXVec2                      _size;

            GXVec3                      _atlasTopLeft;
            GXVec3                      _atlasBottomRight;
        };

        struct Line final
        {
            size_t                      _glyphs = 0U;
            float                       _length = 0U;
        };

        using AlignHander = float ( * ) ( float penX, float parentWidth, float lineLength ) noexcept;

    private:
        // Way the user could override color which arrived from CSS.
        std::optional<GXColorRGB>       _color {};

        std::vector<Glyph>              _glyphs {};
        std::vector<Line>               _lines {};
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
        void ApplyLayout ( ApplyLayoutInfo &info ) noexcept override;
        void Submit ( SubmitInfo &info ) noexcept override;

        [[nodiscard]] GXColorRGB const& ResolveColor () const noexcept;
        [[nodiscard]] std::string const* ResolveFont () const noexcept;

        [[nodiscard]] static float AlignCenter ( float penX, float parentWidth, float lineLength ) noexcept;
        [[nodiscard]] static float AlignLeft ( float penX, float parentWidth, float lineLength ) noexcept;
        [[nodiscard]] static float AlignRight ( float penX, float parentWidth, float lineLength ) noexcept;

        [[nodiscard]] static int OnSetColorHSV ( lua_State* state );
        [[nodiscard]] static int OnSetColorRGB ( lua_State* state );
        [[nodiscard]] static int OnSetText ( lua_State* state );

        [[nodiscard]] static AlignHander ResolveAlignment ( UIElement const* parent ) noexcept;
};

} // namespace pbr


#endif // PBR_TEXT_UI_ELEMENT_H
