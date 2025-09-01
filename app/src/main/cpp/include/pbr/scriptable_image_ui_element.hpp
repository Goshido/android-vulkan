#ifndef PBR_SCRIPTABLE_IMAGE_UI_ELEMENT_HPP
#define PBR_SCRIPTABLE_IMAGE_UI_ELEMENT_HPP


// FUCK - remove namespace
#include <platform/android/pbr/image_ui_element.hpp>

#include "scriptable_ui_element.hpp"


namespace pbr {

class ScriptableImageUIElement final : public ScriptableUIElement
{
    private:
        // FUCK - remove namespace
        android::ImageUIElement     _image;

    public:
        ScriptableImageUIElement () = delete;

        ScriptableImageUIElement ( ScriptableImageUIElement const & ) = delete;
        ScriptableImageUIElement &operator = ( ScriptableImageUIElement const & ) = delete;

        ScriptableImageUIElement ( ScriptableImageUIElement && ) = delete;
        ScriptableImageUIElement &operator = ( ScriptableImageUIElement && ) = delete;

        explicit ScriptableImageUIElement ( bool &success,

            // FUCK - remove namespace
            android::UIElement const* parent,

            lua_State &vm,
            int errorHandlerIdx,
            std::string &&asset,
            CSSComputedValues &&css
        ) noexcept;

        ~ScriptableImageUIElement () override = default;

        // FUCK - remove namespace
        [[nodiscard]] android::UIElement &GetElement () noexcept override;
        [[nodiscard]] android::UIElement const &GetElement () const noexcept override;
};

} // namespace pbr


#endif // PBR_SCRIPTABLE_IMAGE_UI_ELEMENT_HPP
