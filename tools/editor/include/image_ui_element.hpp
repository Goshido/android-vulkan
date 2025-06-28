#ifndef EDITOR_IMAGE_UI_ELEMENT_HPP
#define EDITOR_IMAGE_UI_ELEMENT_HPP


#include "div_ui_element.hpp"
#include <pbr/image_ui_element.hpp>


namespace editor {

class ImageUIElement final : public UIElement
{
    private:
        pbr::ImageUIElement     _image;

    public:
        ImageUIElement () = delete;

        ImageUIElement ( ImageUIElement const & ) = delete;
        ImageUIElement &operator = ( ImageUIElement const & ) = delete;

        ImageUIElement ( ImageUIElement && ) = delete;
        ImageUIElement &operator = ( ImageUIElement && ) = delete;

        explicit ImageUIElement ( MessageQueue &messageQueue,
            pbr::CSSComputedValues &&css,
            std::string &&name
        ) noexcept;

        explicit ImageUIElement ( MessageQueue &messageQueue,
            ImageUIElement &parent,
            pbr::CSSComputedValues &&css,
            std::string &&name
        ) noexcept;

        ~ImageUIElement () override = default;
};

} // namespace editor


#endif // EDITOR_IMAGE_UI_ELEMENT_HPP
