#ifndef PBR_OPAQUE_MATERIAL_HPP
#define PBR_OPAQUE_MATERIAL_HPP


#include "geometry_pass_material.hpp"


namespace pbr {

class OpaqueMaterial final : public GeometryPassMaterial
{
    public:
        OpaqueMaterial () noexcept;

        OpaqueMaterial ( OpaqueMaterial const & ) = default;
        OpaqueMaterial &operator = ( OpaqueMaterial const & ) = default;

        OpaqueMaterial ( OpaqueMaterial && ) = default;
        OpaqueMaterial &operator = ( OpaqueMaterial && ) = default;

        ~OpaqueMaterial () override = default;
};

} // namespace pbr


#endif // PBR_OPAQUE_MATERIAL_HPP
