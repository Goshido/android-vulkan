#include <precompiled_headers.hpp>
#include <cursor.hpp>
#include <glyphs.hpp>
#include <theme.hpp>
#include <ui_combobox.hpp>


namespace editor {


UICombobox::UICombobox ( MessageQueue &messageQueue,
    pbr::DIVUIElement &parent,
    std::string_view caption,
    std::string &&name
) noexcept:
    Widget ( messageQueue ),

    _lineDIV ( &parent,

        {
            ._backgroundColor = theme::TRANSPARENT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::AUTO_LENGTH,
            ._right = theme::ZERO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = theme::TRANSPARENT_COLOR,
            ._display = pbr::DisplayProperty::eValue::Block,
            ._fontFile {},
            ._fontSize = theme::ZERO_LENGTH,
            ._lineHeight = theme::AUTO_LENGTH,
            ._marginBottom = theme::ZERO_LENGTH,
            ._marginLeft = theme::ZERO_LENGTH,
            ._marginRight = theme::ZERO_LENGTH,
            ._marginTop = theme::ZERO_LENGTH,
            ._paddingBottom = pbr::LengthValue ( pbr::LengthValue::eType::PX, 6.0F ),
            ._paddingLeft = pbr::LengthValue ( pbr::LengthValue::eType::PX, 6.0F ),
            ._paddingRight = pbr::LengthValue ( pbr::LengthValue::eType::PX, 6.0F ),
            ._paddingTop = pbr::LengthValue ( pbr::LengthValue::eType::PX, 6.0F ),
            ._position = pbr::PositionProperty::eValue::Static,
            ._textAlign = pbr::TextAlignProperty::eValue::Left,
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
            ._width = theme::AUTO_LENGTH,
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::PX, 20.0F )
        },

        name + " (line)"
    ),

    _columnDIV ( &_lineDIV,

        {
            ._backgroundColor = theme::TRANSPARENT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::AUTO_LENGTH,
            ._right = theme::ZERO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = theme::TRANSPARENT_COLOR,
            ._display = pbr::DisplayProperty::eValue::Block,
            ._fontFile {},
            ._fontSize = theme::INHERIT_LENGTH,
            ._lineHeight = theme::AUTO_LENGTH,
            ._marginBottom = theme::ZERO_LENGTH,
            ._marginLeft = theme::ZERO_LENGTH,
            ._marginRight = theme::ZERO_LENGTH,
            ._marginTop = theme::ZERO_LENGTH,
            ._paddingBottom = theme::ZERO_LENGTH,
            ._paddingLeft = theme::ZERO_LENGTH,
            ._paddingRight = theme::ZERO_LENGTH,
            ._paddingTop = theme::ZERO_LENGTH,
            ._position = pbr::PositionProperty::eValue::Static,
            ._textAlign = pbr::TextAlignProperty::eValue::Left,
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
            ._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F ),
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F )
        },

        name + " (column)"
    ),

    _captionDIV ( &_columnDIV,

        {
            ._backgroundColor = theme::TRANSPARENT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::AUTO_LENGTH,
            ._right = theme::ZERO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = theme::TEXT_COLOR_NORMAL,
            ._display = pbr::DisplayProperty::eValue::InlineBlock,
            ._fontFile { theme::NORMAL_FONT_FAMILY },
            ._fontSize = theme::NORMAL_FONT_SIZE,
            ._lineHeight = theme::AUTO_LENGTH,
            ._marginBottom = theme::ZERO_LENGTH,
            ._marginLeft = theme::ZERO_LENGTH,
            ._marginRight = theme::ZERO_LENGTH,
            ._marginTop = theme::ZERO_LENGTH,
            ._paddingBottom = theme::ZERO_LENGTH,
            ._paddingLeft = theme::ZERO_LENGTH,
            ._paddingRight = theme::ZERO_LENGTH,
            ._paddingTop = pbr::LengthValue ( pbr::LengthValue::eType::PX, 4.5F ),
            ._position = pbr::PositionProperty::eValue::Static,
            ._textAlign = pbr::TextAlignProperty::eValue::Left,
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
            ._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 35.0F ),
            ._height = theme::AUTO_LENGTH
        },

        name + " (caption)"
    ),

    _captionText ( true, &_captionDIV, caption, name + " (caption)" ),

    _valueDIV ( &_columnDIV,

        {
            ._backgroundColor = theme::WIDGET_BACKGROUND_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::AUTO_LENGTH,
            ._right = theme::ZERO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = theme::MAIN_COLOR,
            ._display = pbr::DisplayProperty::eValue::InlineBlock,
            ._fontFile {},
            ._fontSize = theme::INHERIT_LENGTH,
            ._lineHeight = theme::AUTO_LENGTH,
            ._marginBottom = theme::ZERO_LENGTH,
            ._marginLeft = theme::ZERO_LENGTH,
            ._marginRight = theme::ZERO_LENGTH,
            ._marginTop = theme::ZERO_LENGTH,
            ._paddingBottom = theme::ZERO_LENGTH,
            ._paddingLeft = theme::ZERO_LENGTH,
            ._paddingRight = theme::ZERO_LENGTH,
            ._paddingTop = pbr::LengthValue ( pbr::LengthValue::eType::PX, 4.5F ),
            ._position = pbr::PositionProperty::eValue::Relative,
            ._textAlign = pbr::TextAlignProperty::eValue::Inherit,
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
            ._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 65.0F ),
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F )
        },

        name + " (value)"
    ),

    _textDIV ( &_valueDIV,

        {
            ._backgroundColor = theme::TRANSPARENT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::AUTO_LENGTH,
            ._right = theme::ZERO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = theme::INHERIT_COLOR,
            ._display = pbr::DisplayProperty::eValue::Block,
            ._fontFile {},
            ._fontSize = theme::NORMAL_FONT_SIZE,
            ._lineHeight = theme::AUTO_LENGTH,
            ._marginBottom = theme::ZERO_LENGTH,
            ._marginLeft = theme::ZERO_LENGTH,
            ._marginRight = theme::ZERO_LENGTH,
            ._marginTop = theme::ZERO_LENGTH,
            ._paddingBottom = theme::ZERO_LENGTH,
            ._paddingLeft = theme::ZERO_LENGTH,
            ._paddingRight = theme::ZERO_LENGTH,
            ._paddingTop = theme::ZERO_LENGTH,
            ._position = pbr::PositionProperty::eValue::Static,
            ._textAlign = pbr::TextAlignProperty::eValue::Inherit,
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
            ._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F ),
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F )
        },

        name + " (text)"
    ),

    _text ( true, &_textDIV, "1920x1080", name + " (text)" ),

    _iconDIV ( &_valueDIV,

        {
            ._backgroundColor = theme::TRANSPARENT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::AUTO_LENGTH,
            ._right = pbr::LengthValue ( pbr::LengthValue::eType::PX, 2.0F ),
            ._top = pbr::LengthValue ( pbr::LengthValue::eType::PX, 4.5F ),
            ._color = theme::INHERIT_COLOR,
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
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Inherit,
            ._width = pbr::LengthValue ( pbr::LengthValue::eType::PX, 20.0F ),
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::PX, 14.0F )
        },

        name + " (icon)"
    ),

    _icon ( true, &_iconDIV, glyph::COMBOBOX_DOWN, name + " (icon)" )
{
    _captionDIV.AppendChildElement ( _captionText );
    _columnDIV.AppendChildElement ( _captionDIV );

    _textDIV.AppendChildElement ( _text );
    _valueDIV.AppendChildElement ( _textDIV );

    _iconDIV.AppendChildElement ( _icon );
    _valueDIV.AppendChildElement ( _iconDIV );

    _columnDIV.AppendChildElement ( _valueDIV );

    _lineDIV.AppendChildElement ( _columnDIV );
    parent.AppendChildElement ( _lineDIV );
}

void UICombobox::OnMouseKeyDown ( MouseKeyEvent const &event ) noexcept
{
    if ( event._key == eKey::LeftMouseButton ) [[likely]]
    {
        _text.SetColor ( theme::PRESS_COLOR );
        _icon.SetColor ( theme::PRESS_COLOR );
    }
}

void UICombobox::OnMouseKeyUp ( MouseKeyEvent const &event ) noexcept
{
    if ( event._key != eKey::LeftMouseButton ) [[unlikely]]
        return;

    _text.SetColor ( theme::HOVER_COLOR );
    _icon.SetColor ( theme::HOVER_COLOR );
}

void UICombobox::OnMouseMove ( MouseMoveEvent const &event ) noexcept
{
    Widget::OnMouseMove ( event );

    if ( event._eventID - std::exchange ( _eventID, event._eventID ) < 2U ) [[likely]]
        return;

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::ChangeCursor,
            ._params = reinterpret_cast<void*> ( eCursor::Arrow ),
            ._serialNumber = 0U
        }
    );

    _text.SetColor ( theme::HOVER_COLOR );
    _icon.SetColor ( theme::HOVER_COLOR );
}

void UICombobox::UpdatedRect () noexcept
{
    _rect.From ( _valueDIV.GetAbsoluteRect () );
}

void UICombobox::Connect ( Callback &&callback ) noexcept
{
    _callback = std::move ( callback );
}

pbr::CSSComputedValues &UICombobox::GetCSS () noexcept
{
    return _lineDIV.GetCSS ();
}

void UICombobox::OnMouseLeave () noexcept
{
    _text.SetColor ( theme::MAIN_COLOR );
    _icon.SetColor ( theme::MAIN_COLOR );
}

} // namespace editor
