#include <precompiled_headers.hpp>
#include <pbr/opaque_material.hpp>


namespace pbr {

OpaqueMaterial::OpaqueMaterial () noexcept:
    GeometryPassMaterial ( eMaterialType::Opaque )
{
    // NOTHING
}

} // namespace pbr
