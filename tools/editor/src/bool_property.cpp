#include <precompiled_headers.hpp>
#include <bool_property.hpp>


namespace editor {

BoolProperty &BoolProperty::operator = ( bool value ) noexcept
{
    _value = value;
    OnUpdated ();
    return *this;
}

BoolProperty::BoolProperty ( bool value ) noexcept:
    _value ( value )
{
    // NOTHING
}

BoolProperty::operator bool () const noexcept
{
    return _value;
}

} // namespace editor
