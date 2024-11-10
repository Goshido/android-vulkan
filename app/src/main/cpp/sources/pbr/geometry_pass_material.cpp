#include <precompiled_headers.hpp>
#include <pbr/geometry_pass_material.hpp>
#include <logger.hpp>


namespace pbr {

Texture2DRef &GeometryPassMaterial::GetAlbedo () noexcept
{
    return _albedo;
}

void GeometryPassMaterial::SetAlbedo ( Texture2DRef const &texture ) noexcept
{
    _albedo = texture;
}

[[maybe_unused]] void GeometryPassMaterial::SetAlbedoDefault () noexcept
{
    _albedo.reset ();
}

Texture2DRef &GeometryPassMaterial::GetEmission () noexcept
{
    return _emission;
}

void GeometryPassMaterial::SetEmission ( Texture2DRef const &texture ) noexcept
{
    _emission = texture;
}

[[maybe_unused]] void GeometryPassMaterial::SetEmissionDefault () noexcept
{
    _emission.reset ();
}

Texture2DRef &GeometryPassMaterial::GetMask () noexcept
{
    return _mask;
}

void GeometryPassMaterial::SetMask ( Texture2DRef const &texture ) noexcept
{
    _mask = texture;
}

[[maybe_unused]] void GeometryPassMaterial::SetMaskDefault () noexcept
{
    _mask.reset ();
}

Texture2DRef &GeometryPassMaterial::GetNormal () noexcept
{
    return _normal;
}

void GeometryPassMaterial::SetNormal ( Texture2DRef const &texture ) noexcept
{
    _normal = texture;
}

[[maybe_unused]] void GeometryPassMaterial::SetNormalDefault () noexcept
{
    _normal.reset ();
}

Texture2DRef &GeometryPassMaterial::GetParam () noexcept
{
    return _param;
}

void GeometryPassMaterial::SetParam ( Texture2DRef const &texture ) noexcept
{
    _param = texture;
}

[[maybe_unused]] void GeometryPassMaterial::SetParamDefault () noexcept
{
    _param = nullptr;
}

bool GeometryPassMaterial::operator < ( GeometryPassMaterial const &other ) const noexcept
{
    constexpr int8_t less = -1;
    constexpr int8_t equal = 0;

    constexpr auto compare = [] ( Texture2DRef const &a, Texture2DRef const &b ) noexcept -> int8_t {
        constexpr int8_t stringCompare = std::numeric_limits<int8_t>::max ();
        constexpr int8_t greater = 1;
        constexpr int8_t lookupTable[ 4U ] = { stringCompare, less, greater, equal };

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

GeometryPassMaterial::GeometryPassMaterial ( eMaterialType type ) noexcept:
    Material ( type )
{
    // NOTHING
}

} // namespace pbr
