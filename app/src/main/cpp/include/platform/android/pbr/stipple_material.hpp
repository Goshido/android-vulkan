#ifndef PBR_STIPPLE_MATERIAL_HPP
#define PBR_STIPPLE_MATERIAL_HPP


#include "geometry_pass_material.hpp"


namespace pbr {

class StippleMaterial final : public GeometryPassMaterial
{
    public:
        StippleMaterial () noexcept;

        StippleMaterial ( StippleMaterial const & ) = default;
        StippleMaterial &operator = ( StippleMaterial const & ) = default;

        StippleMaterial ( StippleMaterial && ) = default;
        StippleMaterial &operator = ( StippleMaterial && ) = default;

        ~StippleMaterial () override = default;
};

} // namespace pbr


#endif // PBR_STIPPLE_MATERIAL_HPP
