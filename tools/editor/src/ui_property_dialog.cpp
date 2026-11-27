#include <precompiled_headers.hpp>
#include <history.hpp>
#include <message_queue.hpp>
#include <theme.hpp>
#include <ui_property_dialog.hpp>


namespace editor {

namespace {

class SetStringPropertyAction final : public Action
{
    private:
        StringProperty*     _property = nullptr;
        std::string         _before {};
        std::string         _after {};

    public:
        SetStringPropertyAction () = delete;

        SetStringPropertyAction ( SetStringPropertyAction const & ) = delete;
        SetStringPropertyAction &operator = ( SetStringPropertyAction const & ) = delete;

        SetStringPropertyAction ( SetStringPropertyAction && ) = default;
        SetStringPropertyAction &operator = ( SetStringPropertyAction && ) = default;

        explicit SetStringPropertyAction ( StringProperty &property, std::string const &now ) noexcept;

        ~SetStringPropertyAction () override = default;

    private:
        void Redo () noexcept override;
        void Undo () noexcept override;
};

SetStringPropertyAction::SetStringPropertyAction ( StringProperty &property, std::string const &now ) noexcept:
    _property ( &property ),
    _before ( property ),
    _after ( now )
{
    // NOTHING
}

void SetStringPropertyAction::Redo () noexcept
{
    *_property = _after;
}

void SetStringPropertyAction::Undo () noexcept
{
    *_property = _before;
}

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

UIPropertyDialog::EditBoxInfo::EditBoxInfo ( EditBoxInfo &&other ) noexcept:
    _editBox ( std::move ( other._editBox ) ),
    _property ( std::exchange ( other._property, nullptr ) ),
    _connection ( std::exchange ( other._connection, 42U ) )
{
    // NOTHING
}

UIPropertyDialog::EditBoxInfo &UIPropertyDialog::EditBoxInfo::operator = ( UIPropertyDialog::EditBoxInfo &&other ) noexcept
{
    if ( this == &other ) [[unlikely]]
        return *this;

    _property = std::exchange ( other._property, nullptr );
    _editBox = std::move ( other._editBox );
    _connection = std::exchange ( other._connection, 42U );
    return *this;
}

UIPropertyDialog::EditBoxInfo::EditBoxInfo ( DIVUIElement &root,
    std::string_view caption,
    StringProperty &property
) noexcept:
    _editBox (
        std::make_unique<UIEditBox> ( root,
            theme::PROPERTY_FIRST_COLUMN_WIDTH,
            caption,
            property,
            "property " + std::string ( caption ) + ": " + property
        )
    ),

    _property ( &property )
{
    UIEditBox &editBox = *_editBox;

    editBox.ConnectOnNewValue (
        [ &property ] ( std::string const &value ) noexcept {
            History &history = History::Instance ();
            history.Begin ();
            history.Append ( std::make_unique<SetStringPropertyAction> ( property, value ) );
            history.End ();
        }
    );

    _connection = property.Connect (
        [ &editBox, &property ] () noexcept {
            editBox.SetText ( property );
        }
    );
}

UIPropertyDialog::EditBoxInfo::~EditBoxInfo () noexcept
{
    if ( _property )
    {
        _property->Disconnect ( _connection );
    }
}

void UIPropertyDialog::EditBoxInfo::Disconnect () noexcept
{
    std::exchange ( _property, nullptr )->Disconnect ( _connection );
}

void UIPropertyDialog::EditBoxInfo::DumpLayout ( pbr::SVGWriter &writer ) const noexcept
{
    _editBox->DumpLayout ( writer );
}

//----------------------------------------------------------------------------------------------------------------------

UIPropertyDialog::UIPropertyDialog ( UICloseButton::Callback &&onClose ) noexcept:
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
    _closeButton ( _headerLine, "Close button" )
{
    pbr::CSSComputedValues &headerTextStyle = _headerText.GetCSS ();
    headerTextStyle._fontSize = theme::HEADER_FONT_SIZE;
    headerTextStyle._textAlign = pbr::TextAlignProperty::eValue::Center;
    headerTextStyle._paddingTop = theme::HEADER_VERTICAL_PADDING;
    headerTextStyle._width = theme::WHOLE_LENGTH;

    pbr::CSSComputedValues &closeButtonStyle = _closeButton.GetCSS ();
    closeButtonStyle._top = pbr::LengthValue ( pbr::LengthValue::eType::PX, 4.0F );
    closeButtonStyle._right = pbr::LengthValue ( pbr::LengthValue::eType::PX, 4.0F );

    _closeButton.Connect ( std::move ( onClose ) );
    _div.PrependChildElement ( _headerLine );
}

void UIPropertyDialog::GetRect ( Rect &target ) const noexcept
{
    target.From ( _div.GetAbsoluteRect () );
}

void UIPropertyDialog::Disconnect () noexcept
{
    for ( auto &editBox : _editBoxes )
        editBox.Disconnect ();

    for ( auto const &header : _headers )
    {
        header->Disconnect ();
    }
}

DIVUIElement &UIPropertyDialog::GetBody () noexcept
{
    return _div;
}

void UIPropertyDialog::Append ( std::string_view name, StringProperty &property ) noexcept
{
    _editBoxes.push_back ( EditBoxInfo ( _div, name, property ) );
}

void UIPropertyDialog::Append ( StringProperty &componentName,
    bool &componentCollapse,
    bool &componentEdit,
    std::string_view componentGlyph
) noexcept
{
    std::unique_ptr<UIComponentHeader> header = std::make_unique<UIComponentHeader> ( _div,
        componentName,
        componentGlyph,
        componentCollapse,
        componentEdit
    );

    UIComponentHeader &h = *header;
    _headers.push_back ( std::move ( header ) );

    if ( componentCollapse )
        return;

    EditBoxInfo control ( _div, "Name", componentName );
    UIEditBox &c = *control._editBox;
    _editBoxes.push_back ( std::move ( control ) );

    c.ConnectOnCancel (
        [ &h ] ( std::string const &value ) noexcept {
            h.SetName ( value );
        }
    );

    c.ConnectOnChange (
        [ &h ] ( std::string const &value ) noexcept {
            h.SetName ( value );
        }
    );
}

void UIPropertyDialog::AppendSeparator () noexcept
{
    _separators.push_back ( std::make_unique<UISeparator> ( _div, "property: separator" ) );
}

bool UIPropertyDialog::HasChild ( Widget const &child ) const noexcept
{
    if ( this == &child || _closeButton.HasChild ( child ) )
        return true;

    for ( auto const &info : _editBoxes )
    {
        if ( info._editBox.get () == &child )
        {
            return true;
        }
    }

    for ( auto const &header : _headers )
    {
        if ( header.get () == &child )
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

    for ( auto const &editBox : _editBoxes )
    {
        if ( UIEditBox &control = *editBox._editBox; control.IsOverlapped ( event._x, event._y ) )
        {
            control.OnMouseButtonDown ( event );
            return;
        }
    }

    for ( auto const &header : _headers )
    {
        if ( UIComponentHeader &control = *header; control.IsOverlapped ( event._x, event._y ) )
        {
            control.OnMouseButtonDown ( event );
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

    for ( auto const &editBox : _editBoxes )
    {
        if ( UIEditBox &control = *editBox._editBox; control.IsOverlapped ( event._x, event._y ) )
        {
            control.OnMouseButtonUp ( event );
            return;
        }
    }

    for ( auto const &header : _headers )
    {
        if ( UIComponentHeader &control = *header; control.IsOverlapped ( event._x, event._y ) )
        {
            control.OnMouseButtonUp ( event );
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

    for ( auto const &editBox : _editBoxes )
    {
        if ( UIEditBox &control = *editBox._editBox; control.IsOverlapped ( event._x, event._y ) )
        {
            control.OnMouseMove ( event );
            return;
        }
    }

    for ( auto const &header : _headers )
    {
        if ( UIComponentHeader &control = *header; control.IsOverlapped ( event._x, event._y ) )
        {
            control.OnMouseMove ( event );
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

    for ( auto const &editBox : _editBoxes )
        editBox.DumpLayout ( writer );

    for ( auto const &header : _headers )
        header->DumpLayout ( writer );

    for ( auto const &separator : _separators )
    {
        separator->DumpLayout ( writer );
    }
}

void UIPropertyDialog::Submit ( pbr::UIElement::SubmitInfo &info ) noexcept
{
    UIDialogBox::Submit ( info );

    for ( auto const &editBox : _editBoxes )
        editBox._editBox->UpdatedRect ();

    for ( auto const &header : _headers )
        header->UpdatedRect ();

    _closeButton.UpdatedRect ();

    if ( std::exchange ( _firstSubmit, false ) ) [[unlikely]]
    {
        MessageQueue::Instance ().EnqueueBack ( Message ( eMessageType::TriggerMouseMove ) );
    }
}

} // namespace editor
