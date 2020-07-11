#include <pbr/opaque_material.h>


namespace pbr {

OpaqueMaterial::OpaqueMaterial ():
    Material ( eMaterialType::Opaque )
{
    // NOTHING
}

void OpaqueMaterial::SetAlbedo ( Texture2DRef &texture )
{
    _albedo = texture;
}

void OpaqueMaterial::SetAlbedoDefault ()
{
    _albedo = nullptr;
}

void OpaqueMaterial::SetEmission ( Texture2DRef &texture )
{
    _emission = texture;
}

void OpaqueMaterial::SetEmissionDefault ()
{
    _emission = nullptr;
}

void OpaqueMaterial::SetNormal ( Texture2DRef &texture )
{
    _normal = texture;
}

void OpaqueMaterial::SetNormalDefault ()
{
    _normal = nullptr;
}

void OpaqueMaterial::SetParam ( Texture2DRef &texture )
{
    _param = texture;
}

void OpaqueMaterial::SetParamDefault ()
{
    _param = nullptr;
}

} // namespace pbr
