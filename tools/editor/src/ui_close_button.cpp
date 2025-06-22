#include <precompiled_headers.hpp>
#include <cursor.hpp>
#include <glyphs.hpp>
#include <theme.hpp>
#include <ui_close_button.hpp>


namespace editor {

UICloseButton::UICloseButton ( MessageQueue &messageQueue, DIVUIElement &parent, std::string &&name ) noexcept:
    Widget ( messageQueue ),

    _base ( messageQueue,
        parent,

        {
            ._backgroundColor = theme::TRANSPARENT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::AUTO_LENGTH,
            ._right = theme::ZERO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = theme::TRANSPARENT_COLOR,
            ._display = pbr::DisplayProperty::eValue::InlineBlock,
            ._fontFile { glyph::FONT_FAMILY },
            ._fontSize = pbr::LengthValue ( pbr::LengthValue::eType::PX, 20.0F ),
            ._lineHeight = pbr::LengthValue ( pbr::LengthValue::eType::PX, 22.0 ),
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
            ._width = pbr::LengthValue ( pbr::LengthValue::eType::PX, 21.0F ),
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::PX, 21.0F )
        },

        name + " (base)"
    ),

    _backgroundDIV ( messageQueue,
        _base,

        {
            ._backgroundColor = theme::TRANSPARENT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::AUTO_LENGTH,
            ._right = theme::ZERO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = theme::MAIN_COLOR,
            ._display = pbr::DisplayProperty::eValue::InlineBlock,
            ._fontFile = "",
            ._fontSize = pbr::LengthValue ( pbr::LengthValue::eType::Inherit, 42.0F ),
            ._lineHeight = theme::INHERIT_LENGTH,
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
    ),

    _backgroundText ( messageQueue, _backgroundDIV, glyph::CLOSE_BUTTON_BACKGROUND, name + " (background text)" ),

    _borderDIV ( messageQueue,
        _base,

        {
            ._backgroundColor = theme::TRANSPARENT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::AUTO_LENGTH,
            ._right = theme::ZERO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = theme::BORDER_COLOR,
            ._display = pbr::DisplayProperty::eValue::InlineBlock,
            ._fontFile = "",
            ._fontSize = pbr::LengthValue ( pbr::LengthValue::eType::Inherit, 42.0F ),
            ._lineHeight = theme::INHERIT_LENGTH,
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
    ),

    _borderText ( messageQueue, _borderDIV, glyph::CLOSE_BUTTON_BORDER, name + " (border text)" ),

    _crossDIV ( messageQueue,
        _base,

        {
            ._backgroundColor = theme::TRANSPARENT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::AUTO_LENGTH,
            ._right = theme::ZERO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = theme::BORDER_COLOR,
            ._display = pbr::DisplayProperty::eValue::InlineBlock,
            ._fontFile = "",
            ._fontSize = pbr::LengthValue ( pbr::LengthValue::eType::Inherit, 42.0F ),
            ._lineHeight = theme::INHERIT_LENGTH,
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
    ),

    _crossText ( messageQueue, _crossDIV, glyph::CLOSE_BUTTON_CROSS, name + " (cross text)" )
{
    _backgroundDIV.AppendChildElement ( _backgroundText );

    _base.AppendChildElement ( _backgroundDIV );

    _borderDIV.AppendChildElement ( _borderText );
    _base.AppendChildElement ( _borderDIV );

    _crossDIV.AppendChildElement ( _crossText );
    _base.AppendChildElement ( _crossDIV );

    parent.AppendChildElement ( _base );
}

void UICloseButton::OnMouseButtonDown ( MouseButtonEvent const &event ) noexcept
{
    if ( event._key == eKey::LeftMouseButton ) [[likely]]
    {
        _backgroundText.SetColor ( theme::PRESS_COLOR );
    }
}

void UICloseButton::OnMouseButtonUp ( MouseButtonEvent const &event ) noexcept
{
    if ( event._key == eKey::LeftMouseButton ) [[likely]]
    {
        _callback ();
        _backgroundText.SetColor ( theme::HOVER_COLOR );
    }
}

void UICloseButton::OnMouseMove ( MouseMoveEvent const &event ) noexcept
{
    Widget::OnMouseMove ( event );

    if ( event._eventID - std::exchange ( _eventID, event._eventID ) < 2U ) [[likely]]
        return;

    ChangeCursor ( eCursor::Arrow );
    _backgroundText.SetColor ( theme::HOVER_COLOR );
}

void UICloseButton::UpdatedRect () noexcept
{
    _rect.From ( _base.GetAbsoluteRect () );
}

void UICloseButton::Connect ( Callback &&callback ) noexcept
{
    _callback = std::move ( callback );
}

pbr::CSSComputedValues &UICloseButton::GetCSS () noexcept
{
    return _base.GetCSS ();
}

void UICloseButton::OnMouseLeave () noexcept
{
    _backgroundText.SetColor ( theme::MAIN_COLOR );
}

} // namespace editor
