#include <pbr/position_property.h>


namespace pbr {

PositionProperty::PositionProperty ( eValue value ) noexcept:
    Property ( Property::eType::Position ),
    _value ( value )
{
    // NOTHING
}

PositionProperty::eValue PositionProperty::GetValue () const noexcept
{
    return _value;
}

} // namespace pbr
