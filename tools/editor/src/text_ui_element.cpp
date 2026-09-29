#include <precompiled_headers.hpp>
#include <message_queue.hpp>
#include <text_ui_element.hpp>
#include <trace.hpp>


namespace editor {

TextUIElement::TextUIElement ( DIVUIElement &parent, std::string_view text, std::string &&name ) noexcept:
    _text ( new pbr::TextUIElement ( true, &parent.GetNativeElement (), text, std::move ( name ) ) )
{
    // NOTHING
}

TextUIElement::~TextUIElement () noexcept
{
    AV_TRACE ( "Destroy text element" )
    MessageQueue::Instance ().EnqueueBack (
        Message ( eMessageType::UIDeleteElement,
            [ text = std::exchange ( _text, nullptr ) ] () noexcept {
                AV_TRACE ( "Destroy text element" )
                delete text;
                return nullptr;
            }
        )
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

void TextUIElement::SetText ( std::string &&text ) noexcept
{
    AV_TRACE ( "Set text" )

    MessageQueue::Instance ().EnqueueBack (
        Message ( eMessageType::UISetText,
            [ &element = *_text, t = std::move ( text ) ] () noexcept {
                AV_TRACE ( "Set text" )
                element.SetText ( std::string_view ( t ) );
                return nullptr;
            }
        )
    );
}

void TextUIElement::SetText ( std::string_view text ) noexcept
{
    SetText ( std::string ( text ) );
}

void TextUIElement::SetText ( std::u32string_view text ) noexcept
{
    AV_TRACE ( "Set text" )

    MessageQueue::Instance ().EnqueueBack (
        Message ( eMessageType::UISetText,
            [ &element = *_text, t = std::move ( std::u32string ( text ) ) ] () noexcept {
                AV_TRACE ( "Set text" )
                element.SetText ( std::u32string_view ( t ) );
                return nullptr;
            }
        )
    );
}

} // namespace editor
