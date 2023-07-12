#include <pbr/length_property.hpp>


namespace pbr {

LengthProperty::LengthProperty ( eType type, LengthValue value ) noexcept:
    Property ( type ),
    _value ( value )
{
    // NOTHING
}

LengthValue LengthProperty::GetValue () const noexcept
{
    return _value;
}

} // namespace pbr
