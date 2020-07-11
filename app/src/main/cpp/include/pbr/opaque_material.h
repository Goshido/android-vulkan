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
        ~OpaqueMaterial () override = default;

        OpaqueMaterial ( const OpaqueMaterial &other ) = delete;
        OpaqueMaterial& operator = ( const OpaqueMaterial &other ) = delete;

        [[maybe_unused]] void SetAlbedo ( Texture2DRef &texture );
        [[maybe_unused]] void SetAlbedoDefault ();

        [[maybe_unused]] void SetEmission ( Texture2DRef &texture );
        [[maybe_unused]] void SetEmissionDefault ();

        [[maybe_unused]] void SetNormal ( Texture2DRef &texture );
        [[maybe_unused]] void SetNormalDefault ();

        [[maybe_unused]] void SetParam ( Texture2DRef &texture );
        [[maybe_unused]] void SetParamDefault ();
};

} // namespace pbr


#endif // OPAQUE_MATERIAL_H
