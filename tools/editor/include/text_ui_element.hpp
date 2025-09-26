#ifndef EDITOR_TEXT_UI_ELEMENT_HPP
#define EDITOR_TEXT_UI_ELEMENT_HPP


#include "div_ui_element.hpp"
#include <platform/windows/pbr/text_ui_element.hpp>


namespace editor {

class TextUIElement final : public UIElement
{
    private:
        // FUCK - remove namespace
        pbr::windows::TextUIElement*    _text = nullptr;

    public:
        TextUIElement () = delete;

        TextUIElement ( TextUIElement const & ) = delete;
        TextUIElement &operator = ( TextUIElement const & ) = delete;

        TextUIElement ( TextUIElement && ) = delete;
        TextUIElement &operator = ( TextUIElement && ) = delete;

        explicit TextUIElement ( MessageQueue &messageQueue,
            DIVUIElement &parent,
            std::string_view text,
            std::string &&name
        ) noexcept;

        ~TextUIElement () noexcept override;

        // FUCK - remove namespace
        [[nodiscard]] pbr::windows::UIElement &GetNativeElement () noexcept override;

        void SetColor ( pbr::ColorValue const &color ) noexcept;
        void SetText ( std::string_view text ) noexcept;
        void SetText ( std::u32string_view text ) noexcept;
};

} // namespace editor


#endif // EDITOR_TEXT_UI_ELEMENT_HPP
