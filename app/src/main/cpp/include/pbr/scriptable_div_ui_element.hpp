#ifndef PBR_SCRIPTABLE_DIV_UI_ELEMENT_HPP
#define PBR_SCRIPTABLE_DIV_UI_ELEMENT_HPP


#include <platform/android/pbr/div_ui_element.hpp>
#include "scriptable_ui_element.hpp"


namespace pbr {

class ScriptableDIVUIElement final : public ScriptableUIElement
{
    private:
        // FUCK - remove namespace
        DIVUIElement    _div;

    public:
        ScriptableDIVUIElement () = delete;

        ScriptableDIVUIElement ( ScriptableDIVUIElement const & ) = delete;
        ScriptableDIVUIElement &operator = ( ScriptableDIVUIElement const & ) = delete;

        ScriptableDIVUIElement ( ScriptableDIVUIElement && ) = delete;
        ScriptableDIVUIElement &operator = ( ScriptableDIVUIElement && ) = delete;

        explicit ScriptableDIVUIElement ( bool &success,
            UIElement const* parent,
            lua_State &vm,
            int errorHandlerIdx,
            CSSComputedValues &&css
        ) noexcept;

        ~ScriptableDIVUIElement () override = default;

        [[nodiscard]] UIElement &GetElement () noexcept override;
        [[nodiscard]] UIElement const &GetElement () const noexcept override;

        // Lua stack must have the following configuration:
        //      stack[ -1 ] -> child element
        //      stack[ -2 ] -> parent element
        // At the end method will remove 'child element' from Lua stack.
        [[nodiscard]] bool AppendChildElement ( lua_State &vm,
            int errorHandlerIdx,
            int appendChildElementIdx,
            UIElement &element
        ) noexcept;
};

} // namespace pbr


#endif // PBR_SCRIPTABLE_DIV_UI_ELEMENT_HPP
