#ifndef PBR_GEOMETRY_PASS_MATERIAL_HPP
#define PBR_GEOMETRY_PASS_MATERIAL_HPP


#include "types.h"


namespace pbr {

class GeometryPassMaterial : public Material
{
    private:
        Texture2DRef    _albedo {};
        Texture2DRef    _emission {};
        Texture2DRef    _mask {};
        Texture2DRef    _normal {};
        Texture2DRef    _param {};

    public:
        GeometryPassMaterial () = delete;

        GeometryPassMaterial ( GeometryPassMaterial const & ) = default;
        GeometryPassMaterial &operator = ( GeometryPassMaterial const & ) = default;

        GeometryPassMaterial ( GeometryPassMaterial && ) = default;
        GeometryPassMaterial &operator = ( GeometryPassMaterial && ) = default;

        ~GeometryPassMaterial () override = default;

        [[nodiscard]] Texture2DRef &GetAlbedo () noexcept;
        void SetAlbedo ( Texture2DRef const &texture ) noexcept;
        [[maybe_unused]] void SetAlbedoDefault () noexcept;

        [[nodiscard]] Texture2DRef &GetEmission () noexcept;
        void SetEmission ( Texture2DRef const &texture ) noexcept;
        [[maybe_unused]] void SetEmissionDefault () noexcept;

        [[nodiscard]] Texture2DRef &GetMask () noexcept;
        void SetMask ( Texture2DRef const &texture ) noexcept;
        [[maybe_unused]] void SetMaskDefault () noexcept;

        [[nodiscard]] Texture2DRef &GetNormal () noexcept;
        void SetNormal ( Texture2DRef const &texture ) noexcept;
        [[maybe_unused]] void SetNormalDefault () noexcept;

        [[nodiscard]] Texture2DRef &GetParam () noexcept;
        void SetParam ( Texture2DRef const &texture ) noexcept;
        [[maybe_unused]] void SetParamDefault () noexcept;

        [[nodiscard]] bool operator < ( GeometryPassMaterial const &other ) const noexcept;

    protected:
        explicit GeometryPassMaterial ( eMaterialType type ) noexcept;
};

} // namespace pbr


#endif // PBR_GEOMETRY_PASS_MATERIAL_HPP
