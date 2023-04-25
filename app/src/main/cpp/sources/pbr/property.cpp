#include <pbr/property.h>


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
