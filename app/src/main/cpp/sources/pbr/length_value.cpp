#include <precompiled_headers.hpp>
#include <pbr/length_value.hpp>


namespace pbr {

LengthValue::LengthValue ( eType type, float value ) noexcept:
    _type ( type ),
    _value ( value )
{
    // NOTHING
}

LengthValue::LengthValue ( eType firstType, float firstValue, std::vector<CalcRest> &&rest ) noexcept:
    _type ( firstType ),
    _value ( firstValue ),
    _calcRest ( std::move ( rest ) )
{
    // NOTHING
}

LengthValue::eType LengthValue::GetType () const noexcept
{
    return _type;
}

float LengthValue::GetValue () const noexcept
{
    return _value;
}

std::vector<LengthValue::CalcRest> const &LengthValue::GetCalcRest () const noexcept
{
    return _calcRest;
}

} // namespace pbr
