#ifndef EDITOR_WIDGET_HPP
#define EDITOR_WIDGET_HPP


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
        Rect            _rect {};

    public:
        Widget ( Widget const & ) = delete;
        Widget &operator = ( Widget const & ) = delete;

        Widget ( Widget && ) = delete;
        Widget &operator = ( Widget && ) = delete;

        virtual ~Widget () = default;

        virtual void OnKeyDown ( eKey key ) noexcept;
        virtual void OnKeyUp ( eKey key ) noexcept;

        virtual void OnMouseKeyDown ( MouseKeyEvent const &event ) noexcept;
        virtual void OnMouseKeyUp ( MouseKeyEvent const &event ) noexcept;
        virtual void OnMouseMove ( MouseMoveEvent const &event ) noexcept;

        [[nodiscard]] virtual LayoutStatus ApplyLayout ( android_vulkan::Renderer &renderer,
            pbr::FontStorage &fontStorage
        ) noexcept = 0;

        virtual void Submit ( pbr::UIElement::SubmitInfo &info ) noexcept = 0;

        [[nodiscard]] virtual bool UpdateCache ( pbr::FontStorage &fontStorage,
            VkExtent2D const &viewport
        ) noexcept = 0;

        [[nodiscard]] bool IsOverlapped ( int32_t x, int32_t y ) const noexcept;

    protected:
        explicit Widget () = default;
};

} // namespace editor


#endif // EDITOR_WIDGET_HPP
