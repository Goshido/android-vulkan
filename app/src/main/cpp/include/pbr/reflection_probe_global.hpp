#ifndef PBR_REFLECTION_PROBE_GLOBAL_HPP
#define PBR_REFLECTION_PROBE_GLOBAL_HPP


#include "reflection_probe.hpp"


namespace pbr {

class ReflectionProbeGlobal final : public ReflectionProbe
{
    public:
        ReflectionProbeGlobal () = delete;

        ReflectionProbeGlobal ( ReflectionProbeGlobal const & ) = delete;
        ReflectionProbeGlobal &operator = ( ReflectionProbeGlobal const & ) = delete;

        ReflectionProbeGlobal ( ReflectionProbeGlobal && ) = delete;
        ReflectionProbeGlobal &operator = ( ReflectionProbeGlobal && ) = delete;

        explicit ReflectionProbeGlobal ( TextureCubeRef prefilter ) noexcept;

        ~ReflectionProbeGlobal () override = default;
};

} // namespace pbr


#endif // PBR_REFLECTION_PROBE_GLOBAL_HPP

