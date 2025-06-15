#include <precompiled_headers.hpp>
#include <widget.hpp>


namespace editor {

void Widget::ApplyClipboard ( std::u32string const &/*text*/ ) noexcept
{
    // NOTHING
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

void Widget::OnMouseButtonDown ( MouseKeyEvent const &/*event*/ ) noexcept
{
    // NOTHING
}

void Widget::OnMouseButtonUp ( MouseKeyEvent const &/*event*/ ) noexcept
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

} // namespace editor
