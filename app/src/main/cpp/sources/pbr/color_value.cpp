#include <precompiled_headers.hpp>
#include <precompiled_headers.hpp>
#include <pbr/color_value.hpp>


namespace pbr {

ColorValue &ColorValue::operator = ( ColorValue &&other ) noexcept
{
    if ( this == &other ) [[unlikely]]
        return *this;

    _srgb = other._srgb;
    _inherit = other._inherit;

    if ( _notifyChanged ) [[likely]]
        _notifyChanged ( _context );

    return *this;
}

ColorValue &ColorValue::operator = ( GXColorRGB const &srgb ) noexcept
{
    _srgb = srgb;

    if ( _notifyChanged ) [[likely]]
        _notifyChanged ( _context );

    return *this;
}

ColorValue::ColorValue ( bool inherit, GXColorRGB const &srgb ) noexcept:
    _inherit ( inherit ),
    _srgb ( srgb )
{
    // NOTHING
}

void ColorValue::AttachNotifier ( Context context, NotifyChanged notifier ) noexcept
{
    _context = context;
    _notifyChanged = notifier;
}

GXColorRGB ColorValue::GetLinearColor () const noexcept
{
    return _srgb.ToLinearSpace ();
}

GXColorRGB const &ColorValue::GetSRGB () const noexcept
{
    return _srgb;
}

bool ColorValue::IsInherit () const noexcept
{
    return _inherit;
}

} // namespace pbr
