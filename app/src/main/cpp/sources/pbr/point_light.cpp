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

constexpr static float const Z_NEAR = 0.05F;

//----------------------------------------------------------------------------------------------------------------------

[[nodiscard]] constexpr static float ToUnorm ( uint8_t channel )
{
    constexpr float const factor = 1.0F / 255.0F;
    return factor * static_cast<float> ( channel );
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] PointLight::PointLight () noexcept:
    Light ( eLightType::PointLight ),
    _bounds {},
    _hue ( ToUnorm ( DEFAULT_HUE_RED ), ToUnorm ( DEFAULT_HUE_GREEN ), ToUnorm ( DEFAULT_HUE_BLUE ) ),
    _intensity ( android_vulkan::Half::Convert ( DEFAULT_INTENSITY ) ),
    _isNeedUpdateMatrices ( true ),
    _location ( DEFAULT_LOCATION_X, DEFAULT_LOCATION_Y, DEFAULT_LOCATION_Z ),
    _matrices {}
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
    _hue ( hue ),
    _intensity ( intensity ),
    _isNeedUpdateMatrices ( true ),
    _location ( location ),
    _matrices {}
{
    // NOTHING
}

GXAABB const& PointLight::GetBounds () const
{
    return _bounds;
}

[[maybe_unused]] android_vulkan::Half3 const& PointLight::GetHue () const
{
    return _hue;
}

[[maybe_unused]] android_vulkan::Half PointLight::GetIntensity () const
{
    return _intensity;
}

[[maybe_unused]] GXVec3 const& PointLight::GetLocation () const
{
    return _location;
}

PointLight::Matrices const& PointLight::GetMatrices ()
{
    if ( _isNeedUpdateMatrices )
        UpdateMatrices ();

    return _matrices;
}

void PointLight::UpdateMatrices ()
{
    GXMat4 projection;

    projection.Perspective ( GX_MATH_HALF_PI,
        1.0F,
        Z_NEAR,
        std::max ( _bounds.GetWidth (), std::max ( _bounds.GetHeight (), _bounds.GetDepth () ) )
    );

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
        _matrices[ i ].Multiply ( view, projection );
    }

    _isNeedUpdateMatrices = false;
}

} // namespace pbr
