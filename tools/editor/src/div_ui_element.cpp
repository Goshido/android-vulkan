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
    _div ( new pbr::android::DIVUIElement ( nullptr, pbr::CSSComputedValues ( css ), std::string ( name ) ) ),
    _divEXT ( new pbr::windows::DIVUIElement ( nullptr, std::move ( css ), std::move ( name ) ) )
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
    _div (
        new pbr::android::DIVUIElement ( &parent.GetNativeElement (),
            pbr::CSSComputedValues ( css ),
            std::string ( name ) )
    ),

    _divEXT ( new pbr::windows::DIVUIElement ( &parent.GetNativeElementEXT (), std::move ( css ), std::move ( name ) ) )
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

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::UIDeleteElementEXT,
            ._params = std::exchange ( _divEXT, nullptr ),
            ._serialNumber = 0U
        }
    );
}

// FUCK - remove it
pbr::android::UIElement &DIVUIElement::GetNativeElement () noexcept
{
    return *_div;
}

// FUCK - remove namespace
pbr::windows::UIElement &DIVUIElement::GetNativeElementEXT () noexcept
{
    return *_divEXT;
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

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::UIAppendChildElement,
            ._params = new AppendUIChildElementEvent ( *_divEXT, element.GetNativeElementEXT () ),
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

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::UIPrependChildElement,
            ._params = new PrependUIChildElementEvent ( *_divEXT, element.GetNativeElementEXT () ),
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

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::UIAppendChildElement,
            ._params = new AppendUIChildElementEvent ( *_divEXT, element.GetNativeElementEXT () ),
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

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::UIPrependChildElement,
            ._params = new PrependUIChildElementEvent ( *_divEXT, element.GetNativeElementEXT () ),
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

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::UIHideElementEXT,
            ._params = _divEXT,
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

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::UIShowElementEXT,
            ._params = _divEXT,
            ._serialNumber = 0U
        }
    );
}

bool DIVUIElement::IsVisible () const noexcept
{
    return _div->IsVisible () && _divEXT->IsVisible ();
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

    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::UIUpdateElementEXT,
            ._params = _divEXT,
            ._serialNumber = 0U
        }
    );
}

// FUCK - remove it
pbr::android::DIVUIElement::Rect const &DIVUIElement::GetAbsoluteRect () const noexcept
{
    return _div->GetAbsoluteRect ();
}

// FUCK - remove namespace
pbr::windows::DIVUIElement::Rect const &DIVUIElement::GetAbsoluteRectEXT () const noexcept
{
    return _divEXT->GetAbsoluteRect ();
}

// FUCK - remove it
pbr::CSSComputedValues &DIVUIElement::GetCSS () noexcept
{
    return _div->GetCSS ();
}

// FUCK - rename
pbr::CSSComputedValues &DIVUIElement::GetCSSEXT () noexcept
{
    return _divEXT->GetCSS ();
}

// FUCK - remove namespace
void DIVUIElement::ApplyLayout ( pbr::android::UIElement::ApplyInfo &info,
    pbr::windows::UIElement::ApplyInfo &infoEXT
) noexcept
{
    _div->ApplyLayout ( info );
    _divEXT->ApplyLayout ( infoEXT );
}

// FUCK - remove namespace
void DIVUIElement::Submit ( pbr::android::UIElement::SubmitInfo &info,
    pbr::windows::UIElement::SubmitInfo &infoEXT
) noexcept
{
    _div->Submit ( info );
    _divEXT->Submit ( infoEXT );
}

// FUCK - remove namespace
bool DIVUIElement::UpdateCache ( pbr::android::UIElement::UpdateInfo &info,
    pbr::windows::UIElement::UpdateInfo &infoEXT
) noexcept
{
    return _div->UpdateCache ( info ) && _divEXT->UpdateCache ( infoEXT );
}

} // namespace editor
