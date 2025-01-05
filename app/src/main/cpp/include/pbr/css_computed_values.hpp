#ifndef PBR_CSS_COMPUTED_VALUES_HPP
#define PBR_CSS_COMPUTED_VALUES_HPP


#include "color_value.hpp"
#include "css_parser.hpp"
#include "display_property.hpp"
#include "length_value.hpp"
#include "position_property.hpp"
#include "text_align_property.hpp"
#include "vertical_align_property.hpp"
#include <GXCommon/GXMath.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <string>
#include <unordered_set>

GX_RESTORE_WARNING_STATE


namespace pbr {

struct CSSComputedValues final
{
    ColorValue                          _backgroundColor;
    LengthValue                         _backgroundSize;

    LengthValue                         _bottom;
    LengthValue                         _left;
    LengthValue                         _right;
    LengthValue                         _top;

    ColorValue                          _color;
    DisplayProperty::eValue             _display = DisplayProperty::eValue::Block;

    // By convention empty string means 'inherit'.
    std::string                         _fontFile;

    LengthValue                         _fontSize;
    LengthValue                         _lineHeight;

    LengthValue                         _marginBottom;
    LengthValue                         _marginLeft;
    LengthValue                         _marginRight;
    LengthValue                         _marginTop;

    LengthValue                         _paddingBottom;
    LengthValue                         _paddingLeft;
    LengthValue                         _paddingRight;
    LengthValue                         _paddingTop;

    PositionProperty::eValue            _position = PositionProperty::eValue::Static;
    TextAlignProperty::eValue           _textAlign = TextAlignProperty::eValue::Inherit;
    VerticalAlignProperty::eValue       _verticalAlign = VerticalAlignProperty::eValue::Top;

    LengthValue                         _width;
    LengthValue                         _height;

    [[nodiscard]] bool ApplyCSS ( char const* html,
        CSSParser const &css,
        std::unordered_set<std::u32string> const &classes,
        std::u32string const &id
    ) noexcept;
};

} // namespace pbr


#endif // PBR_CSS_COMPUTED_VALUES_HPP
