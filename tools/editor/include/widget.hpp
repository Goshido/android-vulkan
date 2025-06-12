#ifndef EDITOR_WIDGET_HPP
#define EDITOR_WIDGET_HPP


#include "message_queue.hpp"
#include "mouse_key_event.hpp"
#include "mouse_move_event.hpp"
#include <pbr/font_storage.hpp>
#include <pbr/ui_element.hpp>
#include "rect.hpp"


namespace editor {

class Widget
{
    public:
        struct LayoutStatus final
        {
            bool        _hasChanges = true;
            size_t      _neededUIVertices = 0U;
        };

    protected:
        MessageQueue    &_messageQueue;
        Rect            _rect {};

    private:
        size_t          _hoverEventID = 0U;

    public:
        Widget () = delete;

        Widget ( Widget const & ) = delete;
        Widget &operator = ( Widget const & ) = delete;

        Widget ( Widget && ) = delete;
        Widget &operator = ( Widget && ) = delete;

        virtual ~Widget () = default;

        virtual void OnKeyboardKeyDown ( eKey key, KeyModifier modifier ) noexcept;
        virtual void OnKeyboardKeyUp ( eKey key, KeyModifier modifier ) noexcept;

        virtual void OnMouseLeave () noexcept;
        virtual void OnMouseButtonDown ( MouseKeyEvent const &event ) noexcept;
        virtual void OnMouseButtonUp ( MouseKeyEvent const &event ) noexcept;

        // Must be called by child classes.
        virtual void OnMouseMove ( MouseMoveEvent const &event ) noexcept;

        virtual void OnTyping ( char32_t character ) noexcept;

        [[nodiscard]] virtual LayoutStatus ApplyLayout ( android_vulkan::Renderer &renderer,
            pbr::FontStorage &fontStorage
        ) noexcept;

        virtual void Submit ( pbr::UIElement::SubmitInfo &info ) noexcept;
        [[nodiscard]] virtual bool UpdateCache ( pbr::FontStorage &fontStorage, VkExtent2D const &viewport ) noexcept;
        virtual void UpdatedRect () noexcept;

        [[nodiscard]] bool IsOverlapped ( int32_t x, int32_t y ) const noexcept;

    protected:
        explicit Widget ( MessageQueue &messageQueue ) noexcept;
};

} // namespace editor


#endif // EDITOR_WIDGET_HPP
