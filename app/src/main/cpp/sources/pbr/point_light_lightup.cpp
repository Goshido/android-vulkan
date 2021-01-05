#include <pbr/point_light_lightup.h>


namespace pbr {

PointLightLightup::PointLightLightup () noexcept
{
    // NOTHING
}

[[maybe_unused]] bool PointLightLightup::Execute ( LightVolume &/*lightVolume*/,
    android_vulkan::Renderer &/*renderer*/
)
{
    // TODO
    return false;
}

[[maybe_unused]] bool PointLightLightup::Init ( android_vulkan::Renderer &/*renderer*/ )
{
    // TODO
    return false;
}

[[maybe_unused]] void PointLightLightup::Destroy ( android_vulkan::Renderer &/*renderer*/ )
{
    // TODO
}

} // namespace pbr
