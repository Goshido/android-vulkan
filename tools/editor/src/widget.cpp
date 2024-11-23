#include <precompiled_headers.hpp>
#include <widget.hpp>


namespace editor {

void Widget::OnKeyDown ( eKey /*key*/ ) noexcept
{
    // NOTHING
}

void Widget::OnKeyUp ( eKey /*key*/ ) noexcept
{
    // NOTHING
}

void Widget::OnMouseKeyDown ( MouseKeyEvent const &/*event*/ ) noexcept
{
    // NOTHING
}

void Widget::OnMouseKeyUp ( MouseKeyEvent const &/*event*/ ) noexcept
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
