#ifndef EDITOR_UI_LABEL_HPP
#define EDITOR_UI_LABEL_HPP


#include "message_queue.hpp"
#include <pbr/div_ui_element.hpp>
#include <pbr/text_ui_element.hpp>


namespace editor {

class UIManager;

class UILabel final
{
    friend class UIManager;

    private:
        std::unique_ptr<pbr::DIVUIElement>      _div {};
        std::unique_ptr<pbr::TextUIElement>     _text {};
        MessageQueue                            &_messageQueue;

    public:
        UILabel () = delete;

        UILabel ( UILabel const & ) = delete;
        UILabel &operator = ( UILabel const & ) = delete;

        UILabel ( UILabel && ) = delete;
        UILabel &operator = ( UILabel && ) = delete;

        explicit UILabel ( MessageQueue &messageQueue, pbr::DIVUIElement &parent, std::string_view text ) noexcept;

        ~UILabel () = default;

        void operator = ( std::string &&text ) noexcept;

        [[nodiscard]] pbr::CSSComputedValues &GetCSS () noexcept;

    private:
        static void OnSetText ( Message &&message ) noexcept;
};

} // namespace editor


#endif // EDITOR_UI_LABEL_HPP
