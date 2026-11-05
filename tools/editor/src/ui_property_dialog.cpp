#include <precompiled_headers.hpp>
#include <glyphs.hpp>
#include <message_queue.hpp>
#include <theme.hpp>
#include <ui_property_dialog.hpp>


namespace editor {

UIPropertyDialog::UIPropertyDialog ( ActionHandler &&onClose ) noexcept:
    UIDialogBox ( "Properties" ),

    _headerLine ( _div,

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
            ._marginBottom = pbr::LengthValue ( pbr::LengthValue::eType::PX, 2.0F ),
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
            ._width = theme::WHOLE_LENGTH,
            ._height = theme::HEADER_HEIGHT
        },

        "Header line"
    ),

    _headerText ( _headerLine, "Properties", "Header"),
    _closeButton ( _headerLine, "Close button" ),
    _onClose ( std::move ( onClose ) )
{
    pbr::CSSComputedValues &headerTextStyle = _headerText.GetCSS ();
    headerTextStyle._fontSize = theme::HEADER_FONT_SIZE;
    headerTextStyle._textAlign = pbr::TextAlignProperty::eValue::Center;
    headerTextStyle._paddingTop = theme::HEADER_VERTICAL_PADDING;
    headerTextStyle._width = theme::WHOLE_LENGTH;

    pbr::CSSComputedValues &closeButtonStyle = _closeButton.GetCSS ();
    closeButtonStyle._top = pbr::LengthValue ( pbr::LengthValue::eType::PX, 4.0F );
    closeButtonStyle._right = pbr::LengthValue ( pbr::LengthValue::eType::PX, 4.0F );

    _closeButton.Connect ( std::bind_front ( &UIPropertyDialog::Close, this ) );
    _div.PrependChildElement ( _headerLine );
}

void UIPropertyDialog::GetRect ( Rect &target ) const noexcept
{
    target.From ( _div.GetAbsoluteRect () );
}

void UIPropertyDialog::Close () noexcept
{
    MessageQueue::Instance ().EnqueueBack (
        Message ( eMessageType::UIRemoveWidget,
            [ this ] () noexcept {
                _onClose ();
                return this;
            }
        )
    );
}

DIVUIElement &UIPropertyDialog::GetBody () noexcept
{
    return _div;
}

void UIPropertyDialog::Append ( std::unique_ptr<UIEditBox> control ) noexcept
{
    _editBoxes.push_back ( std::move ( control ) );
}

void UIPropertyDialog::AddSeparator () noexcept
{
    _separators.push_back ( std::make_unique<UISeparator> ( _div, "property: separator" ) );
}

void UIPropertyDialog::AddComponentHeader ( std::string const &name, std::string_view glyph ) noexcept
{
    std::unique_ptr<DIVUIElement> rootElement = std::make_unique<DIVUIElement> ( _div,
        pbr::CSSComputedValues
        {
            ._backgroundColor = theme::TRANSPARENT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::ZERO_LENGTH,
            ._left = theme::ZERO_LENGTH,
            ._right = theme::ZERO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = theme::MAIN_COLOR,
            ._display = pbr::DisplayProperty::eValue::Block,
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
            ._width = theme::WHOLE_LENGTH,
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::PX, 21.0F )
        },

        name + " (component header root)"
    );

    DIVUIElement &root = *rootElement;
    _divs.push_back ( std::move ( rootElement ) );
    _div.AppendChildElement ( root );

    std::unique_ptr<DIVUIElement> infoElement = std::make_unique<DIVUIElement> ( root,
        pbr::CSSComputedValues
        {
            ._backgroundColor = theme::TRANSPARENT_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::ZERO_LENGTH,
            ._left = theme::ZERO_LENGTH,
            ._right = theme::ZERO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = theme::MAIN_COLOR,
            ._display = pbr::DisplayProperty::eValue::Block,
            ._fontFile { theme::NORMAL_FONT_FAMILY.data (), theme::NORMAL_FONT_FAMILY.size () },
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
            ._position = pbr::PositionProperty::eValue::Relative,
            ._textAlign = pbr::TextAlignProperty::eValue::Left,
            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
            ._width = theme::WHOLE_LENGTH,
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::PX, 18.0F )
        },

        name + " (component header info)"
    );

    DIVUIElement &info = *infoElement;
    _divs.push_back ( std::move ( infoElement ) );
    root.AppendChildElement ( info );

    _labels.push_back (
        std::make_unique<UILabel> ( info,
            pbr::CSSComputedValues
            {
                ._backgroundColor = theme::TRANSPARENT_COLOR,
                ._backgroundSize = theme::ZERO_LENGTH,
                ._bottom = theme::AUTO_LENGTH,
                ._left = pbr::LengthValue ( pbr::LengthValue::eType::PX, 4.0F ),
                ._right = theme::AUTO_LENGTH,
                ._top = theme::ZERO_LENGTH,
                ._color = theme::MAIN_COLOR,
                ._display = pbr::DisplayProperty::eValue::Block,
                ._fontFile { glyph::FONT_FAMILY },
                ._fontSize = pbr::LengthValue ( pbr::LengthValue::eType::PX, 15.0F ),
                ._lineHeight = theme::AUTO_LENGTH,
                ._marginBottom = theme::ZERO_LENGTH,
                ._marginLeft = theme::ZERO_LENGTH,
                ._marginRight = theme::ZERO_LENGTH,
                ._marginTop = theme::ZERO_LENGTH,
                ._paddingBottom = theme::ZERO_LENGTH,
                ._paddingLeft = theme::ZERO_LENGTH,
                ._paddingRight = theme::ZERO_LENGTH,
                ._paddingTop = pbr::LengthValue ( pbr::LengthValue::eType::PX, 1.0F ),
                ._position = pbr::PositionProperty::eValue::Absolute,
                ._textAlign = pbr::TextAlignProperty::eValue::Center,
                ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
                ._width = pbr::LengthValue ( pbr::LengthValue::eType::PX, 20.0F ),
                ._height = pbr::LengthValue ( pbr::LengthValue::eType::PX, 20.0F )
            },

            glyph,
            name + " (component header icon)"
        )
    );

    _labels.push_back (
        std::make_unique<UILabel> ( info,
            pbr::CSSComputedValues
            {
                ._backgroundColor = theme::TRANSPARENT_COLOR,
                ._backgroundSize = theme::ZERO_LENGTH,
                ._bottom = theme::ZERO_LENGTH,
                ._left = theme::ZERO_LENGTH,
                ._right = theme::ZERO_LENGTH,
                ._top = theme::ZERO_LENGTH,
                ._color = theme::MAIN_COLOR,
                ._display = pbr::DisplayProperty::eValue::Block,
                ._fontFile { theme::NORMAL_FONT_FAMILY },
                ._fontSize = theme::NORMAL_FONT_SIZE,
                ._lineHeight = theme::AUTO_LENGTH,
                ._marginBottom = theme::ZERO_LENGTH,
                ._marginLeft = theme::ZERO_LENGTH,
                ._marginRight = theme::ZERO_LENGTH,
                ._marginTop = theme::ZERO_LENGTH,
                ._paddingBottom = theme::ZERO_LENGTH,
                ._paddingLeft = pbr::LengthValue ( pbr::LengthValue::eType::PX, 25.0F ),
                ._paddingRight = theme::ZERO_LENGTH,
                ._paddingTop = theme::ZERO_LENGTH,
                ._position = pbr::PositionProperty::eValue::Static,
                ._textAlign = pbr::TextAlignProperty::eValue::Left,
                ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
                ._width = theme::WHOLE_LENGTH,
                ._height = pbr::LengthValue ( pbr::LengthValue::eType::PX, 10.0F )
            },

            name,
            name + " (component header name)"
        )
    );

    //_labels.push_back (
    //    std::make_unique<UILabel> ( info,
    //        pbr::CSSComputedValues
    //        {
    //            ._backgroundColor = theme::TRANSPARENT_COLOR,
    //            ._backgroundSize = theme::ZERO_LENGTH,
    //            ._bottom = theme::AUTO_LENGTH,
    //            ._left = theme::AUTO_LENGTH,
    //            ._right = pbr::LengthValue ( pbr::LengthValue::eType::PX, 5.0F ),
    //            ._top = pbr::LengthValue ( pbr::LengthValue::eType::PX, 1.0F ),
    //            ._color = theme::MAIN_COLOR,
    //            ._display = pbr::DisplayProperty::eValue::Block,
    //            ._fontFile { glyph::FONT_FAMILY },
    //            ._fontSize = pbr::LengthValue ( pbr::LengthValue::eType::PX, 11.0F ),
    //            ._lineHeight = theme::AUTO_LENGTH,
    //            ._marginBottom = theme::ZERO_LENGTH,
    //            ._marginLeft = theme::ZERO_LENGTH,
    //            ._marginRight = theme::ZERO_LENGTH,
    //            ._marginTop = theme::ZERO_LENGTH,
    //            ._paddingBottom = theme::ZERO_LENGTH,
    //            ._paddingLeft = theme::ZERO_LENGTH,
    //            ._paddingRight = pbr::LengthValue ( pbr::LengthValue::eType::PX, 3.0F ),
    //            ._paddingTop = theme::ZERO_LENGTH,
    //            ._position = pbr::PositionProperty::eValue::Absolute,
    //            ._textAlign = pbr::TextAlignProperty::eValue::Right,
    //            ._verticalAlign = pbr::VerticalAlignProperty::eValue::Top,
    //            ._width = pbr::LengthValue ( pbr::LengthValue::eType::PX, 20.0F ),
    //            ._height = pbr::LengthValue ( pbr::LengthValue::eType::PX, 14.0F )
    //        },

    //        glyph,
    //        name + " (component header collapse)"
    //    )
    //);

    std::unique_ptr<DIVUIElement> underlineElement = std::make_unique<DIVUIElement> ( root,
        pbr::CSSComputedValues
        {
            ._backgroundColor = theme::MAIN_COLOR,
            ._backgroundSize = theme::ZERO_LENGTH,
            ._bottom = theme::ZERO_LENGTH,
            ._left = theme::ZERO_LENGTH,
            ._right = theme::ZERO_LENGTH,
            ._top = theme::ZERO_LENGTH,
            ._color = pbr::ColorValue ( 255U, 255U, 255U, 255U ),
            ._display = pbr::DisplayProperty::eValue::Block,
            ._fontFile{ theme::NORMAL_FONT_FAMILY },
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
            ._width = theme::WHOLE_LENGTH,
            ._height = pbr::LengthValue ( pbr::LengthValue::eType::PX, 3.0F )
        },

        name + " (component header underline)"
    );

    DIVUIElement &underline = *underlineElement;
    _divs.push_back ( std::move ( underlineElement ) );
    root.AppendChildElement ( underline );
}

bool UIPropertyDialog::HasChild ( Widget const &child ) const noexcept
{
    if ( this == &child || _closeButton.HasChild ( child ) )
        return true;

    for ( auto &control : _editBoxes )
    {
        if ( control.get () == &child )
        {
            return true;
        }
    }

    return false;
}

void UIPropertyDialog::OnMouseButtonDown ( MouseButtonEvent const &event ) noexcept
{
    if ( _closeButton.IsOverlapped ( event._x, event._y ) )
    {
        _closeButton.OnMouseButtonDown ( event );
        return;
    }

    for ( auto &control : _editBoxes )
    {
        if ( control->IsOverlapped ( event._x, event._y ) )
        {
            control->OnMouseButtonDown ( event );
            return;
        }
    }

    UIDialogBox::OnMouseButtonDown ( event );
}

void UIPropertyDialog::OnMouseButtonUp ( MouseButtonEvent const &event ) noexcept
{
    if ( _closeButton.IsOverlapped ( event._x, event._y ) )
    {
        _closeButton.OnMouseButtonUp ( event );
        return;
    }

    for ( auto &control : _editBoxes )
    {
        if ( control->IsOverlapped ( event._x, event._y ) )
        {
            control->OnMouseButtonUp ( event );
            return;
        }
    }

    UIDialogBox::OnMouseButtonUp ( event );
}

void UIPropertyDialog::OnMouseMove ( MouseMoveEvent const &event ) noexcept
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

    for ( auto &control : _editBoxes )
    {
        if ( control->IsOverlapped ( event._x, event._y ) )
        {
            control->OnMouseMove ( event );
            return;
        }
    }

    UIDialogBox::OnMouseMove ( event );
}

void UIPropertyDialog::DumpLayout ( pbr::SVGWriter &writer ) const noexcept
{
    _div.DumpLayout ( writer );
    _headerLine.DumpLayout ( writer );

    _headerText.DumpLayout ( writer );
    _closeButton.DumpLayout ( writer );

    for ( auto const &control : _editBoxes )
        control->DumpLayout ( writer );

    for ( auto const &div : _divs )
        div->DumpLayout ( writer );

    for ( auto const &label : _labels )
        label->DumpLayout ( writer );

    for ( auto const& separator : _separators )
    {
        separator->DumpLayout ( writer );
    }
}

void UIPropertyDialog::Submit ( pbr::UIElement::SubmitInfo &info ) noexcept
{
    UIDialogBox::Submit ( info );
    _closeButton.UpdatedRect ();
}

} // namespace editor
