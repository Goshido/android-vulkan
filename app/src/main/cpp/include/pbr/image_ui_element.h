#ifndef PBR_IMAGE_UI_ELEMENT_H
#define PBR_IMAGE_UI_ELEMENT_H


#include "css_computed_values.h"
#include "ui_element.h"


namespace pbr {

class ImageUIElement : public UIElement
{
    private:
        std::string             _asset {};
        CSSComputedValues       _css {};


    public:
        ImageUIElement () = delete;

        ImageUIElement ( ImageUIElement const & ) = delete;
        ImageUIElement& operator = ( ImageUIElement const & ) = delete;

        ImageUIElement ( ImageUIElement && ) = delete;
        ImageUIElement& operator = ( ImageUIElement && ) = delete;

        explicit ImageUIElement ( bool &success,
            UIElement const* parent,
            lua_State &vm,
            int errorHandlerIdx,
            std::string &&asset,
            CSSComputedValues &&css
        ) noexcept;

        ~ImageUIElement () override = default;

    private:
        void ApplyLayout ( ApplyLayoutInfo &info ) noexcept override;
        void Submit ( SubmitInfo &info ) noexcept override;
};

} // namespace pbr


#endif // PBR_IMAGE_UI_ELEMENT_H
