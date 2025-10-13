#include <precompiled_headers.hpp>
#include <platform/android/pbr/stipple_material.hpp>


namespace pbr {

StippleMaterial::StippleMaterial () noexcept:
    GeometryPassMaterial ( eMaterialType::Stipple )
{
    // NOTHING
}

} // namespace pbr
