#include <precompiled_headers.hpp>
#include <message_queue.hpp>
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

    MessageQueue::Instance ().EnqueueBack (
        Message ( eMessageType::MouseHover,
            [ value = this ] () noexcept {
                return value;
            }
        )
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

void Widget::CaptureMouse () noexcept
{
    MessageQueue &messageQueue = MessageQueue::Instance ();

    messageQueue.EnqueueBack (
        Message ( eMessageType::StartWidgetCaptureMouse,
            [ value = this ] () noexcept {
                return value;
            }
        )
    );

    messageQueue.EnqueueBack ( Message ( eMessageType::CaptureMouse ) );
}

void Widget::ReleaseMouse () noexcept
{
    MessageQueue &messageQueue = MessageQueue::Instance ();
    messageQueue.EnqueueBack ( Message ( eMessageType::StopWidgetCaptureMouse ) );
    messageQueue.EnqueueBack ( Message ( eMessageType::ReleaseMouse ) );
}

void Widget::ChangeCursor ( eCursor cursor ) noexcept
{
    MessageQueue::Instance ().EnqueueBack (
        Message ( eMessageType::ChangeCursor,
            [ value = std::bit_cast<void*> ( cursor ) ] () noexcept {
                return value;
            }
        )
    );
}

void Widget::KillFocus () noexcept
{
    MessageQueue &messageQueue = MessageQueue::Instance ();
    messageQueue.EnqueueBack ( Message ( eMessageType::KillFocus ) );
    messageQueue.EnqueueBack ( Message ( eMessageType::ReleaseKeyboard ) );
}

void Widget::SetFocus () noexcept
{
    MessageQueue &messageQueue = MessageQueue::Instance ();

    messageQueue.EnqueueBack (
        Message ( eMessageType::SetFocus,
            [ value = this ] () noexcept {
                return value;
            }
        )
    );

    messageQueue.EnqueueBack ( Message ( eMessageType::CaptureKeyboard ) );
}

} // namespace editor
