#include <precompiled_headers.hpp>
#include <theme.hpp>
#include <ui_props.hpp>


namespace editor {

UIProps::UIProps ( MessageQueue &messageQueue ) noexcept:
    UIDialogBox ( messageQueue, "Properties" ),

    _headerLine (
        std::make_unique<pbr::DIVUIElement> ( &_div,

            pbr::CSSComputedValues
            {
                ._backgroundColor = theme::HEADER_COLOR,
                ._backgroundSize = theme::ZERO_LENGTH,
                ._bottom = theme::ZERO_LENGTH,
                ._left = theme::ZERO_LENGTH,
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
                ._position = pbr::PositionProperty::eValue::Static,
                ._textAlign = pbr::TextAlignProperty::eValue::Left,
                ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
                ._width =  pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F ),
                ._height = theme::HEADER_HEIGHT
            },

            "Header line"
        )
    ),

    _headerText ( std::make_unique<UILabel> ( messageQueue, *_headerLine, "Properties", "Header" ) ),
    _closeButton ( std::make_unique<UICloseButton> ( messageQueue, *_headerLine, "Close button" ) )
{
    pbr::CSSComputedValues &headerTextStyle = _headerText->GetCSS ();
    headerTextStyle._fontSize = theme::HEADER_FONT_SIZE;
    headerTextStyle._textAlign = pbr::TextAlignProperty::eValue::Center;
    headerTextStyle._paddingTop = theme::HEADER_VERTICAL_PADDING;
    headerTextStyle._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F );

    pbr::CSSComputedValues &closeButtonStyle = _closeButton->GetCSS ();
    closeButtonStyle._top = pbr::LengthValue ( pbr::LengthValue::eType::PX, 2.0F );
    closeButtonStyle._right = pbr::LengthValue ( pbr::LengthValue::eType::PX, 4.0F );

    _div.AppendChildElement ( *_headerLine );
}

void UIProps::OnMouseMove ( MouseMoveEvent const &event ) noexcept
{
    if ( _dragState )
    {
        UIDialogBox::OnMouseMove ( event );
        return;
    }

    UICloseButton &closeButton = *_closeButton;

    if ( closeButton.IsOverlapped ( event._x, event._y ) )
    {
        closeButton.OnMouseMove ( event );
        return;
    }

    UIDialogBox::OnMouseMove ( event );
}

void UIProps::Submit ( pbr::UIElement::SubmitInfo &info ) noexcept
{
    UIDialogBox::Submit ( info );
    _closeButton->UpdatedRect ();
}

} // namespace editor
