#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <message_queue.hpp>
#include <ui_transform.hpp>


namespace editor {

UITransform::UITransform ( CloseHandler &&onClose ) noexcept:
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
    _onClose ( std::move ( onClose ) )
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

    _x.ConnectOnCancel ( std::bind ( &UITransform::CancelX, this, std::placeholders::_1 ) );
    _x.ConnectOnChange ( std::bind ( &UITransform::ChangeX, this, std::placeholders::_1 ) );
    _x.ConnectOnNewValue ( std::bind ( &UITransform::NewX, this, std::placeholders::_1 ) );
    _x.ConnectOnNext ( std::bind ( &UITransform::NextX, this, std::placeholders::_1 ) );

    _y.ConnectOnCancel ( std::bind ( &UITransform::CancelY, this, std::placeholders::_1 ) );
    _y.ConnectOnChange ( std::bind ( &UITransform::ChangeY, this, std::placeholders::_1 ) );
    _y.ConnectOnNewValue ( std::bind ( &UITransform::NewY, this, std::placeholders::_1 ) );
    _y.ConnectOnNext ( std::bind ( &UITransform::NextY, this, std::placeholders::_1 ) );

    _z.ConnectOnCancel ( std::bind ( &UITransform::CancelZ, this, std::placeholders::_1 ) );
    _z.ConnectOnChange ( std::bind ( &UITransform::ChangeZ, this, std::placeholders::_1 ) );
    _z.ConnectOnNewValue ( std::bind ( &UITransform::NewZ, this, std::placeholders::_1 ) );
    _z.ConnectOnNext ( std::bind ( &UITransform::NextZ, this, std::placeholders::_1 ) );

    _scaleX.ConnectOnCancel ( std::bind ( &UITransform::CancelScaleX, this, std::placeholders::_1 ) );
    _scaleX.ConnectOnChange ( std::bind ( &UITransform::ChangeScaleX, this, std::placeholders::_1 ) );
    _scaleX.ConnectOnNewValue ( std::bind ( &UITransform::NewScaleX, this, std::placeholders::_1 ) );
    _scaleX.ConnectOnNext ( std::bind ( &UITransform::NextScaleX, this, std::placeholders::_1 ) );

    _scaleY.ConnectOnCancel ( std::bind ( &UITransform::CancelScaleY, this, std::placeholders::_1 ) );
    _scaleY.ConnectOnChange ( std::bind ( &UITransform::ChangeScaleY, this, std::placeholders::_1 ) );
    _scaleY.ConnectOnNewValue ( std::bind ( &UITransform::NewScaleY, this, std::placeholders::_1 ) );
    _scaleY.ConnectOnNext ( std::bind ( &UITransform::NextScaleY, this, std::placeholders::_1 ) );

    _scaleZ.ConnectOnCancel ( std::bind ( &UITransform::CancelScaleZ, this, std::placeholders::_1 ) );
    _scaleZ.ConnectOnChange ( std::bind ( &UITransform::ChangeScaleZ, this, std::placeholders::_1 ) );
    _scaleZ.ConnectOnNewValue ( std::bind ( &UITransform::NewScaleZ, this, std::placeholders::_1 ) );
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

void UITransform::CancelX ( std::string const &/*value*/ ) noexcept
{
    android_vulkan::LogDebug ( "CancelX" );
}

void UITransform::ChangeX ( std::string const &/*value*/ ) noexcept
{
    android_vulkan::LogDebug ( "ChangeX" );
}

void UITransform::NewX ( std::string const &/*value*/ ) noexcept
{
    android_vulkan::LogDebug ( "NewX" );
}

void UITransform::NextX ( std::string const &/*value*/ ) noexcept
{
    android_vulkan::LogDebug ( "NextX" );
    _y.Focus ();
    _y.SelectAll ();
}

void UITransform::CancelY ( std::string const &/*value*/ ) noexcept
{
    android_vulkan::LogDebug ( "CancelY" );
}

void UITransform::ChangeY ( std::string const &/*value*/ ) noexcept
{
    android_vulkan::LogDebug ( "ChangeY" );
}

void UITransform::NewY ( std::string const &/*value*/ ) noexcept
{
    android_vulkan::LogDebug ( "NewY" );
}

void UITransform::NextY ( std::string const &/*value*/ ) noexcept
{
    android_vulkan::LogDebug ( "NextY" );
    _z.Focus ();
    _z.SelectAll ();
}

void UITransform::CancelZ ( std::string const &/*value*/ ) noexcept
{
    android_vulkan::LogDebug ( "CancelZ" );
}

void UITransform::ChangeZ ( std::string const &/*value*/ ) noexcept
{
    android_vulkan::LogDebug ( "ChangeZ" );
}

void UITransform::NewZ ( std::string const &/*value*/ ) noexcept
{
    android_vulkan::LogDebug ( "NewZ" );
}

void UITransform::NextZ ( std::string const &/*value*/ ) noexcept
{
    android_vulkan::LogDebug ( "NextZ" );
    _scaleX.Focus ();
    _scaleX.SelectAll ();
}

void UITransform::CancelScaleX ( std::string const &/*value*/ ) noexcept
{
    android_vulkan::LogDebug ( "CancelScaleX" );
}

void UITransform::ChangeScaleX ( std::string const &/*value*/ ) noexcept
{
    android_vulkan::LogDebug ( "ChangeScaleX" );
}

void UITransform::NewScaleX ( std::string const &/*value*/ ) noexcept
{
    android_vulkan::LogDebug ( "NewScaleX" );
}

void UITransform::NextScaleX ( std::string const &/*value*/ ) noexcept
{
    android_vulkan::LogDebug ( "NextScaleX" );
    _scaleY.Focus ();
    _scaleY.SelectAll ();
}

void UITransform::CancelScaleY ( std::string const &/*value*/ ) noexcept
{
    android_vulkan::LogDebug ( "CancelScaleY" );
}

void UITransform::ChangeScaleY ( std::string const &/*value*/ ) noexcept
{
    android_vulkan::LogDebug ( "ChangeScaleY" );
}

void UITransform::NewScaleY ( std::string const &/*value*/ ) noexcept
{
    android_vulkan::LogDebug ( "NewScaleY" );
}

void UITransform::NextScaleY ( std::string const &/*value*/ ) noexcept
{
    android_vulkan::LogDebug ( "NextScaleY" );
    _scaleZ.Focus ();
    _scaleZ.SelectAll ();
}

void UITransform::CancelScaleZ ( std::string const &/*value*/ ) noexcept
{
    android_vulkan::LogDebug ( "CancelScaleZ" );
}

void UITransform::ChangeScaleZ ( std::string const &/*value*/ ) noexcept
{
    android_vulkan::LogDebug ( "ChangeScaleZ" );
}

void UITransform::NewScaleZ ( std::string const &/*value*/ ) noexcept
{
    android_vulkan::LogDebug ( "NewScaleZ" );
}

void UITransform::NextScaleZ ( std::string const &/*value*/ ) noexcept
{
    android_vulkan::LogDebug ( "NextScaleZ" );
    _x.Focus ();
    _x.SelectAll ();
}

} // namespace editor
