#include <pbr/color_property.h>


namespace pbr {

ColorProperty::ColorProperty ( eType type, ColorValue const &value ) noexcept:
    Property ( type ),
    _value ( value )
{
    // NOTHING
}

ColorValue const &ColorProperty::GetValue () const noexcept
{
    return _value;
}

} // namespace pbr
