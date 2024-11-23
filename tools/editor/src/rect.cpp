#include <precompiled_headers.hpp>
#include <pbr/css_unit_to_device_pixel.hpp>
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

GXVec2 Rect::GetTopLeft () const noexcept
{
    return GXVec2 ( static_cast<float> ( _left ), static_cast<float> ( _top ) );
}

GXVec2 Rect::GetBottomRight () const noexcept
{
    return GXVec2 ( static_cast<float> ( _right ), static_cast<float> ( _bottom ) );
}

int32_t Rect::GetWidth () const noexcept
{
    return _right - _left;
}

int32_t Rect::GetHeight () const noexcept
{
    return _bottom - _top;
}

void Rect::From ( GXVec2 const &topLeft, GXVec2 const &bottomRight ) noexcept
{
    _left = static_cast<int32_t> ( topLeft._data[ 0U ] );
    _top = static_cast<int32_t> ( topLeft._data[ 1U ] );
    _right = static_cast<int32_t> ( bottomRight._data[ 0U ] );
    _bottom = static_cast<int32_t> ( bottomRight._data[ 1U ] );
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

void Rect::ToCSSBounds ( pbr::CSSComputedValues &css ) noexcept
{
    float const convert = pbr::CSSUnitToDevicePixel::GetInstance ()._devicePXtoCSSPX;
    css._left = pbr::LengthValue ( pbr::LengthValue::eType::PX, convert * static_cast<float> ( _left ) );
    css._top = pbr::LengthValue ( pbr::LengthValue::eType::PX, convert * static_cast<float> ( _top ) );
    css._width = pbr::LengthValue ( pbr::LengthValue::eType::PX, convert * static_cast<float> ( _right - _left ) );
    css._height = pbr::LengthValue ( pbr::LengthValue::eType::PX, convert * static_cast<float> ( _bottom - _top ) );
}

} // namespace editor
