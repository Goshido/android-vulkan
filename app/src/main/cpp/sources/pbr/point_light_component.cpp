#include <pbr/point_light_component.h>
#include <pbr/point_light_component_desc.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace pbr {

[[maybe_unused]] constexpr static uint32_t const POINT_LIGHT_COMPONENT_DESC_FORMAT_VERSION = 1U;

PointLightComponent::PointLightComponent ( PointLightComponentDesc const &desc ) noexcept:
    Component ( ClassID::PointLight ),
    _pointLight {}
{
    // Sanity checks.
    static_assert ( sizeof ( desc._location ) == sizeof ( GXVec3 ) );
    assert ( desc._formatVersion == POINT_LIGHT_COMPONENT_DESC_FORMAT_VERSION );

    android_vulkan::ColorUnorm const& hue = desc._hue;

    GXColorRGB const unorm ( static_cast<GXUByte> ( hue._red ),
        static_cast<GXUByte> ( hue._green ),
        static_cast<GXUByte> ( hue._blue ),
        static_cast<GXUByte> ( 255U )
    );

    GXAABB bounds;
    android_vulkan::Vec3 const &low = desc._bounds._min;
    bounds.AddVertex ( low[ 0U ], low[ 1U ], low[ 2U ] );

    android_vulkan::Vec3 const &up = desc._bounds._max;
    bounds.AddVertex ( up[ 0U ], up[ 1U ], up[ 2U ] );

    _pointLight = std::static_pointer_cast<Light> (
        std::make_shared<PointLight> (
            android_vulkan::Half3 ( unorm._data[ 0U ], unorm._data[ 1U ], unorm._data[ 2U ] ),

            // TODO remove this multiplier in the future.
            android_vulkan::Half ( desc._intensity * 14.1414F ),

            reinterpret_cast<GXVec3 const &> ( desc._location ),
            bounds
        )
    );
}

PointLightComponent::PointLightComponent () noexcept:
    Component ( ClassID::PointLight )
{
    _pointLight = std::make_shared<PointLight> ();
}

void PointLightComponent::Submit ( RenderSession &renderSession )
{
    renderSession.SubmitLight ( _pointLight );
}

void PointLightComponent::SetBoundDimensions ( float width, float height, float depth ) noexcept
{
    // NOLINTNEXTLINE
    auto& light = *static_cast<PointLight*> ( _pointLight.get () );
    light.SetBoundDimensions ( width, height, depth );
}

[[maybe_unused]] void PointLightComponent::SetHue ( GXColorRGB const &hue ) noexcept
{
    // NOLINTNEXTLINE
    auto& light = *static_cast<PointLight*> ( _pointLight.get () );
    light.SetHue ( hue );
}

void PointLightComponent::SetIntensity ( float intensity ) noexcept
{
    // NOLINTNEXTLINE
    auto& light = *static_cast<PointLight*> ( _pointLight.get () );
    light.SetIntensity ( intensity );
}

void PointLightComponent::SetLocation ( GXVec3 const &location ) noexcept
{
    // NOLINTNEXTLINE
    auto& light = *static_cast<PointLight*> ( _pointLight.get () );
    light.SetLocation ( location );
}

void PointLightComponent::FreeTransferResources ( VkDevice /*device*/ )
{
    // NOTHING
}

} // namespace pbr
