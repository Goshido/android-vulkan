#include <precompiled_headers.hpp>
#include <pbr/property.hpp>


namespace pbr {

Property::eType Property::GetType () const noexcept
{
    return _type;
}

Property::Property ( eType type ) noexcept:
    _type ( type )
{
    // NOTHING
}

} // namespace pbr
