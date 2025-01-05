#include <precompiled_headers.hpp>
#include <glyphs.hpp>
#include <theme.hpp>
#include <ui_close_button.hpp>


namespace editor {

UICloseButton::UICloseButton ( MessageQueue &messageQueue, pbr::DIVUIElement &parent, std::string &&name ) noexcept:
    _messageQueue ( messageQueue ),

    _base (
        std::make_unique<pbr::DIVUIElement> ( &parent,

            pbr::CSSComputedValues
            {
                ._backgroundColor = theme::TRANSPARENT_COLOR,
                ._backgroundSize = theme::ZERO_LENGTH,
                ._bottom = theme::AUTO_LENGTH,
                ._left = theme::AUTO_LENGTH,
                ._right = theme::ZERO_LENGTH,
                ._top = theme::ZERO_LENGTH,
                ._color = theme::TRANSPARENT_COLOR,
                ._display = pbr::DisplayProperty::eValue::InlineBlock,
                ._fontFile { glyph::FONT_FAMILY.data (), glyph::FONT_FAMILY.size () },
                ._fontSize = pbr::LengthValue ( pbr::LengthValue::eType::PX, 20.0F ),
                ._lineHeight = pbr::LengthValue ( pbr::LengthValue::eType::PX, 20.0F ),
                ._marginBottom = theme::ZERO_LENGTH,
                ._marginLeft = theme::ZERO_LENGTH,
                ._marginRight = theme::ZERO_LENGTH,
                ._marginTop = theme::ZERO_LENGTH,
                ._paddingBottom = theme::ZERO_LENGTH,
                ._paddingLeft = theme::ZERO_LENGTH,
                ._paddingRight = theme::ZERO_LENGTH,
                ._paddingTop = theme::ZERO_LENGTH,
                ._position = pbr::PositionProperty::eValue::Absolute,
                ._textAlign = pbr::TextAlignProperty::eValue::Right,
                ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
                ._width = theme::SMALL_BUTTON_HEIGHT,
                ._height = theme::SMALL_BUTTON_HEIGHT
            },

            name + " (base)"
        )
    ),

    _backgroundDIV (
        std::make_unique<pbr::DIVUIElement> ( _base.get (),

            pbr::CSSComputedValues
            {
                ._backgroundColor = theme::TRANSPARENT_COLOR,
                ._backgroundSize = theme::ZERO_LENGTH,
                ._bottom = theme::AUTO_LENGTH,
                ._left = theme::AUTO_LENGTH,
                ._right = theme::ZERO_LENGTH,
                ._top = theme::ZERO_LENGTH,
                ._color = pbr::ColorValue ( 106U, 172U, 0U, 255U ),
                ._display = pbr::DisplayProperty::eValue::InlineBlock,
                ._fontFile = "",
                ._fontSize = pbr::LengthValue ( pbr::LengthValue::eType::EM, 1.0F ),
                ._lineHeight = theme::AUTO_LENGTH,
                ._marginBottom = theme::ZERO_LENGTH,
                ._marginLeft = theme::ZERO_LENGTH,
                ._marginRight = theme::ZERO_LENGTH,
                ._marginTop = theme::ZERO_LENGTH,
                ._paddingBottom = theme::ZERO_LENGTH,
                ._paddingLeft = theme::ZERO_LENGTH,
                ._paddingRight = theme::ZERO_LENGTH,
                ._paddingTop = theme::ZERO_LENGTH,
                ._position = pbr::PositionProperty::eValue::Absolute,
                ._textAlign = pbr::TextAlignProperty::eValue::Inherit,
                ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
                ._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F ),
                ._height = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F )
            },

            name + " (background DIV)"
        )
    ),

    _backgroundText (
        std::make_unique<pbr::TextUIElement> ( true,
            _backgroundDIV.get (),
            glyph::CLOSE_BUTTON_BACKGROUND,
            name + " (background text)"
        )
    ),

    _borderDIV (
        std::make_unique<pbr::DIVUIElement> ( _base.get (),

            pbr::CSSComputedValues
            {
                ._backgroundColor = theme::TRANSPARENT_COLOR,
                ._backgroundSize = theme::ZERO_LENGTH,
                ._bottom = theme::AUTO_LENGTH,
                ._left = theme::AUTO_LENGTH,
                ._right = theme::ZERO_LENGTH,
                ._top = theme::ZERO_LENGTH,
                ._color = pbr::ColorValue ( 255U, 255U, 255U, 255U ),
                ._display = pbr::DisplayProperty::eValue::InlineBlock,
                ._fontFile = "",
                ._fontSize = pbr::LengthValue ( pbr::LengthValue::eType::EM, 1.0F ),
                ._lineHeight = theme::AUTO_LENGTH,
                ._marginBottom = theme::ZERO_LENGTH,
                ._marginLeft = theme::ZERO_LENGTH,
                ._marginRight = theme::ZERO_LENGTH,
                ._marginTop = theme::ZERO_LENGTH,
                ._paddingBottom = theme::ZERO_LENGTH,
                ._paddingLeft = theme::ZERO_LENGTH,
                ._paddingRight = theme::ZERO_LENGTH,
                ._paddingTop = theme::ZERO_LENGTH,
                ._position = pbr::PositionProperty::eValue::Absolute,
                ._textAlign = pbr::TextAlignProperty::eValue::Inherit,
                ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
                ._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F ),
                ._height = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F )
            },

            name + " (border DIV)"
        )
    ),

    _borderText (
        std::make_unique<pbr::TextUIElement> ( true,
            _borderDIV.get (),
            glyph::CLOSE_BUTTON_BORDER,
            name + " (border text)"
        )
    ),

    _crossDIV (
        std::make_unique<pbr::DIVUIElement> ( _base.get (),

            pbr::CSSComputedValues
            {
                ._backgroundColor = theme::TRANSPARENT_COLOR,
                ._backgroundSize = theme::ZERO_LENGTH,
                ._bottom = theme::AUTO_LENGTH,
                ._left = theme::AUTO_LENGTH,
                ._right = theme::ZERO_LENGTH,
                ._top = theme::ZERO_LENGTH,
                ._color = pbr::ColorValue ( 0U, 0U, 0U, 255U ),
                ._display = pbr::DisplayProperty::eValue::InlineBlock,
                ._fontFile = "",
                ._fontSize = pbr::LengthValue ( pbr::LengthValue::eType::EM, 1.0F ),
                ._lineHeight = theme::AUTO_LENGTH,
                ._marginBottom = theme::ZERO_LENGTH,
                ._marginLeft = theme::ZERO_LENGTH,
                ._marginRight = theme::ZERO_LENGTH,
                ._marginTop = theme::ZERO_LENGTH,
                ._paddingBottom = theme::ZERO_LENGTH,
                ._paddingLeft = theme::ZERO_LENGTH,
                ._paddingRight = theme::ZERO_LENGTH,
                ._paddingTop = theme::ZERO_LENGTH,
                ._position = pbr::PositionProperty::eValue::Absolute,
                ._textAlign = pbr::TextAlignProperty::eValue::Inherit,
                ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
                ._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F ),
                ._height = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F )
            },

            name + " (cross DIV)"
        )
    ),

    _crossText (
        std::make_unique<pbr::TextUIElement> ( true,
            _crossDIV.get (),
            glyph::CLOSE_BUTTON_CROSS,
            name + " (cross text)"
        )
    )
{
    pbr::DIVUIElement &backgroundDIV = *_backgroundDIV;
    backgroundDIV.AppendChildElement ( *_backgroundText );

    pbr::DIVUIElement &base= *_base;
    base.AppendChildElement ( backgroundDIV );

    pbr::DIVUIElement &borderDIV = *_borderDIV;
    borderDIV.AppendChildElement ( *_borderText );
    base.AppendChildElement ( borderDIV );

    pbr::DIVUIElement &crossDIV = *_crossDIV;
    crossDIV.AppendChildElement ( *_crossText );
    base.AppendChildElement ( crossDIV );

    parent.AppendChildElement ( base );
}

pbr::CSSComputedValues &UICloseButton::GetCSS () noexcept
{
    return _base->GetCSS ();
}

} // namespace editor
