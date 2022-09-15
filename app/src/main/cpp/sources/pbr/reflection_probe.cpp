#include <pbr/reflection_probe.h>

namespace pbr {

void ReflectionProbe::FreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept
{
    _prefilter->FreeTransferResources ( renderer );
}

TextureCubeRef& ReflectionProbe::GetPrefilter () noexcept
{
    return _prefilter;
}

ReflectionProbe::ReflectionProbe ( eLightType type, TextureCubeRef &prefilter ) noexcept:
    Light ( type ),
    _prefilter ( prefilter )
{
    // NOTHING
}

} // namespace pbr
