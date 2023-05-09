#include <pbr/image_ui_element.h>


namespace pbr {

ImageUIElement::ImageUIElement ( std::string &&asset, CSSComputedValues const &/*css*/ ) noexcept:
    UIElement ( true ),
    _asset ( std::move ( asset ) )
{
    // TODO
}

void ImageUIElement::ApplyLayout () noexcept
{
    // TODO
}

void ImageUIElement::Render () noexcept
{
    // TODO
}

} // namespace pbr
