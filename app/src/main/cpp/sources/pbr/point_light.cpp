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
    _location ( DEFAULT_LOCATION_X, DEFAULT_LOCATION_Y, DEFAULT_LOCATION_Z )
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

[[maybe_unused]] PointLight::PointLight ( android_vulkan::Half3 const &hue,
    android_vulkan::Half intensity,
    GXVec3 const &location,
    GXAABB const &bounds
) noexcept:
    Light ( eLightType::PointLight ),
    _bounds ( bounds ),
    _hue ( hue ),
    _intensity ( intensity ),
    _location ( location )
{
    // NOTHING
}

[[maybe_unused]] GXAABB const& PointLight::GetBounds () const
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

} // namespace pbr
