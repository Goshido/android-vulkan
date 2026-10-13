#include <precompiled_headers.hpp>
#include <message_queue.hpp>
#include <ui_transform.hpp>


namespace editor {

UITransform::UITransform ( ActionHandler &&onClose,
    ActionHandler &&onCancelX,
    UIEditBox::ValueHandler &&onChangeX,
    UIEditBox::ValueHandler &&onNewX,
    UIEditBox::ValueHandler &&onNextX,

    ActionHandler &&onCancelY,
    UIEditBox::ValueHandler &&onChangeY,
    UIEditBox::ValueHandler &&onNewY,
    UIEditBox::ValueHandler &&onNextY,

    ActionHandler &&onCancelZ,
    UIEditBox::ValueHandler &&onChangeZ,
    UIEditBox::ValueHandler &&onNewZ,
    UIEditBox::ValueHandler &&onNextZ,

    ActionHandler &&onCancelScaleX,
    UIEditBox::ValueHandler &&onChangeScaleX,
    UIEditBox::ValueHandler &&onNewScaleX,
    UIEditBox::ValueHandler &&onNextScaleX,

    ActionHandler &&onCancelScaleY,
    UIEditBox::ValueHandler &&onChangeScaleY,
    UIEditBox::ValueHandler &&onNewScaleY,
    UIEditBox::ValueHandler &&onNextScaleY,

    ActionHandler &&onCancelScaleZ,
    UIEditBox::ValueHandler &&onChangeScaleZ,
    UIEditBox::ValueHandler &&onNewScaleZ,
    UIEditBox::ValueHandler &&onNextScaleZ
) noexcept:
    UIDialogBox ( "Transform" ),

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
            ._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F ),
            ._height = theme::HEADER_HEIGHT
        },

        "Header line"
    ),

    _headerText ( _headerLine, "Transform", "Header"),
    _closeButton ( _headerLine, "Close button" ),

    _x ( _div, "X", "0", "EditBox[x]" ),
    _y ( _div, "Y", "0", "EditBox[y]" ),
    _z ( _div, "Z", "0", "EditBox[z]" ),

    _separator ( _div, "Separator" ),

    _scaleX ( _div, "Scale X", "0", "EditBox[scale-x]" ),
    _scaleY ( _div, "Scale Y", "0", "EditBox[scale-y]" ),
    _scaleZ ( _div, "Scale Z", "0", "EditBox[scale-z]" ),

    _onClose ( std::move ( onClose ) ),
    _onNextX ( std::move ( onNextX ) ),
    _onNextY ( std::move ( onNextY ) ),
    _onNextZ ( std::move ( onNextZ ) ),

    _onNextScaleX ( std::move ( onNextScaleX ) ),
    _onNextScaleY ( std::move ( onNextScaleY ) ),
    _onNextScaleZ ( ( std::move ( onNextScaleZ ) ) )
{
    pbr::CSSComputedValues &headerTextStyle = _headerText.GetCSS ();
    headerTextStyle._fontSize = theme::HEADER_FONT_SIZE;
    headerTextStyle._textAlign = pbr::TextAlignProperty::eValue::Center;
    headerTextStyle._paddingTop = theme::HEADER_VERTICAL_PADDING;
    headerTextStyle._width = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F );

    pbr::CSSComputedValues &closeButtonStyle = _closeButton.GetCSS ();
    closeButtonStyle._top = pbr::LengthValue ( pbr::LengthValue::eType::PX, 4.0F );
    closeButtonStyle._right = pbr::LengthValue ( pbr::LengthValue::eType::PX, 4.0F );

    _closeButton.Connect ( std::bind ( &UITransform::Close, this ) );

    _x.ConnectOnCancel (
        [ onCancel = std::move ( onCancelX ) ] ( std::string const &/*value*/ ) mutable noexcept {
            onCancel ();
        }
    );

    _x.ConnectOnChange ( std::move ( onChangeX ) );
    _x.ConnectOnNewValue ( std::move ( onNewX ) );
    _x.ConnectOnNext ( std::bind ( &UITransform::NextX, this, std::placeholders::_1 ) );

    _y.ConnectOnCancel (
        [ onCancel = std::move ( onCancelY ) ] ( std::string const &/*value*/ ) mutable noexcept {
            onCancel ();
        }
    );

    _y.ConnectOnChange ( std::move ( onChangeY ) );
    _y.ConnectOnNewValue ( std::move ( onNewY ) );
    _y.ConnectOnNext ( std::bind ( &UITransform::NextY, this, std::placeholders::_1 ) );

    _z.ConnectOnCancel (
        [ onCancel = std::move ( onCancelZ ) ] ( std::string const &/*value*/ ) mutable noexcept {
            onCancel ();
        }
    );

    _z.ConnectOnChange ( std::move ( onChangeZ ) );
    _z.ConnectOnNewValue ( std::move ( onNewZ ) );
    _z.ConnectOnNext ( std::bind ( &UITransform::NextZ, this, std::placeholders::_1 ) );

    _scaleX.ConnectOnCancel (
        [ onCancel = std::move ( onCancelScaleX ) ] ( std::string const &/*value*/ ) mutable noexcept {
            onCancel ();
        }
    );

    _scaleX.ConnectOnChange ( std::move ( onChangeScaleX ) );
    _scaleX.ConnectOnNewValue ( std::move ( onNewScaleX ) );
    _scaleX.ConnectOnNext ( std::bind ( &UITransform::NextScaleX, this, std::placeholders::_1 ) );

    _scaleY.ConnectOnCancel (
        [ onCancel = std::move ( onCancelScaleY ) ] ( std::string const &/*value*/ ) mutable noexcept {
            onCancel ();
        }
    );

    _scaleY.ConnectOnChange ( std::move ( onChangeScaleY ) );
    _scaleY.ConnectOnNewValue ( std::move ( onNewScaleY ) );
    _scaleY.ConnectOnNext ( std::bind ( &UITransform::NextScaleY, this, std::placeholders::_1 ) );

    _scaleZ.ConnectOnCancel (
        [ onCancel = std::move ( onCancelScaleZ ) ] ( std::string const &/*value*/ ) mutable noexcept {
            onCancel ();
        }
    );

    _scaleZ.ConnectOnChange ( std::move ( onChangeScaleZ ) );
    _scaleZ.ConnectOnNewValue ( std::move ( onNewScaleZ ) );
    _scaleZ.ConnectOnNext ( std::bind ( &UITransform::NextScaleZ, this, std::placeholders::_1 ) );

    _div.PrependChildElement ( _headerLine );
}

void UITransform::GetRect ( Rect &target ) const noexcept
{
    target.From ( _div.GetAbsoluteRect () );
}

void UITransform::Close () noexcept
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

void UITransform::SetX ( std::string &&value ) noexcept
{
    _x.SetText ( std::move ( value ) );
}

void UITransform::SetY ( std::string &&value ) noexcept
{
    _y.SetText ( std::move ( value ) );
}

void UITransform::SetZ ( std::string &&value ) noexcept
{
    _z.SetText ( std::move ( value ) );
}

void UITransform::SetScaleX ( std::string &&value ) noexcept
{
    _scaleX.SetText ( std::move ( value ) );
}

void UITransform::SetScaleY ( std::string &&value ) noexcept
{
    _scaleY.SetText ( std::move ( value ) );
}

void UITransform::SetScaleZ ( std::string &&value ) noexcept
{
    _scaleZ.SetText ( std::move ( value ) );
}

bool UITransform::HasChild ( Widget const &child ) const noexcept
{
    return this == &child ||
        _closeButton.HasChild ( child ) ||
        _x.HasChild ( child ) ||
        _y.HasChild ( child ) ||
        _z.HasChild ( child ) ||
        _scaleX.HasChild ( child ) ||
        _scaleY.HasChild ( child ) ||
        _scaleZ.HasChild ( child );
}

void UITransform::OnMouseButtonDown ( MouseButtonEvent const &event ) noexcept
{
    if ( _closeButton.IsOverlapped ( event._x, event._y ) )
    {
        _closeButton.OnMouseButtonDown ( event );
        return;
    }

    if ( _x.IsOverlapped ( event._x, event._y ) )
    {
        _x.OnMouseButtonDown ( event );
        return;
    }

    if ( _y.IsOverlapped ( event._x, event._y ) )
    {
        _y.OnMouseButtonDown ( event );
        return;
    }

    if ( _z.IsOverlapped ( event._x, event._y ) )
    {
        _z.OnMouseButtonDown ( event );
        return;
    }

    if ( _scaleX.IsOverlapped ( event._x, event._y ) )
    {
        _scaleX.OnMouseButtonDown ( event );
        return;
    }

    if ( _scaleY.IsOverlapped ( event._x, event._y ) )
    {
        _scaleY.OnMouseButtonDown ( event );
        return;
    }

    if ( _scaleZ.IsOverlapped ( event._x, event._y ) )
    {
        _scaleZ.OnMouseButtonDown ( event );
        return;
    }

    UIDialogBox::OnMouseButtonDown ( event );
}

void UITransform::OnMouseButtonUp ( MouseButtonEvent const &event ) noexcept
{
    if ( _closeButton.IsOverlapped ( event._x, event._y ) )
    {
        _closeButton.OnMouseButtonUp ( event );
        return;
    }

    if ( _x.IsOverlapped ( event._x, event._y ) )
    {
        _x.OnMouseButtonUp ( event );
        return;
    }

    if ( _y.IsOverlapped ( event._x, event._y ) )
    {
        _y.OnMouseButtonUp ( event );
        return;
    }

    if ( _z.IsOverlapped ( event._x, event._y ) )
    {
        _z.OnMouseButtonUp ( event );
        return;
    }

    if ( _scaleX.IsOverlapped ( event._x, event._y ) )
    {
        _scaleX.OnMouseButtonUp ( event );
        return;
    }

    if ( _scaleY.IsOverlapped ( event._x, event._y ) )
    {
        _scaleY.OnMouseButtonUp ( event );
        return;
    }

    if ( _scaleZ.IsOverlapped ( event._x, event._y ) )
    {
        _scaleZ.OnMouseButtonUp ( event );
        return;
    }

    UIDialogBox::OnMouseButtonUp ( event );
}

void UITransform::OnMouseMove ( MouseMoveEvent const &event ) noexcept
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

    if ( _x.IsOverlapped ( event._x, event._y ) )
    {
        _x.OnMouseMove ( event );
        return;
    }

    if ( _y.IsOverlapped ( event._x, event._y ) )
    {
        _y.OnMouseMove ( event );
        return;
    }

    if ( _z.IsOverlapped ( event._x, event._y ) )
    {
        _z.OnMouseMove ( event );
        return;
    }

    if ( _scaleX.IsOverlapped ( event._x, event._y ) )
    {
        _scaleX.OnMouseMove ( event );
        return;
    }

    if ( _scaleY.IsOverlapped ( event._x, event._y ) )
    {
        _scaleY.OnMouseMove ( event );
        return;
    }

    if ( _scaleZ.IsOverlapped ( event._x, event._y ) )
    {
        _scaleZ.OnMouseMove ( event );
        return;
    }

    UIDialogBox::OnMouseMove ( event );
}

void UITransform::Submit ( pbr::UIElement::SubmitInfo &info ) noexcept
{
    UIDialogBox::Submit ( info );
    _closeButton.UpdatedRect ();
    _x.UpdatedRect ();
    _y.UpdatedRect ();
    _z.UpdatedRect ();
    _scaleX.UpdatedRect ();
    _scaleY.UpdatedRect ();
    _scaleZ.UpdatedRect ();
}

void UITransform::NextX ( std::string const &value ) noexcept
{
    _y.Focus ();
    _y.SelectAll ();
    _onNextX ( value );
}

void UITransform::NextY ( std::string const &value ) noexcept
{
    _z.Focus ();
    _z.SelectAll ();
    _onNextY ( value );
}

void UITransform::NextZ ( std::string const &value ) noexcept
{
    _scaleX.Focus ();
    _scaleX.SelectAll ();
    _onNextZ ( value );
}

void UITransform::NextScaleX ( std::string const &value ) noexcept
{
    _scaleY.Focus ();
    _scaleY.SelectAll ();
    _onNextScaleX ( value );
}

void UITransform::NextScaleY ( std::string const &value ) noexcept
{
    _scaleZ.Focus ();
    _scaleZ.SelectAll ();
    _onNextScaleY ( value );
}

void UITransform::NextScaleZ ( std::string const &value ) noexcept
{
    _x.Focus ();
    _x.SelectAll ();
    _onNextScaleZ ( value );
}

} // namespace editor
