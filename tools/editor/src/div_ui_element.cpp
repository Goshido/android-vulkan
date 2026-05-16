#include <precompiled_headers.hpp>
#include <div_ui_element.hpp>
#include <message_queue.hpp>
#include <text_ui_element.hpp>


namespace editor {

DIVUIElement::DIVUIElement ( pbr::CSSComputedValues &&css, std::string &&name ) noexcept:
    _div ( new pbr::DIVUIElement ( nullptr, std::move ( css ), std::move ( name ) ) )
{
    // NOTHING
}

DIVUIElement::DIVUIElement ( DIVUIElement &parent, pbr::CSSComputedValues &&css, std::string &&name ) noexcept:
    _div ( new pbr::DIVUIElement ( &parent.GetNativeElement (), std::move ( css ), std::move ( name ) ) )
{
    // NOTHING
}

DIVUIElement::~DIVUIElement () noexcept
{
    MessageQueue::Instance ().EnqueueBack (
        {
            ._type = eMessageType::UIDeleteElement,

            ._action = [ div = std::exchange ( _div, nullptr ) ] () noexcept {
                delete div;
                return nullptr;
            },

            ._serialNumber = 0U
        }
    );
}

pbr::UIElement &DIVUIElement::GetNativeElement () noexcept
{
    return *_div;
}

void DIVUIElement::AppendChildElement ( DIVUIElement &element ) noexcept
{
    MessageQueue::Instance ().EnqueueBack (
        {
            ._type = eMessageType::UIAppendChildElement,

            ._action = [ &parent = *_div, &element = element.GetNativeElement () ] () noexcept {
                parent.AppendChildElement ( element );
                return nullptr;
            },

            ._serialNumber = 0U
        }
    );
}

void DIVUIElement::PrependChildElement ( DIVUIElement &element ) noexcept
{
    MessageQueue::Instance ().EnqueueBack (
        {
            ._type = eMessageType::UIPrependChildElement,

            ._action = [ &parent = *_div, &element = element.GetNativeElement () ] () noexcept {
                parent.PrependChildElement ( element );
                return nullptr;
            },

            ._serialNumber = 0U
        }
    );
}

void DIVUIElement::AppendChildElement ( TextUIElement &element ) noexcept
{
    MessageQueue::Instance ().EnqueueBack (
        {
            ._type = eMessageType::UIAppendChildElement,

            ._action = [ &parent = *_div, &element = element.GetNativeElement () ] () noexcept {
                parent.AppendChildElement ( element );
                return nullptr;
            },

            ._serialNumber = 0U
        }
    );
}

void DIVUIElement::PrependChildElement ( TextUIElement &element ) noexcept
{
    MessageQueue::Instance ().EnqueueBack (
        {
            ._type = eMessageType::UIPrependChildElement,

            ._action = [ &parent = *_div, &element = element.GetNativeElement () ] () noexcept {
                parent.PrependChildElement ( element );
                return nullptr;
            },

            ._serialNumber = 0U
        }
    );
}

void DIVUIElement::Hide () noexcept
{
    MessageQueue::Instance ().EnqueueBack (
        {
            ._type = eMessageType::UIHideElement,

            ._action = [ &div = *_div ] () noexcept {
                div.Hide ();
                return nullptr;
            },

            ._serialNumber = 0U
        }
    );
}

void DIVUIElement::Show () noexcept
{
    MessageQueue::Instance ().EnqueueBack (
        {
            ._type = eMessageType::UIShowElement,

            ._action = [ &div = *_div ] () noexcept {
                div.Show ();
                return nullptr;
            },

            ._serialNumber = 0U
        }
    );
}

bool DIVUIElement::IsVisible () const noexcept
{
    return _div->IsVisible ();
}

void DIVUIElement::Update () noexcept
{
    MessageQueue::Instance ().EnqueueBack (
        {
            ._type = eMessageType::UIUpdateElement,

            ._action = [ &div = *_div ] () noexcept {
                div.Update ();
                return nullptr;
            },

            ._serialNumber = 0U
        }
    );
}

pbr::DIVUIElement::Rect const &DIVUIElement::GetAbsoluteRect () const noexcept
{
    return _div->GetAbsoluteRect ();
}

pbr::CSSComputedValues &DIVUIElement::GetCSS () noexcept
{
    return _div->GetCSS ();
}

void DIVUIElement::ApplyLayout ( pbr::UIElement::ApplyInfo &info ) noexcept
{
    _div->ApplyLayout ( info );
}

void DIVUIElement::Submit ( pbr::UIElement::SubmitInfo &info ) noexcept
{
    _div->Submit ( info );
}

bool DIVUIElement::UpdateCache ( pbr::UIElement::UpdateInfo &info ) noexcept
{
    return _div->UpdateCache ( info );
}

} // namespace editor
