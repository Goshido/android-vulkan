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

ColorValue &ColorValue::operator = ( GXColorUNORM srgb ) noexcept
{
    _srgb = srgb;

    if ( _notifyChanged ) [[likely]]
        _notifyChanged ( _context );

    return *this;
}

ColorValue::ColorValue ( bool inherit, GXColorUNORM srgb ) noexcept:
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

GXColorUNORM ColorValue::GetSRGB () const noexcept
{
    return _srgb;
}

bool ColorValue::IsInherit () const noexcept
{
    return _inherit;
}

} // namespace pbr
