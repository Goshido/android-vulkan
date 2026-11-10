#include <precompiled_headers.hpp>
#include <string_property.hpp>


namespace editor {

StringProperty::StringProperty ( std::string &&value ) noexcept:
    _value ( std::move ( value ) )
{
    // NOTHING
}

std::string const &StringProperty::Get () const noexcept
{
    return _value;
}

void StringProperty::Set ( std::string &&value ) noexcept
{
    _value = std::move ( value );
    OnUpdated ();
}

} // namespace editor
