#include <precompiled_headers.hpp>
#include <pbr/display_property.hpp>


namespace pbr {

DisplayProperty::DisplayProperty ( eValue value ) noexcept:
    Property ( Property::eType::Display ),
    _value ( value )
{
    // NOTHING
}

DisplayProperty::eValue DisplayProperty::GetValue () const noexcept
{
    return _value;
}

} // namespace pbr
