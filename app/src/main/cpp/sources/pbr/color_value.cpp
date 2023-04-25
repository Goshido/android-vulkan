#include <pbr/color_value.h>


namespace pbr {

ColorValue::ColorValue ( bool inherit, GXColorRGB const &value ) noexcept:
    _inherit ( inherit ),
    _value ( value )
{
    // NOTHING
}

GXColorRGB const& ColorValue::GetValue () const noexcept
{
    return _value;
}

bool ColorValue::IsInherit () const noexcept
{
    return _inherit;
}

} // namespace pbr
