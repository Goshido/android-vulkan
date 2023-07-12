#ifndef PBR_CSS_UI_ELEMENT_HPP
#define PBR_CSS_UI_ELEMENT_HPP


#include "css_computed_values.h"
#include "ui_element.h"


namespace pbr {

class CSSUIElement : public UIElement
{
    public:
        CSSComputedValues       _css {};

    public:
        CSSUIElement () = delete;

        CSSUIElement ( CSSUIElement const & ) = delete;
        CSSUIElement &operator = ( CSSUIElement const & ) = delete;
    
        CSSUIElement ( CSSUIElement && ) = delete;
        CSSUIElement &operator = ( CSSUIElement && ) = delete;

        explicit CSSUIElement ( bool visible, UIElement const* parent, CSSComputedValues &&css ) noexcept;

        ~CSSUIElement () override = default;
};

} // namespace pbr


#endif // PBR_CSS_UI_ELEMENT_HPP
