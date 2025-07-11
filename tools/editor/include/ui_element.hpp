#ifndef EDITOR_UI_ELEMENT_HPP
#define EDITOR_UI_ELEMENT_HPP


#include "message_queue.hpp"
#include <pbr/ui_element.hpp>


namespace editor {

class UIElement
{
    protected:
        MessageQueue    &_messageQueue;

    public:
        UIElement () = delete;

        UIElement ( UIElement const & ) = delete;
        UIElement &operator = ( UIElement const & ) = delete;

        UIElement ( UIElement && ) = delete;
        UIElement &operator = ( UIElement && ) = delete;

        virtual ~UIElement () = default;

        [[nodiscard]] virtual pbr::UIElement &GetNativeElement () noexcept = 0;

    protected:
        explicit UIElement ( MessageQueue &messageQueue ) noexcept;
};

} // namespace editor


#endif // EDITOR_UI_ELEMENT_HPP
