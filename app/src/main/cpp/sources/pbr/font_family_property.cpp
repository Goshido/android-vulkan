#include <precompiled_headers.hpp>
#include <pbr/font_family_property.hpp>


namespace pbr {

FontFamilyProperty::FontFamilyProperty ( std::u32string &&value ) noexcept:
    Property ( eType::FontFamily ),
    _value ( std::move ( value ) )
{
    // NOTHING
}

std::u32string const &FontFamilyProperty::GetValue () const noexcept
{
    return _value;
}

} // namespace pbr
