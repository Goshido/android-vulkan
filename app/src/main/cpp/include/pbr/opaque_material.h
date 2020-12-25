#ifndef OPAQUE_MATERIAL_H
#define OPAQUE_MATERIAL_H


#include "types.h"


namespace pbr {

class OpaqueMaterial final : public Material
{
    private:
        Texture2DRef    _albedo;
        Texture2DRef    _emission;
        Texture2DRef    _mask;
        Texture2DRef    _normal;
        Texture2DRef    _param;

    public:
        OpaqueMaterial ();

        OpaqueMaterial ( OpaqueMaterial const & ) = default;
        OpaqueMaterial& operator = ( OpaqueMaterial const & ) = default;

        OpaqueMaterial ( OpaqueMaterial && ) = default;
        OpaqueMaterial& operator = ( OpaqueMaterial && ) = default;

        ~OpaqueMaterial () override = default;

        [[nodiscard]] Texture2DRef& GetAlbedo ();
        void SetAlbedo ( Texture2DRef &texture );
        [[maybe_unused]] void SetAlbedoDefault ();

        [[nodiscard]] Texture2DRef& GetEmission ();
        void SetEmission ( Texture2DRef &texture );
        [[maybe_unused]] void SetEmissionDefault ();

        [[nodiscard]] Texture2DRef& GetMask ();
        void SetMask ( Texture2DRef &texture );
        [[maybe_unused]] void SetMaskDefault ();

        [[nodiscard]] Texture2DRef& GetNormal ();
        void SetNormal ( Texture2DRef &texture );
        [[maybe_unused]] void SetNormalDefault ();

        [[nodiscard]] Texture2DRef& GetParam ();
        void SetParam ( Texture2DRef &texture );
        [[maybe_unused]] void SetParamDefault ();

        [[nodiscard]] bool operator < ( const OpaqueMaterial &other ) const;

};

} // namespace pbr


#endif // OPAQUE_MATERIAL_H
