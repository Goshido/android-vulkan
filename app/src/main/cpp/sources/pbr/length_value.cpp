#include <precompiled_headers.hpp>
#include <pbr/length_value.hpp>


namespace pbr {

LengthValue::eType LengthValue::GetType () const noexcept
{
    return _type;
}

float LengthValue::GetValue () const noexcept
{
    return _value;
}

} // namespace pbr
