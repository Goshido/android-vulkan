#include <precompiled_headers.hpp>
#include <message_queue.hpp>
#include <ui_element.hpp>


namespace editor {

UIElement::UIElement () noexcept
{
    MessageQueue::Instance ().EnqueueBack (
        {
            ._type = eMessageType::UIElementCreated,
            ._action = nullptr,
            ._serialNumber = 0U
        }
    );
}

} // namespace editor
