#include <precompiled_headers.hpp>
#include <glyphs.hpp>
#include <theme.hpp>
#include <ui_checkbox.hpp>

namespace editor {

UICheckbox::UICheckbox ( MessageQueue &messageQueue,
    pbr::DIVUIElement &parent,
    std::string_view text,
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

    _nameDIV ( &_columnDIV,

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
            ._paddingTop = theme::ZERO_LENGTH,
            ._position = pbr::PositionProperty::eValue::Static,
            ._textAlign = pbr::TextAlignProperty::eValue::Left,
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
            ._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 35.0F ),
            ._height = theme::AUTO_LENGTH
        },

        name + " (name)"
    ),

    _nameText ( true, &_nameDIV, text, name + " (name)" ),

    _valueDIV ( &_columnDIV,

        {
            ._backgroundColor = theme::TRANSPARENT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::AUTO_LENGTH,
            ._right = theme::ZERO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = theme::MAIN_COLOR,
            ._display = pbr::DisplayProperty::eValue::InlineBlock,
            ._fontFile { glyph::FONT_FAMILY },
            ._fontSize = theme::NORMAL_FONT_SIZE,
            ._lineHeight = theme::AUTO_LENGTH,
            ._marginBottom = theme::ZERO_LENGTH,
            ._marginLeft = theme::ZERO_LENGTH,
            ._marginRight = theme::ZERO_LENGTH,
            ._marginTop = pbr::LengthValue ( pbr::LengthValue::eType::PX, 1.5F ),
            ._paddingBottom = theme::ZERO_LENGTH,
            ._paddingLeft = theme::ZERO_LENGTH,
            ._paddingRight = theme::ZERO_LENGTH,
            ._paddingTop = theme::ZERO_LENGTH,
            ._position = pbr::PositionProperty::eValue::Static,
            ._textAlign = pbr::TextAlignProperty::eValue::Left,
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
            ._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 65.0F ),
            ._height = theme::AUTO_LENGTH
        },

        name + " (value)"
    ),

    _valueIcon ( true, &_valueDIV, glyph::CHECKBOX_CHECK, name + " (icon)" )
{
    _nameDIV.AppendChildElement ( _nameText );
    _columnDIV.AppendChildElement ( _nameDIV );

    _valueDIV.AppendChildElement ( _valueIcon );
    _columnDIV.AppendChildElement ( _valueDIV );

    _lineDIV.AppendChildElement ( _columnDIV );
    parent.AppendChildElement ( _lineDIV );
}

void UICheckbox::OnMouseKeyDown ( MouseKeyEvent const &event ) noexcept
{
    if ( event._key == eKey::LeftMouseButton ) [[likely]]
    {
        _valueIcon.SetColor ( theme::PRESS_COLOR );
    }
}

void UICheckbox::OnMouseKeyUp ( MouseKeyEvent const &event ) noexcept
{
    if ( event._key != eKey::LeftMouseButton ) [[unlikely]]
        return;

    switch ( _state )
    {
        case editor::UICheckbox::eState::Check:
            _state = eState::Unckeck;
            _valueIcon.SetText ( glyph::CHECKBOX_UNCHECK );
        break;

        case editor::UICheckbox::eState::Unckeck:
            [[fallthrough]];
        case editor::UICheckbox::eState::Multi:
            _state = eState::Check;
            _valueIcon.SetText ( glyph::CHECKBOX_CHECK );
        break;

        default:
            // IMPOSSIBLE
        break;
    }

    _callback ( _state );
    _valueIcon.SetColor ( theme::HOVER_COLOR );
}

void UICheckbox::OnMouseMove ( MouseMoveEvent const &event ) noexcept
{
    Widget::OnMouseMove ( event );

    if ( event._eventID - std::exchange ( _eventID, event._eventID ) > 1U ) [[unlikely]]
    {
        _valueIcon.SetColor ( theme::HOVER_COLOR );
    }
}

void UICheckbox::UpdatedRect () noexcept
{
    _rect.From ( _valueDIV.GetAbsoluteRect () );
}

void UICheckbox::Connect ( Callback &&callback ) noexcept
{
    _callback = std::move ( callback );
}

pbr::CSSComputedValues &UICheckbox::GetCSS () noexcept
{
    return _lineDIV.GetCSS ();
}

void UICheckbox::OnMouseLeave () noexcept
{
    _valueIcon.SetColor ( theme::MAIN_COLOR );
}

} // namespace editor
