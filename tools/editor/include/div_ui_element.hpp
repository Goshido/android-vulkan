#ifndef EDITOR_DIV_UI_ELEMENT_HPP
#define EDITOR_DIV_UI_ELEMENT_HPP


// FUCK - remove namespace
#include <platform/android/pbr/div_ui_element.hpp>
#include <platform/windows/pbr/div_ui_element.hpp>

#include "ui_element.hpp"


namespace editor {

class TextUIElement;

class DIVUIElement final : public UIElement
{
    private:
        // FUCK - remove it
        pbr::android::DIVUIElement*     _div = nullptr;

        // FUCK - remove namespace
        pbr::windows::DIVUIElement*     _divEXT = nullptr;

    public:
        DIVUIElement () = delete;

        DIVUIElement ( DIVUIElement const & ) = delete;
        DIVUIElement &operator = ( DIVUIElement const & ) = delete;

        DIVUIElement ( DIVUIElement && ) = delete;
        DIVUIElement &operator = ( DIVUIElement && ) = delete;

        explicit DIVUIElement ( MessageQueue &messageQueue,
            pbr::CSSComputedValues &&css,
            std::string &&name
        ) noexcept;

        explicit DIVUIElement ( MessageQueue &messageQueue,
            DIVUIElement &parent,
            pbr::CSSComputedValues &&css,
            std::string &&name
        ) noexcept;

        ~DIVUIElement () noexcept override;

        // FUCK - remove it
        [[nodiscard]] pbr::android::UIElement &GetNativeElement () noexcept override;

        // FUCK - remove namespace
        [[nodiscard]] pbr::windows::UIElement &GetNativeElementEXT () noexcept override;

        void AppendChildElement ( DIVUIElement &element ) noexcept;
        void PrependChildElement ( DIVUIElement &element ) noexcept;

        void AppendChildElement ( TextUIElement &element ) noexcept;
        void PrependChildElement ( TextUIElement &element ) noexcept;

        void Hide () noexcept;
        void Show () noexcept;
        [[nodiscard]] bool IsVisible () const noexcept;
        void Update () noexcept;

        // FUCK - remove it
        [[nodiscard]] pbr::android::DIVUIElement::Rect const &GetAbsoluteRect () const noexcept;

        // FUCK - remove namespace
        [[nodiscard]] pbr::windows::DIVUIElement::Rect const &GetAbsoluteRectEXT () const noexcept;

        // FUCK - remove it
        [[nodiscard]] pbr::CSSComputedValues &GetCSS () noexcept;

        // FUCK - rename
        [[nodiscard]] pbr::CSSComputedValues &GetCSSEXT () noexcept;

        // FUCK - remove namespace
        void ApplyLayout ( pbr::android::UIElement::ApplyInfo &info,
            pbr::windows::UIElement::ApplyInfo &infoEXT
        ) noexcept;

        // FUCK - remove namespace
        void Submit ( pbr::android::UIElement::SubmitInfo &info,
            pbr::windows::UIElement::SubmitInfo &infoEXT
        ) noexcept;

        // FUCK - remove namespace
        [[nodiscard]] bool UpdateCache ( pbr::android::UIElement::UpdateInfo &info,
            pbr::windows::UIElement::UpdateInfo &infoEXT
        ) noexcept;
};

} // namespace editor


#endif // EDITOR_DIV_UI_ELEMENT_HPP
