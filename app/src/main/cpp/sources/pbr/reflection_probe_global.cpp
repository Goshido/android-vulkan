#include <pbr/reflection_probe_global.hpp>


namespace pbr {

ReflectionProbeGlobal::ReflectionProbeGlobal ( TextureCubeRef prefilter ) noexcept:
    ReflectionProbe ( eLightType::ReflectionGlobal, prefilter )
{
    // NOTHING
}

} // namespace pbr
