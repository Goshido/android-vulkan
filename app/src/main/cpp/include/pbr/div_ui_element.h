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
        std::deque<UIElement*>      _children {};

    public:
        CSSComputedValues           _css {};

    public:
        DIVUIElement () = delete;

        DIVUIElement ( DIVUIElement const & ) = delete;
        DIVUIElement& operator = ( DIVUIElement const & ) = delete;

        DIVUIElement ( DIVUIElement && ) = delete;
        DIVUIElement& operator = ( DIVUIElement && ) = delete;

        explicit DIVUIElement ( bool &success,
            UIElement const* parent,
            lua_State &vm,
            int errorHandlerIdx,
            CSSComputedValues const &css
        ) noexcept;

        ~DIVUIElement () override = default;

        void ApplyLayout ( android_vulkan::Renderer &renderer,
            FontStorage &fontStorage,
            CSSUnitToDevicePixel const &cssUnits,
            GXVec2 &penLocation,
            float &lineHeight,
            GXVec2 const &canvasSize,
            float parentLeft,
            float parentWidth
        ) noexcept override;

        void Render () noexcept override;

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


#endif // PBR_DIV_UI_ELEMENT_H
