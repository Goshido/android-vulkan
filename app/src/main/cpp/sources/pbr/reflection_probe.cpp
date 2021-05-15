#include <pbr/reflection_probe.h>

namespace pbr {

void ReflectionProbe::FreeTransferResources ( VkDevice device )
{
    _prefilter->FreeTransferResources ( device );
}

TextureCubeRef& ReflectionProbe::GetPrefilter ()
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
