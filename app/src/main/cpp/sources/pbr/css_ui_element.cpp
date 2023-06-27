#include <pbr/css_ui_element.h>


namespace pbr {

CSSUIElement::CSSUIElement ( bool visible, UIElement const* parent, CSSComputedValues &&css ) noexcept:
    UIElement ( visible, parent ),
    _css ( std::move ( css ) )
{
    // NOTHING
}

} // namespace pbr
