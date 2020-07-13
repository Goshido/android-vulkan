#include <pbr/opaque_material.h>


namespace pbr {

OpaqueMaterial::OpaqueMaterial ():
    Material ( eMaterialType::Opaque )
{
    // NOTHING
}

Texture2DRef& OpaqueMaterial::GetAlbedo ()
{
    return _albedo;
}

void OpaqueMaterial::SetAlbedo ( Texture2DRef &texture )
{
    _albedo = texture;
}

void OpaqueMaterial::SetAlbedoDefault ()
{
    _albedo = nullptr;
}

Texture2DRef& OpaqueMaterial::GetEmission ()
{
    return _emission;
}

void OpaqueMaterial::SetEmission ( Texture2DRef &texture )
{
    _emission = texture;
}

void OpaqueMaterial::SetEmissionDefault ()
{
    _emission = nullptr;
}

Texture2DRef& OpaqueMaterial::GetNormal ()
{
    return _normal;
}

void OpaqueMaterial::SetNormal ( Texture2DRef &texture )
{
    _normal = texture;
}

void OpaqueMaterial::SetNormalDefault ()
{
    _normal = nullptr;
}

Texture2DRef& OpaqueMaterial::GetParam ()
{
    return _param;
}

void OpaqueMaterial::SetParam ( Texture2DRef &texture )
{
    _param = texture;
}

void OpaqueMaterial::SetParamDefault ()
{
    _param = nullptr;
}

bool OpaqueMaterial::operator < ( const OpaqueMaterial &other ) const
{
    constexpr const int8_t less = -1;
    constexpr const int8_t equal = 0;
    constexpr const int8_t greater = 1;

    auto compare = [] ( const Texture2DRef &a, const Texture2DRef &b ) -> int8_t {
        constexpr const int8_t stringCompare = INT8_MAX;
        constexpr const int8_t lookupTable[ 4U ] = { stringCompare, less, greater, equal };

        const size_t selector = ( static_cast<size_t> ( !b ) << 1U ) | static_cast<size_t> ( !a );
        const int8_t result = lookupTable[ selector ];

        return result != stringCompare ? result : static_cast<int8_t> ( a->GetName ().compare ( b->GetName () ) );
    };

    int8_t result = compare ( _albedo, other._albedo );

    if ( result != equal )
        return result == less;

    result = compare ( _param, other._param );

    if ( result != equal )
        return result == less;

    result = compare ( _normal, other._normal );

    if ( result != equal )
        return result == less;

    return compare ( _emission, other._emission ) == less;
}

} // namespace pbr
