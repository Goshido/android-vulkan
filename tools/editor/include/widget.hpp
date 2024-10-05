#ifndef EDITOR_WIDGET_HPP
#define EDITOR_WIDGET_HPP


#include "mouse_move_event.hpp"
#include "rect.hpp"


namespace editor {

class Widget
{
    protected:
        Rect    _rect {};

    public:
        Widget ( Widget const & ) = delete;
        Widget &operator = ( Widget const & ) = delete;

        Widget ( Widget && ) = delete;
        Widget &operator = ( Widget && ) = delete;

        virtual ~Widget () = default;

        virtual void OnButtonDown () noexcept;
        virtual void OnButtonUp () noexcept;
        virtual void OnMouseMove ( MouseMoveEvent const &event ) noexcept;

        [[nodiscard]] bool IsOverlapped ( int32_t x, int32_t y ) const noexcept;

    protected:
        explicit Widget () = default;
};

} // namespace editor


#endif // EDITOR_WIDGET_HPP
