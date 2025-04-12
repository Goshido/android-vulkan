#ifndef EDITOR_TEXT_UI_ELEMENT_HPP
#define EDITOR_TEXT_UI_ELEMENT_HPP


#include "div_ui_element.hpp"
#include <pbr/text_ui_element.hpp>


namespace editor {

class TextUIElement final : public UIElement
{
    private:
        pbr::TextUIElement*     _text = nullptr;

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

        [[nodiscard]] pbr::UIElement &GetNativeElement () noexcept override;

        void SetColor ( pbr::ColorValue const &color ) noexcept;
        void SetText ( std::string_view text ) noexcept;
};

} // namespace editor


#endif // EDITOR_TEXT_UI_ELEMENT_HPP
