#ifndef OPAQUE_MATERIAL_H
#define OPAQUE_MATERIAL_H


#include "types.h"


namespace pbr {

class OpaqueMaterial final : public Material
{
    private:
        Texture2DRef    _albedo;
        Texture2DRef    _emission;
        Texture2DRef    _normal;
        Texture2DRef    _param;

    public:
        OpaqueMaterial ();
        OpaqueMaterial ( const OpaqueMaterial &other ) = default;
        ~OpaqueMaterial () override = default;

        [[nodiscard]] Texture2DRef& GetAlbedo ();
        void SetAlbedo ( Texture2DRef &texture );
        [[maybe_unused]] void SetAlbedoDefault ();

        [[nodiscard]] Texture2DRef& GetEmission ();
        [[maybe_unused]] void SetEmission ( Texture2DRef &texture );
        [[maybe_unused]] void SetEmissionDefault ();

        [[nodiscard]] Texture2DRef& GetNormal ();
        void SetNormal ( Texture2DRef &texture );
        [[maybe_unused]] void SetNormalDefault ();

        [[nodiscard]] Texture2DRef& GetParam ();
        [[maybe_unused]] void SetParam ( Texture2DRef &texture );
        [[maybe_unused]] void SetParamDefault ();

        [[nodiscard]] bool operator < ( const OpaqueMaterial &other ) const;
        OpaqueMaterial& operator = ( const OpaqueMaterial &other ) = default;
};

} // namespace pbr


#endif // OPAQUE_MATERIAL_H