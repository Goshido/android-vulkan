#include <precompiled_headers.hpp>
#include <set_text_event.hpp>
#include <text_ui_element.hpp>


namespace editor {

TextUIElement::TextUIElement ( MessageQueue &messageQueue,
    DIVUIElement &parent,
    std::string_view text,
    std::string &&name
) noexcept:
    UIElement ( messageQueue ),

    // FUCK - remove namespace
    _text ( new pbr::android::TextUIElement ( true, &parent.GetNativeElement (), text, std::move ( name ) ) )
{
    // NOTHING
}

TextUIElement::~TextUIElement () noexcept
{
    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::UIDeleteElement,
            ._params = std::exchange ( _text, nullptr ),
            ._serialNumber = 0U
        }
    );
}

pbr::UIElement &TextUIElement::GetNativeElement () noexcept
{
    return *_text;
}

void TextUIElement::SetColor ( pbr::ColorValue const &color ) noexcept
{
    _text->SetColor ( color );
}

void TextUIElement::SetText ( std::string_view text ) noexcept
{
    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::UISetText,
            ._params = SetTextEvent::Create ( *_text, text ),
            ._serialNumber = 0U
        }
    );
}

void TextUIElement::SetText ( std::u32string_view text ) noexcept
{
    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::UISetText,
            ._params = SetTextEvent::Create ( *_text, text ),
            ._serialNumber = 0U
        }
    );
}

} // namespace editor
