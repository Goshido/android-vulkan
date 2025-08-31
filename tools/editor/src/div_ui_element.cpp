#include <precompiled_headers.hpp>
#include <append_ui_child_element_event.hpp>
#include <div_ui_element.hpp>
#include <prepend_ui_child_element_event.hpp>
#include <text_ui_element.hpp>


namespace editor {

DIVUIElement::DIVUIElement ( MessageQueue &messageQueue,
    pbr::CSSComputedValues &&css,
    std::string &&name
) noexcept:
    UIElement ( messageQueue ),

    // FUCK - remove namespace
    _div ( new pbr::android::DIVUIElement ( nullptr, std::move ( css ), std::move ( name ) ) )
{
    // NOTHING
}

DIVUIElement::DIVUIElement ( MessageQueue &messageQueue,
    DIVUIElement &parent,
    pbr::CSSComputedValues &&css,
    std::string &&name
) noexcept:
    UIElement ( messageQueue ),

    // FUCK - remove namespace
    _div ( new pbr::android::DIVUIElement ( &parent.GetNativeElement (), std::move ( css ), std::move ( name ) ) )
{
    // NOTHING
}

DIVUIElement::~DIVUIElement () noexcept
{
    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::UIDeleteElement,
            ._params = std::exchange ( _div, nullptr ),
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
    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::UIAppendChildElement,
            ._params = new AppendUIChildElementEvent ( *_div, element.GetNativeElement () ),
            ._serialNumber = 0U
        }
    );
}

void DIVUIElement::PrependChildElement ( DIVUIElement &element ) noexcept
{
    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::UIPrependChildElement,
            ._params = new PrependUIChildElementEvent ( *_div, element.GetNativeElement () ),
            ._serialNumber = 0U
        }
    );
}

void DIVUIElement::AppendChildElement ( TextUIElement &element ) noexcept
{
    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::UIAppendChildElement,
            ._params = new AppendUIChildElementEvent ( *_div, element.GetNativeElement () ),
            ._serialNumber = 0U
        }
    );
}

void DIVUIElement::PrependChildElement ( TextUIElement &element ) noexcept
{
    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::UIPrependChildElement,
            ._params = new PrependUIChildElementEvent ( *_div, element.GetNativeElement () ),
            ._serialNumber = 0U
        }
    );
}

void DIVUIElement::Hide () noexcept
{
    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::UIHideElement,
            ._params = _div,
            ._serialNumber = 0U
        }
    );
}

void DIVUIElement::Show () noexcept
{
    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::UIShowElement,
            ._params = _div,
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
    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::UIUpdateElement,
            ._params = _div,
            ._serialNumber = 0U
        }
    );
}

// FUCK - remove namespace
pbr::android::DIVUIElement::Rect const &DIVUIElement::GetAbsoluteRect () const noexcept
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
