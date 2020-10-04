#include <pbr/material.h>


namespace pbr {

eMaterialType Material::GetMaterialType () const
{
    return _type;
}

Material::Material ( eMaterialType type ):
    _type ( type )
{
    // NOTHING
}

} // namespace pbr
