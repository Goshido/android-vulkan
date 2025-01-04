#ifndef EDITOR_UI_CLOSE_BUTTON_HPP
#define EDITOR_UI_CLOSE_BUTTON_HPP


#include "message_queue.hpp"
#include <pbr/div_ui_element.hpp>
#include <pbr/text_ui_element.hpp>


namespace editor {

class UIManager;

class UICloseButton final
{
    friend class UIManager;

    private:
        MessageQueue                            &_messageQueue;

        std::unique_ptr<pbr::DIVUIElement>      _base {};

        std::unique_ptr<pbr::DIVUIElement>      _backgroundDIV {};
        std::unique_ptr<pbr::TextUIElement>     _backgroundText {};

        std::unique_ptr<pbr::DIVUIElement>      _borderDIV {};
        std::unique_ptr<pbr::TextUIElement>     _borderText {};

        std::unique_ptr<pbr::DIVUIElement>      _crossDIV {};
        std::unique_ptr<pbr::TextUIElement>     _crossText {};

    public:
        UICloseButton () = delete;

        UICloseButton ( UICloseButton const & ) = delete;
        UICloseButton &operator = ( UICloseButton const & ) = delete;

        UICloseButton ( UICloseButton && ) = delete;
        UICloseButton &operator = ( UICloseButton && ) = delete;

        explicit UICloseButton ( MessageQueue &messageQueue, pbr::DIVUIElement &parent ) noexcept;

        ~UICloseButton () = default;

        [[nodiscard]] pbr::CSSComputedValues &GetCSS () noexcept;
};

} // namespace editor


#endif // EDITOR_UI_CLOSE_BUTTON_HPP
