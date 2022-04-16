#include <pbr/point_light_component.h>
#include <pbr/point_light_component_desc.h>
#include <guid_generator.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace pbr {

[[maybe_unused]] constexpr static uint32_t const POINT_LIGHT_COMPONENT_DESC_FORMAT_VERSION = 1U;

//----------------------------------------------------------------------------------------------------------------------

PointLightComponent::PointLightComponent ( PointLightComponentDesc const &desc ) noexcept:
    RenderableComponent ( ClassID::PointLight, android_vulkan::GUID::GenerateAsString ( "PointLight" ) )
{
    // Sanity checks.
    static_assert ( sizeof ( desc._location ) == sizeof ( GXVec3 ) );
    assert ( desc._formatVersion == POINT_LIGHT_COMPONENT_DESC_FORMAT_VERSION );

    android_vulkan::ColorUnorm const& hue = desc._hue;

    GXColorRGB const unorm ( hue._red, hue._green, hue._blue, 255U );
    GXAABB bounds;
    android_vulkan::Vec3 const &low = desc._bounds._min;
    bounds.AddVertex ( low[ 0U ], low[ 1U ], low[ 2U ] );

    android_vulkan::Vec3 const &up = desc._bounds._max;
    bounds.AddVertex ( up[ 0U ], up[ 1U ], up[ 2U ] );

    _pointLight = std::static_pointer_cast<Light> (
        std::make_shared<PointLight> (
            *reinterpret_cast<GXVec3 const*> ( unorm._data ),

            // TODO remove this multiplier in the future.
            desc._intensity * 14.1414F,

            reinterpret_cast<GXVec3 const &> ( desc._location ),
            bounds
        )
    );
}

PointLightComponent::PointLightComponent () noexcept:
    RenderableComponent ( ClassID::PointLight, android_vulkan::GUID::GenerateAsString ( "PointLight" ) ),
    _pointLight ( std::make_shared<PointLight> () )
{
    // NOTHING
}

void PointLightComponent::Submit ( RenderSession &renderSession ) noexcept
{
    renderSession.SubmitLight ( _pointLight );
}

void PointLightComponent::SetBoundDimensions ( float width, float height, float depth ) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto& light = static_cast<PointLight&> ( *_pointLight );
    light.SetBoundDimensions ( width, height, depth );
}

void PointLightComponent::SetBoundDimensions ( GXVec3 const &dimensions ) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto& light = static_cast<PointLight&> ( *_pointLight );
    light.SetBoundDimensions ( dimensions );
}

[[maybe_unused]] void PointLightComponent::SetHue ( GXColorRGB const &hue ) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto& light = static_cast<PointLight&> ( *_pointLight );
    light.SetHue ( hue );
}

void PointLightComponent::SetIntensity ( float intensity ) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto& light = static_cast<PointLight&> ( *_pointLight );
    light.SetIntensity ( intensity );
}

void PointLightComponent::SetLocation ( GXVec3 const &location ) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto& light = static_cast<PointLight&> ( *_pointLight );
    light.SetLocation ( location );
}

} // namespace pbr
