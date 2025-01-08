#ifndef PBR_HTML5_PARSER_HPP
#define PBR_HTML5_PARSER_HPP


#include "html5_element.hpp"
#include "parse_result.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <unordered_set>

GX_RESTORE_WARNING_STATE


namespace pbr {

class HTML5Parser final
{
    private:
        bool                                    _bodyElementParsed = false;
        bool                                    _headElementParsed = false;

        std::unordered_set<std::u32string>      _bodyClasses {};
        HTML5Children                           _bodyChildren {};

        CSSComputedValues                       _bodyCSS
        {
            ._backgroundColor = ColorValue ( false, GXColorUNORM ( 0U, 0U, 0U, 0U ) ),
            ._backgroundSize = LengthValue ( LengthValue::eType::Percent, 100.0F ),
            ._bottom = LengthValue ( LengthValue::eType::Auto, 42.0F ),
            ._left = LengthValue ( LengthValue::eType::Auto, 42.0F ),
            ._right = LengthValue ( LengthValue::eType::Auto, 42.0F ),
            ._top = LengthValue ( LengthValue::eType::Auto, 42.0F ),
            ._color = ColorValue ( false, GXColorUNORM ( 0U, 0U, 0U, 0xFFU ) ),
            ._display = DisplayProperty::eValue::Block,
            ._fontFile {},
            ._fontSize = LengthValue ( LengthValue::eType::PX, 16.0F ),
            ._lineHeight = LengthValue ( LengthValue::eType::Auto, 42.0F ),
            ._marginBottom = LengthValue ( LengthValue::eType::PX, 8.0F ),
            ._marginLeft = LengthValue ( LengthValue::eType::PX, 8.0F ),
            ._marginRight = LengthValue ( LengthValue::eType::PX, 8.0F ),
            ._marginTop = LengthValue ( LengthValue::eType::PX, 8.0F ),
            ._paddingBottom = LengthValue ( LengthValue::eType::PX, 0.0F ),
            ._paddingLeft = LengthValue ( LengthValue::eType::PX, 0.0F ),
            ._paddingRight = LengthValue ( LengthValue::eType::PX, 0.0F ),
            ._paddingTop = LengthValue ( LengthValue::eType::PX, 0.0F ),
            ._position = PositionProperty::eValue::Static,
            ._textAlign = TextAlignProperty::eValue::Left,
            ._verticalAlign = VerticalAlignProperty::eValue::Top,
            ._width = LengthValue ( LengthValue::eType::Percent, 100.0F ),
            ._height = LengthValue ( LengthValue::eType::Percent, 100.0F ),
        };

        std::u32string                          _bodyID {};
        CSSParser                               _css {};

    public:
        HTML5Parser () = default;

        HTML5Parser ( HTML5Parser const & ) = delete;
        HTML5Parser &operator = ( HTML5Parser const & ) = delete;

        HTML5Parser ( HTML5Parser && ) = delete;
        HTML5Parser &operator = ( HTML5Parser && ) = delete;

        ~HTML5Parser () = default;

        [[nodiscard]] CSSComputedValues &GetBodyCSS () noexcept;
        [[nodiscard]] HTML5Children &GetBodyChildren () noexcept;
        [[nodiscard]] std::u32string &GetBodyID () noexcept;
        [[nodiscard]] CSSParser &GetCSSParser () noexcept;

        [[nodiscard]] bool Parse ( char const* html, Stream stream, char const* assetRoot ) noexcept;

    private:
        [[nodiscard]] ParseResult ParseBodyElement ( char const* html, Stream stream, char const* assetRoot ) noexcept;
        [[nodiscard]] bool ParseHTMLElement ( char const* html, Stream stream, char const* assetRoot ) noexcept;
        [[nodiscard]] ParseResult ParseHeadElement ( char const* html, Stream stream, char const* assetRoot ) noexcept;
};

} // namespace pbr


#endif // PBR_HTML5_PARSER_HPP
