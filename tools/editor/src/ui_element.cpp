#include <precompiled_headers.hpp>
#include <ui_element.hpp>


namespace editor {

UIElement::UIElement ( MessageQueue &messageQueue ) noexcept:
    _messageQueue ( messageQueue )
{
    messageQueue.EnqueueBack (
        {
            ._type = eMessageType::UIElementCreated,
            ._params = nullptr,
            ._serialNumber = 0U
        }
    );
}

} // namespace editor
