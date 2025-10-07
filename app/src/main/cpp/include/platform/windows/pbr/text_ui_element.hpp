#ifndef PBR_TEXT_UI_ELEMENT_HPP
#define PBR_TEXT_UI_ELEMENT_HPP


#include <pbr/text_ui_element_base.hpp>
#include "ui_element.hpp"


namespace pbr {

struct TextGlyph final
{
    int32_t                     _advance = 0;

    android_vulkan::Half2       _atlasTopLeft {};
    android_vulkan::Half2       _atlasBottomRight {};
    uint16_t                    _atlasPage = 0U;

    int32_t                     _offsetX = 0;
    int32_t                     _offsetY = 0;
    size_t                      _parentLine = 0U;

    int32_t                     _width = 0;
    int32_t                     _height = 0;
};

class TextUIElement final :
    public TextUIElementBase<UIElement, TextGlyph, GlyphInfo, UIVertexStream0, UIVertexStream1, UIPass, FontStorage>
{
    private:
        // Font storage uploads glyphs into GPU memory after text layout is calculated.
        // Because of that glyph atlas image can not be resolved at text layout computation stage.
        // This field is a connection between font storage glyph data and '_glyphs' field
        // to write atlas image index when it's ready.
        std::vector<uint16_t const*>    _atlasPromise {};

    public:
        TextUIElement () = delete;

        TextUIElement ( TextUIElement const & ) = delete;
        TextUIElement &operator = ( TextUIElement const & ) = delete;

        TextUIElement ( TextUIElement && ) = default;
        TextUIElement &operator = ( TextUIElement && ) = delete;

        explicit TextUIElement ( bool visible, UIElement const* parent, std::u32string &&text ) noexcept;

        explicit TextUIElement ( bool visible,
            UIElement const* parent,
            std::u32string &&text,
            std::string &&name
        ) noexcept;

        explicit TextUIElement ( bool visible, UIElement const* parent, std::string_view text ) noexcept;

        explicit TextUIElement ( bool visible,
            UIElement const* parent,
            std::string_view text,
            std::string &&name
        ) noexcept;

        ~TextUIElement () override = default;

    private:
        void OnCacheUpdated ( std::span<TextGlyph> glyphs ) noexcept override;

        void OnGlyphAdded ( GlyphInfo const &glyphInfo ) noexcept override;
        void OnGlyphCleared () noexcept override;
        void OnGlyphResized ( size_t count ) noexcept override;
};

} // namespace pbr


#endif // PBR_TEXT_UI_ELEMENT_HPP
