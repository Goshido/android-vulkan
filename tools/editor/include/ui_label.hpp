#ifndef EDITOR_UI_LABEL_HPP
#define EDITOR_UI_LABEL_HPP


#include "message_queue.hpp"
#include "div_ui_element.hpp"
#include "text_ui_element.hpp"


namespace editor {

class UILabel final
{
    private:
        DIVUIElement        _div;
        TextUIElement       _text;
        MessageQueue        &_messageQueue;

    public:
        UILabel () = delete;

        UILabel ( UILabel const & ) = delete;
        UILabel &operator = ( UILabel const & ) = delete;

        UILabel ( UILabel && ) = delete;
        UILabel &operator = ( UILabel && ) = delete;

        explicit UILabel ( MessageQueue &messageQueue,
            DIVUIElement &parent,
            std::string_view text,
            std::string &&name
        ) noexcept;

        ~UILabel () = default;

        // FUCK - remove it
        [[nodiscard]] pbr::CSSComputedValues &GetCSS () noexcept;

        // FUCK - rename
        [[nodiscard]] pbr::CSSComputedValues &GetCSSEXT () noexcept;
};

} // namespace editor


#endif // EDITOR_UI_LABEL_HPP
