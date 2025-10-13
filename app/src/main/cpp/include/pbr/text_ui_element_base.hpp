#ifndef PBR_TEXT_UI_ELEMENT_BASE_HPP
#define PBR_TEXT_UI_ELEMENT_BASE_HPP


#include "color_value.hpp"
#include "text_align_property.hpp"
#include "vertical_align_property.hpp"


namespace pbr {

template<typename E, typename G, typename GI, typename S0, typename S1, typename U, typename F>
class TextUIElementBase : public E
{
    private:
        struct Line final
        {
            size_t                      _glyphs = 0U;
            int32_t                     _length = 0;
        };

        struct ApplyLayoutCache final
        {
            bool                        _isTextChanged = true;
            std::vector<float>          _lineHeights {};
            GXVec2                      _penIn {};
            GXVec2                      _penOut {};
            size_t                      _vertices = 0U;

            void Clear () noexcept;
            [[nodiscard]] bool Run ( E::ApplyInfo &info ) noexcept;
        };

        struct SubmitCache final
        {
            bool                        _isColorChanged = true;
            bool                        _isTextChanged = true;

            GXVec2                      _penIn {};
            GXVec2                      _penOut {};

            std::vector<float>          _parentLineHeights {};
            GXVec2                      _parenSize {};

            std::vector<S0>             _stream0 {};
            std::vector<S1>             _stream1 {};

            void Clear () noexcept;

            [[nodiscard]] bool Run ( E::UpdateInfo &info,
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
        ApplyLayoutCache                _applyLayoutCache {};
        SubmitCache                     _submitCache {};

        // Way the user could override color which arrived from CSS.
        std::optional<GXColorUNORM>     _color {};

        int32_t                         _baselineToBaseline = 0;
        int32_t                         _contentAreaHeight = 0;
        std::vector<G>                  _glyphs {};

        std::vector<Line>               _lines {};
        std::u32string                  _text {};

    public:
        TextUIElementBase () = delete;

        TextUIElementBase ( TextUIElementBase const & ) = delete;
        TextUIElementBase &operator = ( TextUIElementBase const & ) = delete;

        TextUIElementBase ( TextUIElementBase && ) = default;
        TextUIElementBase &operator = ( TextUIElementBase && ) = delete;

        explicit TextUIElementBase ( bool visible, E const* parent, std::u32string &&text ) noexcept;

        explicit TextUIElementBase ( bool visible,
            E const* parent,
            std::u32string &&text,
            std::string &&name
        ) noexcept;

        explicit TextUIElementBase ( bool visible, E const* parent, std::string_view text ) noexcept;

        explicit TextUIElementBase ( bool visible,
            E const* parent,
            std::string_view text,
            std::string &&name
        ) noexcept;

        ~TextUIElementBase () override = default;

        void SetColor ( ColorValue const &color ) noexcept;
        void SetColor ( GXColorUNORM color ) noexcept;
        void SetText ( char const* text ) noexcept;
        void SetText ( std::string_view text ) noexcept;
        void SetText ( std::u32string_view text ) noexcept;

    protected:
        virtual void OnCacheUpdated ( std::span<G> glyphs ) noexcept;

        virtual void OnGlyphAdded ( GI const &glyphInfo ) noexcept;
        virtual void OnGlyphCleared () noexcept;
        virtual void OnGlyphResized ( size_t count ) noexcept;

    private:
        void ApplyLayout ( E::ApplyInfo &info ) noexcept override;
        void Submit ( E::SubmitInfo &info ) noexcept override;
        [[nodiscard]] bool UpdateCache ( E::UpdateInfo &info ) noexcept override;

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


#include <pbr/text_ui_element_base.ipp>


#endif // PBR_TEXT_UI_ELEMENT_BASE_HPP
