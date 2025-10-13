#ifndef EDITOR_DIV_UI_ELEMENT_HPP
#define EDITOR_DIV_UI_ELEMENT_HPP


#include <platform/windows/pbr/div_ui_element.hpp>

#include "ui_element.hpp"


namespace editor {

class TextUIElement;

class DIVUIElement final : public UIElement
{
    private:
        pbr::DIVUIElement*     _div = nullptr;

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

        [[nodiscard]] pbr::UIElement &GetNativeElement () noexcept override;

        void AppendChildElement ( DIVUIElement &element ) noexcept;
        void PrependChildElement ( DIVUIElement &element ) noexcept;

        void AppendChildElement ( TextUIElement &element ) noexcept;
        void PrependChildElement ( TextUIElement &element ) noexcept;

        void Hide () noexcept;
        void Show () noexcept;
        [[nodiscard]] bool IsVisible () const noexcept;
        void Update () noexcept;

        [[nodiscard]] pbr::DIVUIElement::Rect const &GetAbsoluteRect () const noexcept;
        [[nodiscard]] pbr::CSSComputedValues &GetCSS () noexcept;

        void ApplyLayout ( pbr::UIElement::ApplyInfo &info ) noexcept;
        void Submit ( pbr::UIElement::SubmitInfo &info ) noexcept;
        [[nodiscard]] bool UpdateCache ( pbr::UIElement::UpdateInfo &info ) noexcept;
};

} // namespace editor


#endif // EDITOR_DIV_UI_ELEMENT_HPP
