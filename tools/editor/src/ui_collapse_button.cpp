#include <precompiled_headers.hpp>
#include <cursor.hpp>
#include <glyphs.hpp>
#include <theme.hpp>
#include <ui_collapse_button.hpp>


namespace editor {

UICollapseButton::UICollapseButton ( DIVUIElement &parent, bool collapse, std::string &&name ) noexcept:
    _div ( parent,

        {
            ._backgroundColor = theme::TRANSPARENT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::AUTO_LENGTH,
            ._right = pbr::LengthValue ( pbr::LengthValue::eType::PX, 5.0F ),
            ._top = pbr::LengthValue ( pbr::LengthValue::eType::PX, 1.0F ),
            ._color = theme::MAIN_COLOR,
            ._display = pbr::DisplayProperty::eValue::Block,
            ._fontFile { glyph::FONT_FAMILY },
            ._fontSize = pbr::LengthValue ( pbr::LengthValue::eType::PX, 11.0F ),
            ._lineHeight = theme::AUTO_LENGTH,
            ._marginBottom = theme::ZERO_LENGTH,
            ._marginLeft = theme::ZERO_LENGTH,
            ._marginRight = theme::ZERO_LENGTH,
            ._marginTop = theme::ZERO_LENGTH,
            ._paddingBottom = theme::ZERO_LENGTH,
            ._paddingLeft = theme::ZERO_LENGTH,
            ._paddingRight = pbr::LengthValue ( pbr::LengthValue::eType::PX, 3.0F ),
            ._paddingTop = theme::ZERO_LENGTH,
            ._position = pbr::PositionProperty::eValue::Absolute,
            ._textAlign = pbr::TextAlignProperty::eValue::Right,
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
            ._width = pbr::LengthValue ( pbr::LengthValue::eType::PX, 20.0F ),
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::PX, 14.0F )
        },

        name + " (collapse button)"
    ),

    _icon ( _div, collapse ? glyph::COMBOBOX_UP : glyph::COMBOBOX_DOWN, name + " (collapse button)" ),
    _collapse ( collapse )
{
    _div.AppendChildElement ( _icon );
    parent.AppendChildElement ( _div );
}

void UICollapseButton::OnMouseButtonDown ( MouseButtonEvent const &event ) noexcept
{
    if ( event._key == eKey::LeftMouseButton ) [[likely]]
    {
        _icon.SetColor ( theme::PRESS_COLOR );
    }
}

void UICollapseButton::OnMouseButtonUp ( MouseButtonEvent const &event ) noexcept
{
    if ( event._key != eKey::LeftMouseButton ) [[unlikely]]
        return;

    _collapse = !_collapse;
    _icon.SetColor ( theme::HOVER_COLOR );
    _icon.SetText ( _collapse ? glyph::COMBOBOX_UP : glyph::COMBOBOX_DOWN );

    if ( _callback ) [[likely]]
    {
        _callback ( _collapse );
    }
}

void UICollapseButton::OnMouseMove ( MouseMoveEvent const &event ) noexcept
{
    Widget::OnMouseMove ( event );

    if ( event._eventID - std::exchange ( _eventID, event._eventID ) < 2U ) [[likely]]
        return;

    ChangeCursor ( eCursor::Arrow );
    _icon.SetColor ( theme::HOVER_COLOR );
}

void UICollapseButton::UpdatedRect () noexcept
{
    _rect.From ( _div.GetAbsoluteRect () );
}

void UICollapseButton::Connect ( Callback &&callback ) noexcept
{
    _callback = std::move ( callback );
}

void UICollapseButton::DumpLayout ( pbr::SVGWriter &writer ) const noexcept
{
    _div.DumpLayout ( writer );
}

void UICollapseButton::OnMouseLeave () noexcept
{
    _icon.SetColor ( theme::MAIN_COLOR );
}

} // namespace editor
