#include <pbr/opaque_material.h>
#include <logger.h>


namespace pbr {

OpaqueMaterial::OpaqueMaterial () noexcept:
    Material ( eMaterialType::Opaque ),
    _albedo {},
    _emission {},
    _mask {},
    _normal {},
    _param {}
{
    // NOTHING
}

Texture2DRef& OpaqueMaterial::GetAlbedo ()
{
    return _albedo;
}

void OpaqueMaterial::SetAlbedo ( Texture2DRef const &texture )
{
    _albedo = texture;
}

[[maybe_unused]] void OpaqueMaterial::SetAlbedoDefault ()
{
    _albedo.reset ();
}

Texture2DRef& OpaqueMaterial::GetEmission ()
{
    return _emission;
}

void OpaqueMaterial::SetEmission ( Texture2DRef const &texture )
{
    _emission = texture;
}

void OpaqueMaterial::SetEmissionDefault ()
{
    _emission.reset ();
}

Texture2DRef& OpaqueMaterial::GetMask ()
{
    return _mask;
}

void OpaqueMaterial::SetMask ( Texture2DRef const &texture )
{
    _mask = texture;
}

[[maybe_unused]] void OpaqueMaterial::SetMaskDefault ()
{
    _mask.reset ();
}

Texture2DRef& OpaqueMaterial::GetNormal ()
{
    return _normal;
}

void OpaqueMaterial::SetNormal ( Texture2DRef const &texture )
{
    _normal = texture;
}

[[maybe_unused]] void OpaqueMaterial::SetNormalDefault ()
{
    _normal.reset ();
}

Texture2DRef& OpaqueMaterial::GetParam ()
{
    return _param;
}

void OpaqueMaterial::SetParam ( Texture2DRef const &texture )
{
    _param = texture;
}

[[maybe_unused]] void OpaqueMaterial::SetParamDefault ()
{
    _param = nullptr;
}

bool OpaqueMaterial::operator < ( OpaqueMaterial const &other ) const
{
    constexpr int8_t const less = -1;
    constexpr int8_t const equal = 0;

    auto compare = [] ( Texture2DRef const &a, Texture2DRef const &b ) -> int8_t {
        constexpr int8_t const stringCompare = INT8_MAX;
        constexpr int8_t const greater = 1;
        constexpr int8_t const lookupTable[ 4U ] = { stringCompare, less, greater, equal };

        size_t const selector = ( static_cast<size_t> ( !b ) << 1U ) | static_cast<size_t> ( !a );
        int8_t const result = lookupTable[ selector ];

        return result != stringCompare ? result : static_cast<int8_t> ( a->GetName ().compare ( b->GetName () ) );
    };

    int8_t result = compare ( _albedo, other._albedo );

    if ( result != equal )
        return result <= less;

    result = compare ( _param, other._param );

    if ( result != equal )
        return result <= less;

    result = compare ( _mask, other._mask );

    if ( result != equal )
        return result <= less;

    result = compare ( _normal, other._normal );

    if ( result != equal )
        return result <= less;

    return compare ( _emission, other._emission ) <= less;
}

} // namespace pbr
