#include <precompiled_headers.hpp>
#include <pbr/point_light_component.hpp>
#include <av_assert.hpp>
#include <guid_generator.hpp>


namespace pbr {

namespace {

[[maybe_unused]] constexpr uint32_t POINT_LIGHT_COMPONENT_DESC_FORMAT_VERSION = 2U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

PointLightComponent::PointLightComponent ( PointLightComponentDesc const &desc, uint8_t const* data ) noexcept:
    RenderableComponent ( ClassID::PointLight )
{
    // Sanity checks.
    static_assert ( sizeof ( desc._location ) == sizeof ( GXVec3 ) );
    AV_ASSERT ( desc._formatVersion == POINT_LIGHT_COMPONENT_DESC_FORMAT_VERSION )

    _name = reinterpret_cast<char const*> ( data + desc._name );

    android_vulkan::ColorUnorm const &hue = desc._hue;

    GXColorRGB const unorm ( hue._red, hue._green, hue._blue, 1.0F );
    GXAABB bounds;
    android_vulkan::Vec3 const &low = desc._bounds._min;
    bounds.AddVertex ( low[ 0U ], low[ 1U ], low[ 2U ] );

    android_vulkan::Vec3 const &up = desc._bounds._max;
    bounds.AddVertex ( up[ 0U ], up[ 1U ], up[ 2U ] );

    _pointLight = std::static_pointer_cast<Light> (
        std::make_shared<PointLight> (
            *reinterpret_cast<GXVec3 const*> ( unorm._data ),

            // TODO remove this multiplier in the future.
            desc._intensity * 28.0F,

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
    auto &light = static_cast<PointLight &> ( *_pointLight );
    light.SetBoundDimensions ( width, height, depth );
}

void PointLightComponent::SetBoundDimensions ( GXVec3 const &dimensions ) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto &light = static_cast<PointLight &> ( *_pointLight );
    light.SetBoundDimensions ( dimensions );
}

[[maybe_unused]] void PointLightComponent::SetHue ( GXColorRGB const &hue ) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto &light = static_cast<PointLight &> ( *_pointLight );
    light.SetHue ( hue );
}

void PointLightComponent::SetIntensity ( float intensity ) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto &light = static_cast<PointLight &> ( *_pointLight );
    light.SetIntensity ( intensity );
}

void PointLightComponent::SetLocation ( GXVec3 const &location ) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto &light = static_cast<PointLight &> ( *_pointLight );
    light.SetLocation ( location );
}

ComponentRef &PointLightComponent::GetReference () noexcept
{
    // TODO
    static ComponentRef dummy {};
    return dummy;
}

void PointLightComponent::OnTransform ( GXMat4 const &transformWorld ) noexcept
{
    SetLocation ( *reinterpret_cast<GXVec3 const*> ( transformWorld._data[ 3U ] ) );
}

} // namespace pbr
