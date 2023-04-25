#include <pbr/display_property.h>


namespace pbr {

DisplayProperty::DisplayProperty ( eValue value ) noexcept:
    Property ( Property::eType::Display ),
    _value ( value )
{
    // NOHTING
}

DisplayProperty::eValue DisplayProperty::GetValue () const noexcept
{
    return _value;
}

} // namespace pbr
