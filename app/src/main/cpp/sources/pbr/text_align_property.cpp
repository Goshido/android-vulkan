#include <precompiled_headers.hpp>
#include <pbr/text_align_property.hpp>


namespace pbr {

TextAlignProperty::TextAlignProperty ( eValue value ) noexcept:
    Property ( Property::eType::TextAlign ),
    _value ( value )
{
    // NOTHING
}

TextAlignProperty::eValue TextAlignProperty::GetValue () const noexcept
{
    return _value;
}

} // namespace pbr
