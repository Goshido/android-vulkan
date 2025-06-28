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

            android_vulkan::Half2           _atlasTopLeft;
            android_vulkan::Half2           _atlasBottomRight;
            uint8_t                         _atlasLayer;

            int32_t                         _offsetX;
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

            void Clear () noexcept;
            [[nodiscard]] bool Run ( ApplyInfo &info ) noexcept;
        };

        struct SubmitCache final
        {
            bool                            _isColorChanged = true;
            bool                            _isTextChanged = true;

            GXVec2                          _penIn {};
            GXVec2                          _penOut {};

            std::vector<float>              _parentLineHeights {};
            GXVec2                          _parenSize {};

            std::vector<GXVec2>             _positions {};
            size_t                          _positionBufferBytes = 0U;

            std::vector<UIVertex>           _vertices {};
            size_t                          _vertexBufferBytes = 0U;

            void Clear () noexcept;

            [[nodiscard]] bool Run ( UpdateInfo &info,
                TextAlignProperty::eValue horizontal,
                VerticalAlignProperty::eValue vertical,
                std::vector<float> const &cachedLineHeight
            ) noexcept;
        };

        using AlignIntegerHandler = int32_t ( * ) ( int32_t pen,
            int32_t parentSize,
            int32_t lineSize,
            int32_t leading
        ) noexcept;

    private:
        ApplyLayoutCache                    _applyLayoutCache {};
        SubmitCache                         _submitCache {};

        // Way the user could override color which arrived from CSS.
        std::optional<GXColorUNORM>         _color {};

        int32_t                             _baselineToBaseline = 0;
        int32_t                             _contentAreaHeight = 0;
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

        void SetColor ( ColorValue const &color ) noexcept;
        void SetColor ( GXColorUNORM color ) noexcept;
        void SetText ( char const* text ) noexcept;
        void SetText ( std::string_view text ) noexcept;
        void SetText ( std::u32string_view text ) noexcept;

    private:
        void ApplyLayout ( ApplyInfo &info ) noexcept override;
        void Submit ( SubmitInfo &info ) noexcept override;
        [[nodiscard]] bool UpdateCache ( UpdateInfo &info ) noexcept override;

        [[nodiscard]] TextAlignProperty::eValue GetTextAlignment () const noexcept;
        [[nodiscard]] VerticalAlignProperty::eValue GetVerticalAlignment () const noexcept;

        [[nodiscard]] AlignIntegerHandler GetIntegerTextAlignment () const noexcept;
        [[nodiscard]] AlignIntegerHandler GetIntegerVerticalAlignment () const noexcept;

        [[nodiscard]] GXColorUNORM ResolveColor () const noexcept;

        [[nodiscard]] static int32_t AlignIntegerToCenter ( int32_t pen,
            int32_t parentSize,
            int32_t lineSize,
            int32_t halfLeading
        ) noexcept;

        [[nodiscard]] static int32_t AlignIntegerToStart ( int32_t pen,
            int32_t parentSize,
            int32_t lineSize,
            int32_t halfLeading
        ) noexcept;

        [[nodiscard]] static int32_t AlignIntegerToEnd ( int32_t pen,
            int32_t parentSize,
            int32_t lineSize,
            int32_t halfLeading
        ) noexcept;
};

} // namespace pbr


#endif // PBR_TEXT_UI_ELEMENT_HPP
