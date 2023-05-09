#ifndef PBR_IMAGE_UI_ELEMENT_H
#define PBR_IMAGE_UI_ELEMENT_H


#include "css_computed_values.h"
#include "ui_element.h"


namespace pbr {

class ImageUIElement : public UIElement
{
    private:
        std::string     _asset;

    public:
        ImageUIElement () = delete;

        ImageUIElement ( ImageUIElement const & ) = delete;
        ImageUIElement& operator = ( ImageUIElement const & ) = delete;

        ImageUIElement ( ImageUIElement && ) = delete;
        ImageUIElement& operator = ( ImageUIElement && ) = delete;

        explicit ImageUIElement ( std::string &&asset, CSSComputedValues const &css ) noexcept;

        ~ImageUIElement () override = default;

    private:
        void ApplyLayout () noexcept override;
        void Render () noexcept override;
};

} // namespace pbr

#endif // PBR_IMAGE_UI_ELEMENT_H
