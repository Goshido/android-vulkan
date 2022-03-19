#include <pbr/opaque_material.h>


namespace pbr {

OpaqueMaterial::OpaqueMaterial () noexcept:
    GeometryPassMaterial ( eMaterialType::Opaque )
{
    // NOTHING
}

} // namespace pbr
