#include <precompiled_headers.hpp>
#include <text_ui_element.hpp>


namespace editor {

TextUIElement::TextUIElement ( MessageQueue &messageQueue,
    DIVUIElement &parent,
    std::string_view text,
    std::string &&name
) noexcept:
    UIElement ( messageQueue ),
    _text ( new pbr::TextUIElement ( true, &parent.GetNativeElement (), text, std::move ( name ) ) )
{
    // NOTHING
}

TextUIElement::~TextUIElement () noexcept
{
    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::UIDeleteElement,

            ._action = [ text = std::exchange ( _text, nullptr ) ] () noexcept {
                delete text;
                return nullptr;
            },

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

            ._action = [ &element = *_text, t = std::move ( std::string ( text ) ) ] () noexcept {
                element.SetText ( std::string_view ( t ) );
                return nullptr;
            },

            ._serialNumber = 0U
        }
    );
}

void TextUIElement::SetText ( std::u32string_view text ) noexcept
{
    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::UISetText,

            ._action = [ &element = *_text, t = std::move ( std::u32string ( text ) ) ] () noexcept {
                element.SetText ( std::u32string_view ( t ) );
                return nullptr;
            },

            ._serialNumber = 0U
        }
    );
}

} // namespace editor
