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
            lua_State &vm,
            int errorHandlerIdx,
            std::string &&asset,
            CSSComputedValues const &css
        ) noexcept;

        ~ImageUIElement () override = default;

        static void Init ( lua_State &vm ) noexcept;

    private:
        void ApplyLayout () noexcept override;
        void Render () noexcept override;

        [[nodiscard]] static int OnGarbageCollected ( lua_State* state );
};

} // namespace pbr

#endif // PBR_IMAGE_UI_ELEMENT_H
