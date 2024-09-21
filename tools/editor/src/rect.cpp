#include <rect.hpp>


namespace editor {

bool Rect::IsOverlapped ( int32_t x, int32_t y ) const noexcept
{
    return ( _left <= x ) & ( _top <= y ) & ( x <= _right ) & ( y <= _bottom );
}

Rect Rect::ApplyOffset ( int32_t x, int32_t y ) const noexcept
{
    return Rect ( _left + x, _right + x, _top + y, _bottom + y );
}

int32_t Rect::GetWidth () const noexcept
{
    return _right - _left;
}

int32_t Rect::GetHeight () const noexcept
{
    return _bottom - _top;
}

void Rect::Normalize () noexcept
{
    int32_t const casesX[] = { _left, _right };
    auto const x = static_cast<size_t> ( _left < _right );
    _left = casesX[ 1U - x ];
    _right = casesX[ x ];

    int32_t const casesY[] = { _top, _bottom };
    auto const y = static_cast<size_t> ( _top < _bottom );
    _top = casesY[ 1U - y ];
    _bottom = casesY[ y ];
}

} // namespace editor
