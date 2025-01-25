#include <precompiled_headers.hpp>
#include <theme.hpp>
#include <ui_props.hpp>


namespace editor {

UIProps::UIProps ( MessageQueue &messageQueue ) noexcept:
    UIDialogBox ( messageQueue, "Properties" ),

    _headerLine ( &_div,

        pbr::CSSComputedValues
        {
            ._backgroundColor = theme::HEADER_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::AUTO_LENGTH,
            ._left = theme::AUTO_LENGTH,
            ._right = theme::ZERO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = theme::TRANSPARENT_COLOR,
            ._display = pbr::DisplayProperty::eValue::Block,
            ._fontFile { theme::NORMAL_FONT_FAMILY.data (), theme::NORMAL_FONT_FAMILY.size () },
            ._fontSize = theme::HEADER_FONT_SIZE,
            ._lineHeight = theme::AUTO_LENGTH,
            ._marginBottom = theme::ZERO_LENGTH,
            ._marginLeft = theme::ZERO_LENGTH,
            ._marginRight = theme::ZERO_LENGTH,
            ._marginTop = theme::ZERO_LENGTH,
            ._paddingBottom = theme::ZERO_LENGTH,
            ._paddingLeft = theme::ZERO_LENGTH,
            ._paddingRight = theme::ZERO_LENGTH,
            ._paddingTop = theme::ZERO_LENGTH,
            ._position = pbr::PositionProperty::eValue::Relative,
            ._textAlign = pbr::TextAlignProperty::eValue::Left,
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
            ._width =  pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F ),
            ._height = theme::HEADER_HEIGHT
        },

        "Header line"
    ),

    _headerText ( messageQueue, _headerLine, "Properties", "Header" ),
    _closeButton ( messageQueue, _headerLine, "Close button" ),
    _checkbox ( messageQueue, _div, "Shadows", "Checkbox" ),
    _combobox ( messageQueue, _div, "Resolution", "Combobox" )
{
    pbr::CSSComputedValues &headerTextStyle = _headerText.GetCSS ();
    headerTextStyle._fontSize = theme::HEADER_FONT_SIZE;
    headerTextStyle._textAlign = pbr::TextAlignProperty::eValue::Center;
    headerTextStyle._paddingTop = theme::HEADER_VERTICAL_PADDING;
    headerTextStyle._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F );

    pbr::CSSComputedValues &closeButtonStyle = _closeButton.GetCSS ();
    closeButtonStyle._top = pbr::LengthValue ( pbr::LengthValue::eType::PX, 4.0F );
    closeButtonStyle._right = pbr::LengthValue ( pbr::LengthValue::eType::PX, 4.0F );

    _closeButton.Connect (
        [ this ] () noexcept {
            OnClose ();
        }
    );

    _checkbox.Connect (
        [ this ] ( UICheckbox::eState state ) noexcept {
            OnCheckBox ( state );
        }
    );

    _combobox.Connect (
        [ this ] ( uint32_t value ) noexcept {
            OnCombobox ( value );
        }
    );

    _div.PrependChildElement ( _headerLine );
}

void UIProps::OnMouseKeyDown ( MouseKeyEvent const &event ) noexcept
{
    if ( _closeButton.IsOverlapped ( event._x, event._y ) )
    {
        _closeButton.OnMouseKeyDown ( event );
        return;
    }

    if ( _checkbox.IsOverlapped ( event._x, event._y ) )
    {
        _checkbox.OnMouseKeyDown ( event );
        return;
    }

    if ( _combobox.IsOverlapped ( event._x, event._y ) )
    {
        _combobox.OnMouseKeyDown ( event );
        return;
    }

    UIDialogBox::OnMouseKeyDown ( event );
}

void UIProps::OnMouseKeyUp ( MouseKeyEvent const &event ) noexcept
{
    if ( _closeButton.IsOverlapped ( event._x, event._y ) )
    {
        _closeButton.OnMouseKeyUp ( event );
        return;
    }

    if ( _checkbox.IsOverlapped ( event._x, event._y ) )
    {
        _checkbox.OnMouseKeyUp ( event );
        return;
    }

    if ( _combobox.IsOverlapped ( event._x, event._y ) )
    {
        _combobox.OnMouseKeyUp ( event );
        return;
    }

    UIDialogBox::OnMouseKeyUp ( event );
}

void UIProps::OnMouseMove ( MouseMoveEvent const &event ) noexcept
{
    if ( _dragState )
    {
        UIDialogBox::OnMouseMove ( event );
        return;
    }

    if ( _closeButton.IsOverlapped ( event._x, event._y ) )
    {
        _closeButton.OnMouseMove ( event );
        return;
    }

    if ( _checkbox.IsOverlapped ( event._x, event._y ) )
    {
        _checkbox.OnMouseMove ( event );
        return;
    }

    if ( _combobox.IsOverlapped ( event._x, event._y ) )
    {
        _combobox.OnMouseMove ( event );
        return;
    }

    UIDialogBox::OnMouseMove ( event );
}

void UIProps::Submit ( pbr::UIElement::SubmitInfo &info ) noexcept
{
    UIDialogBox::Submit ( info );
    _closeButton.UpdatedRect ();
    _checkbox.UpdatedRect ();
    _combobox.UpdatedRect ();
}

void UIProps::OnCheckBox ( UICheckbox::eState /*state*/ ) noexcept
{
    // FUCK - need to implement
}

void UIProps::OnClose () noexcept
{
    // FUCK - need to implement
}

void UIProps::OnCombobox ( uint32_t /*value*/ ) noexcept
{
    // FUCK - need to implement
}

} // namespace editor
