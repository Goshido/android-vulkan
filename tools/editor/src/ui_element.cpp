#include <precompiled_headers.hpp>
#include <message_queue.hpp>
#include <ui_element.hpp>


namespace editor {

UIElement::UIElement () noexcept
{
    MessageQueue::Instance ().EnqueueBack ( Message ( eMessageType::UIElementCreated ) );
}

} // namespace editor
