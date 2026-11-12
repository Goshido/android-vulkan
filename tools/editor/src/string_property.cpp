#include <precompiled_headers.hpp>
#include <string_property.hpp>


namespace editor {

std::string operator + ( char const* left, StringProperty const &right ) noexcept
{
    return left + right._value;
}

StringProperty &StringProperty::operator = ( std::string &&value ) noexcept
{
    _value = value;
    OnUpdated ();
    return *this;
}

StringProperty::StringProperty ( std::string &&value ) noexcept:
    _value ( std::move ( value ) )
{
    // NOTHING
}

StringProperty::operator std::string const &() const noexcept
{
    return _value;
}

StringProperty::operator std::string_view () const noexcept
{
    return std::string_view ( _value );
}

std::string StringProperty::operator + ( char const* other ) const noexcept
{
    return _value + other;
}

} // namespace editor
