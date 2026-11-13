#include <precompiled_headers.hpp>
#include <glyphs.hpp>
#include <message_queue.hpp>
#include <theme.hpp>
#include <ui_property_dialog.hpp>


namespace editor {

UIPropertyDialog::HeaderInfo::HeaderInfo ( HeaderInfo &&other ) noexcept:
    _label ( std::move ( other._label ) ),
    _property ( std::exchange ( other._property, nullptr ) ),
    _connection ( std::exchange ( other._connection, 42UZ ) )
{
    // NOTHING
}

UIPropertyDialog::HeaderInfo& UIPropertyDialog::HeaderInfo::operator = ( UIPropertyDialog::HeaderInfo &&other ) noexcept
{
    if ( this == &other ) [[unlikely]]
        return *this;

    _property = std::exchange ( other._property, nullptr );
    _label = std::move ( other._label );
    _connection = std::exchange ( other._connection, 42UZ );
    return *this;
}

UIPropertyDialog::HeaderInfo::HeaderInfo ( DIVUIElement &root, StringProperty &name ) noexcept:
    _label (
        std::make_unique<UILabel> ( root,
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
            name + " (component header componentName)"
        )
    ),

    _property ( &name )
{
    _connection = name.Connect (
        [ &label = *_label.get () ]() noexcept {
            // FUCK
            android_vulkan::LogDebug ( "%p", &label );
        }
    );
}

UIPropertyDialog::HeaderInfo::~HeaderInfo () noexcept
{
    if ( _property )
    {
        _property->Disconnect ( _connection );
    }
}

void UIPropertyDialog::HeaderInfo::Disconnect () noexcept
{
    std::exchange ( _property, nullptr )->Disconnect ( _connection );
}

void UIPropertyDialog::HeaderInfo::DumpLayout ( pbr::SVGWriter &writer ) const noexcept
{
    _label->DumpLayout ( writer );
}

//----------------------------------------------------------------------------------------------------------------------

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

void UIPropertyDialog::Disconnect () noexcept
{
    for ( auto &header : _headers )
    {
        header.Disconnect ();
    }
}

DIVUIElement &UIPropertyDialog::GetBody () noexcept
{
    return _div;
}

void UIPropertyDialog::Append ( std::string_view name, StringProperty &property ) noexcept
{
    _editBoxes.push_back (
        std::make_unique<UIEditBox> (
            _div,
            theme::PROPERTY_FIRST_COLUMN_WIDTH,
            name,
            property,
            "property: " + property
        )
    );
}

void UIPropertyDialog::Append ( StringProperty &componentName,
    BoolProperty &componentCollapse,
    std::string_view componentGlyph
) noexcept
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

        componentName + " (component header root)"
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

        componentName + " (component header info)"
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

            componentGlyph,
            componentName + " (component header icon)"
        )
    );

    HeaderInfo headerInfo ( info, componentName );
    UILabel &label = *headerInfo._label;
    _headers.push_back ( std::move ( headerInfo ) );

    _labels.push_back (
        std::make_unique<UILabel> ( info,
            pbr::CSSComputedValues
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

            componentCollapse ? glyph::COMBOBOX_UP : glyph::COMBOBOX_DOWN,
            componentName + " (component header collapse)"
        )
    );

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

        componentName + " (component header underline)"
    );

    DIVUIElement &underline = *underlineElement;
    _divs.push_back ( std::move ( underlineElement ) );
    root.AppendChildElement ( underline );

    if ( componentCollapse )
        return;

    std::unique_ptr<UIEditBox> control = std::make_unique<UIEditBox> (
        _div,
        theme::PROPERTY_FIRST_COLUMN_WIDTH,
        "Name",
        componentName,
        "property componentName: " + componentName
    );

    UIEditBox &c = *control;

    c.ConnectOnCancel (
        [ &label ] ( std::string const &value ) noexcept {
            label.SetText ( value );
        }
    );

    c.ConnectOnChange (
        [ &label ] ( std::string const &value ) noexcept {
            label.SetText ( value );
        }
    );

    c.ConnectOnNewValue (
        [ &label ] ( std::string const &value ) noexcept {
            // FUCK - issue history, remove label update here
            label.SetText ( value );
        }
    );

    _editBoxes.push_back ( std::move ( control ) );
}

void UIPropertyDialog::AppendSeparator () noexcept
{
    _separators.push_back ( std::make_unique<UISeparator> ( _div, "property: separator" ) );
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

    for ( auto const &header : _headers )
        header.DumpLayout ( writer );

    for ( auto const& separator : _separators )
    {
        separator->DumpLayout ( writer );
    }
}

void UIPropertyDialog::Submit ( pbr::UIElement::SubmitInfo &info ) noexcept
{
    UIDialogBox::Submit ( info );

    for ( auto &control : _editBoxes )
        control->UpdatedRect ();

    _closeButton.UpdatedRect ();
}

} // namespace editor
