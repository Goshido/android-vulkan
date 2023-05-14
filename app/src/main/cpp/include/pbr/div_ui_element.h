#ifndef PBR_DIV_UI_ELEMENT_H
#define PBR_DIV_UI_ELEMENT_H


#include "css_computed_values.h"
#include "ui_element.h"

GX_DISABLE_COMMON_WARNINGS

#include <deque>

GX_RESTORE_WARNING_STATE


namespace pbr {

class DIVUIElement final : public UIElement
{
    private:
        std::deque<std::unique_ptr<UIElement>>      _childs {};
        CSSComputedValues                           _css {};

    public:
        DIVUIElement () = delete;

        DIVUIElement ( DIVUIElement const & ) = delete;
        DIVUIElement& operator = ( DIVUIElement const & ) = delete;

        DIVUIElement ( DIVUIElement && ) = delete;
        DIVUIElement& operator = ( DIVUIElement && ) = delete;

        explicit DIVUIElement ( bool &success,
            lua_State &vm,
            int errorHandlerIdx,
            CSSComputedValues const &css
        ) noexcept;

        ~DIVUIElement () override = default;

        // Lua stack must have the following configuration:
        //      stack[ -1 ] -> child element
        //      stack[ -2 ] -> parent element
        // At the end method will remove 'child element' from Lua stack.
        [[nodiscard]] bool AppendChildElement ( lua_State &vm,
            int errorHandlerIdx,
            int appendChildElementIdx,
            std::unique_ptr<UIElement> &&element
        ) noexcept;

        static void Init ( lua_State &vm ) noexcept;

    private:
        void ApplyLayout () noexcept override;
        void Render () noexcept override;

        [[nodiscard]] static int OnGarbageCollected ( lua_State* state );
};

} // namespace pbr

#endif // PBR_DIV_UI_ELEMENT_H
