#include <pbr/point_light.h>
#include <pbr/point_light_component_desc.h>


namespace pbr {

constexpr static float const DEFAULT_INTENSITY = 1.0F;

constexpr static uint8_t const DEFAULT_HUE_RED = 255U;
constexpr static uint8_t const DEFAULT_HUE_GREEN = 255U;
constexpr static uint8_t const DEFAULT_HUE_BLUE = 255U;

constexpr static float const DEFAULT_LOCATION_X = 0.0F;
constexpr static float const DEFAULT_LOCATION_Y = 0.0F;
constexpr static float const DEFAULT_LOCATION_Z = 0.0F;

constexpr static float const DEFAULT_SIZE = 1.0F;
constexpr static float const HALF_DEFAULT_SIZE = 0.5F * DEFAULT_SIZE;

constexpr static float const Z_NEAR = 0.05F;

//----------------------------------------------------------------------------------------------------------------------

[[nodiscard]] constexpr static float ToUnorm ( uint8_t channel )
{
    constexpr float const factor = 1.0F / 255.0F;
    return factor * static_cast<float> ( channel );
}

//----------------------------------------------------------------------------------------------------------------------

PointLight::PointLight () noexcept:
    Light ( eLightType::PointLight ),
    _bounds {},
    _dimensions ( HALF_DEFAULT_SIZE, HALF_DEFAULT_SIZE, HALF_DEFAULT_SIZE ),
    _hue ( ToUnorm ( DEFAULT_HUE_RED ), ToUnorm ( DEFAULT_HUE_GREEN ), ToUnorm ( DEFAULT_HUE_BLUE ) ),
    _intensity ( android_vulkan::Half::Convert ( DEFAULT_INTENSITY ) ),
    _isNeedUpdate ( true ),
    _location ( DEFAULT_LOCATION_X, DEFAULT_LOCATION_Y, DEFAULT_LOCATION_Z ),
    _matrices {},
    _projection {}
{
    constexpr float const halfSize = 0.5F * DEFAULT_SIZE;

    constexpr GXVec3 const minimum ( DEFAULT_LOCATION_X - halfSize,
        DEFAULT_LOCATION_X - halfSize,
        DEFAULT_LOCATION_Z - halfSize
    );

    constexpr GXVec3 const maximum ( DEFAULT_LOCATION_X + halfSize,
        DEFAULT_LOCATION_X + halfSize,
        DEFAULT_LOCATION_Z + halfSize
    );

    _bounds.AddVertex ( minimum );
    _bounds.AddVertex ( maximum );
}

PointLight::PointLight ( android_vulkan::Half3 const &hue,
    android_vulkan::Half intensity,
    GXVec3 const &location,
    GXAABB const &bounds
) noexcept:
    Light ( eLightType::PointLight ),
    _bounds ( bounds ),
    _dimensions ( 0.5F * bounds.GetWidth (), 0.5F * bounds.GetHeight (), 0.5F * bounds.GetDepth () ),
    _hue ( hue ),
    _intensity ( intensity ),
    _isNeedUpdate ( true ),
    _location ( location ),
    _matrices {},
    _projection {}
{
    // NOTHING
}

GXAABB const& PointLight::GetBounds () const noexcept
{
    return _bounds;
}

[[maybe_unused]] void PointLight::SetBoundDimensions ( float width, float height, float depth ) noexcept
{
    _dimensions._data[ 0U ] = width;
    _dimensions._data[ 1U ] = height;
    _dimensions._data[ 2U ] = depth;
    _dimensions.Multiply ( _dimensions, 0.5F );
    _isNeedUpdate = true;
}

android_vulkan::Half3 const& PointLight::GetHue () const noexcept
{
    return _hue;
}

[[maybe_unused]] void PointLight::SetHue ( GXColorRGB const &hue ) noexcept
{
    float const* rgb = hue._data;
    _hue._data[ 0U ] = android_vulkan::Half::Convert ( rgb[ 0U ] );
    _hue._data[ 1U ] = android_vulkan::Half::Convert ( rgb[ 1U ] );
    _hue._data[ 2U ] = android_vulkan::Half::Convert ( rgb[ 2U ] );
}

android_vulkan::Half PointLight::GetIntensity () const noexcept
{
    return _intensity;
}

[[maybe_unused]] void PointLight::SetIntensity ( float intensity ) noexcept
{
    _intensity = intensity;
}

GXVec3 const& PointLight::GetLocation () const noexcept
{
    return _location;
}

void PointLight::SetLocation ( GXVec3 const &location ) noexcept
{
    _location = location;
    _isNeedUpdate = true;
}

PointLight::Matrices const& PointLight::GetMatrices () noexcept
{
    if ( _isNeedUpdate )
        UpdateMatrices ();

    return _matrices;
}

GXMat4 const& PointLight::GetProjection () noexcept
{
    if ( _isNeedUpdate )
        UpdateMatrices ();

    return _projection;
}

void PointLight::UpdateMatrices () noexcept
{
    _projection.Perspective ( GX_MATH_HALF_PI,
        1.0F,
        Z_NEAR,
        std::max ( _bounds.GetWidth (), std::max ( _bounds.GetHeight (), _bounds.GetDepth () ) )
    );

    _bounds.Empty ();

    GXVec3 alpha;
    alpha.Sum ( _location, _dimensions );
    _bounds.AddVertex ( alpha );

    alpha.Subtract ( _location, _dimensions );
    _bounds.AddVertex ( alpha );

    GXMat4 locals[ PBR_POINT_LIGHT_SHADOW_CASTER_PROJECTION_COUNT ];
    locals[ static_cast<size_t> ( eFaceIndex::PositiveX ) ].RotationY ( GX_MATH_HALF_PI );
    locals[ static_cast<size_t> ( eFaceIndex::NegativeX ) ].RotationY ( -GX_MATH_HALF_PI );
    locals[ static_cast<size_t> ( eFaceIndex::PositiveY ) ].RotationX ( -GX_MATH_HALF_PI );
    locals[ static_cast<size_t> ( eFaceIndex::NegativeY ) ].RotationX ( GX_MATH_HALF_PI );
    locals[ static_cast<size_t> ( eFaceIndex::PositiveZ ) ].Identity ();
    locals[ static_cast<size_t> ( eFaceIndex::NegativeZ ) ].RotationY ( GX_MATH_PI );

    GXMat4 view;

    for ( size_t i = 0U; i < PBR_POINT_LIGHT_SHADOW_CASTER_PROJECTION_COUNT; ++i )
    {
        GXMat4 &local = locals[ i ];
        local.SetW ( _location );
        view.Inverse ( local );
        _matrices[ i ].Multiply ( view, _projection );
    }

    _isNeedUpdate = false;
}

} // namespace pbr
