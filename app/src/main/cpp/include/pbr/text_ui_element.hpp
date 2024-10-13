#ifndef PBR_TEXT_UI_ELEMENT_HPP
#define PBR_TEXT_UI_ELEMENT_HPP


#include "ui_element.hpp"


namespace pbr {

class TextUIElement final : public UIElement
{
    private:
        struct Glyph final
        {
            int32_t                         _advance;

            GXVec3                          _atlasTopLeft;
            GXVec3                          _atlasBottomRight;

            int32_t                         _offsetY;
            size_t                          _parentLine;

            int32_t                         _width;
            int32_t                         _height;
        };

        struct Line final
        {
            size_t                          _glyphs = 0U;
            int32_t                         _length = 0;
        };

        struct ApplyLayoutCache final
        {
            bool                            _isTextChanged = true;
            std::vector<float>              _lineHeights {};
            GXVec2                          _penIn {};
            GXVec2                          _penOut {};
            size_t                          _vertices = 0U;

            [[nodiscard]] bool Run ( ApplyInfo &info ) noexcept;
        };

        struct SubmitCache final
        {
            bool                            _isColorChanged = true;
            bool                            _isTextChanged = true;

            GXVec2                          _penIn {};
            GXVec2                          _penOut {};

            std::vector<float>              _parentLineHeights {};
            GXVec2                          _parenTopLeft {};

            std::vector<UIVertexInfo>       _vertices {};
            size_t                          _vertexBufferBytes = 0U;

            [[nodiscard]] bool Run ( UpdateInfo &info, std::vector<float> const &cachedLineHeight ) noexcept;
        };

        using AlignIntegerHandler = int32_t ( * ) ( int32_t pen, int32_t parentSize, int32_t lineSize ) noexcept;

    private:
        ApplyLayoutCache                    _applyLayoutCache {};
        SubmitCache                         _submitCache {};

        // Way the user could override color which arrived from CSS.
        std::optional<GXColorRGB>           _color {};

        int32_t                             _fontSize = 0;
        std::vector<Glyph>                  _glyphs {};

        std::vector<Line>                   _lines {};
        std::u32string                      _text {};

    public:
        TextUIElement () = delete;

        TextUIElement ( TextUIElement const & ) = delete;
        TextUIElement &operator = ( TextUIElement const & ) = delete;

        TextUIElement ( TextUIElement && ) = default;
        TextUIElement &operator = ( TextUIElement && ) = delete;

        explicit TextUIElement ( bool visible, UIElement const* parent, std::u32string &&text ) noexcept;

        ~TextUIElement () override = default;

        void SetColor ( GXColorRGB const &color ) noexcept;
        void SetText ( char const* text ) noexcept;

    private:
        void ApplyLayout ( ApplyInfo &info ) noexcept override;
        void Submit ( SubmitInfo &info ) noexcept override;
        [[nodiscard]] bool UpdateCache ( UpdateInfo &info ) noexcept override;

        [[nodiscard]] GXColorRGB const &ResolveColor () const noexcept;
        [[nodiscard]] std::string const* ResolveFont () const noexcept;

        [[nodiscard]] static int32_t AlignIntegerToCenter ( int32_t pen,
            int32_t parentSize,
            int32_t lineSize
        ) noexcept;

        [[nodiscard]] static int32_t AlignIntegerToStart ( int32_t pen, int32_t parentSize, int32_t lineSize ) noexcept;
        [[nodiscard]] static int32_t AlignIntegerToEnd ( int32_t pen, int32_t parentSize, int32_t lineSize ) noexcept;

        [[nodiscard]] static AlignIntegerHandler ResolveIntegerTextAlignment ( UIElement const* parent ) noexcept;
        [[nodiscard]] static AlignIntegerHandler ResolveIntegerVerticalAlignment ( UIElement const* parent ) noexcept;
};

} // namespace pbr


#endif // PBR_TEXT_UI_ELEMENT_HPP
