#include <widget.hpp>


namespace editor {

void Widget::OnButtonDown () noexcept
{
    // NOTHING
}

void Widget::OnButtonUp () noexcept
{
    // NOTHING
}

void Widget::OnMouseMove ( MouseMoveEvent const &/*event*/ ) noexcept
{
    // NOTHING
}

bool Widget::IsOverlapped ( int32_t x, int32_t y ) const noexcept
{
    return _rect.IsOverlapped ( x, y );
}

} // namespace editor
