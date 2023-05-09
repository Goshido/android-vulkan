#include <pbr/div_ui_element.h>


namespace pbr {

DIVUIElement::DIVUIElement ( CSSComputedValues const &css ) noexcept:
    UIElement ( css._display != DisplayProperty::eValue::None ),
    _css ( css )
{
    // NOTHING
}

void DIVUIElement::AppendChildElement ( std::unique_ptr<UIElement> &&element ) noexcept
{
    _childs.emplace_back ( std::move ( element ) );
}

void DIVUIElement::ApplyLayout () noexcept
{
    // TODO
}

void DIVUIElement::Render () noexcept
{
    // TODO
}

} // namespace pbr
