#include <precompiled_headers.hpp>
#include <widget.hpp>


namespace editor {

void Widget::ApplyClipboard ( std::u32string const &/*text*/ ) noexcept
{
    // NOTHING
}

void Widget::OnDoubleClick ( MouseButtonEvent const &event ) noexcept
{
    OnMouseButtonDown ( event );
}

void Widget::OnKeyboardKeyDown ( eKey /*key*/, KeyModifier /*modifier*/ ) noexcept
{
    // NOTHING
}

void Widget::OnKeyboardKeyUp ( eKey /*key*/, KeyModifier /*modifier*/ ) noexcept
{
    // NOTHING
}

void Widget::OnMouseLeave () noexcept
{
    // NOTHING
}

void Widget::OnMouseButtonDown ( MouseButtonEvent const &/*event*/ ) noexcept
{
    // NOTHING
}

void Widget::OnMouseButtonUp ( MouseButtonEvent const &/*event*/ ) noexcept
{
    // NOTHING
}

void Widget::OnMouseMove ( MouseMoveEvent const &event ) noexcept
{
    if ( event._eventID - std::exchange ( _hoverEventID, event._eventID ) < 2U ) [[likely]]
        return;

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::MouseHover,
            ._params = this,
            ._serialNumber = 0U
        }
    );
}

void Widget::OnTyping ( char32_t /*character*/ ) noexcept
{
    // NOTHING
}

Widget::LayoutStatus Widget::ApplyLayout ( android_vulkan::Renderer &/*renderer*/,
    pbr::FontStorage &/*fontStorage*/
) noexcept
{
    return {};
}

void Widget::Submit ( pbr::UIElement::SubmitInfo &/*info*/ ) noexcept
{
    // NOTHING
}

bool Widget::UpdateCache ( pbr::FontStorage &/*fontStorage*/, VkExtent2D const &/*viewport*/ ) noexcept
{
    return false;
}

void Widget::UpdatedRect () noexcept
{
    // NOTHING
}

bool Widget::IsOverlapped ( int32_t x, int32_t y ) const noexcept
{
    return _rect.IsOverlapped ( x, y );
}

Widget::Widget ( MessageQueue &messageQueue ) noexcept:
    _messageQueue ( messageQueue )
{
    // NOTHING
}

void Widget::CaptureMouse () noexcept
{
    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::StartWidgetCaptureMouse,
            ._params = this,
            ._serialNumber = 0U
        }
    );

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::CaptureMouse,
            ._params = nullptr,
            ._serialNumber = 0U
        }
    );
}

void Widget::ReleaseMouse () noexcept
{
    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::StopWidgetCaptureMouse,
            ._params = nullptr,
            ._serialNumber = 0U
        }
    );

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::ReleaseMouse,
            ._params = nullptr,
            ._serialNumber = 0U
        }
    );
}

void Widget::ChangeCursor ( eCursor cursor ) noexcept
{
    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::ChangeCursor,
            ._params = std::bit_cast<void*> ( cursor ),
            ._serialNumber = 0U
        }
    );
}

void Widget::KillFocus () noexcept
{
    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::KillFocus,
            ._params = nullptr,
            ._serialNumber = 0U
        }
    );

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::ReleaseKeyboard,
            ._params = nullptr,
            ._serialNumber = 0U
        }
    );
}

void Widget::SetFocus () noexcept
{
    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::SetFocus,
            ._params = this,
            ._serialNumber = 0U
        }
    );

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::CaptureKeyboard,
            ._params = nullptr,
            ._serialNumber = 0U
        }
    );
}

} // namespace editor
