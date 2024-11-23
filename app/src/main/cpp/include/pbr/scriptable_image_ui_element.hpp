#ifndef PBR_SCRIPTABLE_IMAGE_UI_ELEMENT_HPP
#define PBR_SCRIPTABLE_IMAGE_UI_ELEMENT_HPP


#include "image_ui_element.hpp"
#include "scriptable_ui_element.hpp"


namespace pbr {

class ScriptableImageUIElement final : public ScriptableUIElement
{
    private:
        ImageUIElement       _image;

    public:
        ScriptableImageUIElement () = delete;

        ScriptableImageUIElement ( ScriptableImageUIElement const & ) = delete;
        ScriptableImageUIElement &operator = ( ScriptableImageUIElement const & ) = delete;

        ScriptableImageUIElement ( ScriptableImageUIElement && ) = delete;
        ScriptableImageUIElement &operator = ( ScriptableImageUIElement && ) = delete;

        explicit ScriptableImageUIElement ( bool &success,
            UIElement const* parent,
            lua_State &vm,
            int errorHandlerIdx,
            std::string &&asset,
            CSSComputedValues &&css
        ) noexcept;

        ~ScriptableImageUIElement () override = default;

        [[nodiscard]] UIElement &GetElement () noexcept override;
        [[nodiscard]] UIElement const &GetElement () const noexcept override;
};

} // namespace pbr


#endif // PBR_SCRIPTABLE_IMAGE_UI_ELEMENT_HPP
