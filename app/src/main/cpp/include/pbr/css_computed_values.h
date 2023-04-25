#ifndef PBR_CSS_COMPUTED_VALUES_H
#define PBR_CSS_COMPUTED_VALUES_H


#include "color_value.h"
#include "css_parser.h"
#include "display_property.h"
#include "length_value.h"
#include "position_property.h"
#include "text_align_property.h"
#include "vertical_align_property.h"
#include <GXCommon/GXMath.h>

GX_DISABLE_COMMON_WARNINGS

#include <string>
#include <unordered_set>

GX_RESTORE_WARNING_STATE


namespace pbr {

class CSSComputedValues final
{
    public:
        ColorValue                          _backgroundColor;
        LengthValue                         _backgroundSize;

        ColorValue                          _color;
        DisplayProperty::eValue             _display;

        // By convention empty string means 'inherit'.
        std::string                         _fontFile;

        LengthValue                         _fontSize;

        LengthValue                         _marginBottom;
        LengthValue                         _marginLeft;
        LengthValue                         _marginRight;
        LengthValue                         _marginTop;

        LengthValue                         _paddingBottom;
        LengthValue                         _paddingLeft;
        LengthValue                         _paddingRight;
        LengthValue                         _paddingTop;

        PositionProperty::eValue            _position;
        TextAlignProperty::eValue           _textAlign;
        VerticalAlignProperty::eValue       _verticalAlign;

        LengthValue                         _width;
        LengthValue                         _height;

    public:
        CSSComputedValues () = default;

        CSSComputedValues ( CSSComputedValues const & ) = delete;
        CSSComputedValues& operator = ( CSSComputedValues const & ) = delete;

        CSSComputedValues ( CSSComputedValues && ) = delete;
        CSSComputedValues& operator = ( CSSComputedValues && ) = delete;

        ~CSSComputedValues () = default;

        [[nodiscard]] bool ApplyCSS ( char const* html,
            CSSParser const &css,
            std::unordered_set<std::u32string> const &classes,
            std::u32string const &id
        ) noexcept;
};

} // namespace pbr


#endif // PBR_CSS_COMPUTED_VALUES_H
