#include <pbr/reflection_probe_global.h>


namespace pbr {

ReflectionProbeGlobal::ReflectionProbeGlobal ( TextureCubeRef prefilter ) noexcept:
    ReflectionProbe ( eLightType::ReflectionGlobal, prefilter )
{
    // NOTHING
}

} // namespace pbr
