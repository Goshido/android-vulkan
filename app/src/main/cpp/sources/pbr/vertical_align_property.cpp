#include <pbr/vertical_align_property.hpp>


namespace pbr {

VerticalAlignProperty::VerticalAlignProperty ( eValue value ) noexcept:
    Property ( Property::eType::VerticalAlign ),
    _value ( value )
{
    // NOTHING
}

VerticalAlignProperty::eValue VerticalAlignProperty::GetValue () const noexcept
{
    return _value;
}

} // namespace pbr
