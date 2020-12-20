#include <pbr/material.h>


namespace pbr {

eMaterialType Material::GetMaterialType () const
{
    return _type;
}

Material::Material ( eMaterialType type ) noexcept:
    _type ( type )
{
    // NOTHING
}

} // namespace pbr
