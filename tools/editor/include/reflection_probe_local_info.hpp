#ifndef EDITOR_REFLECTION_PROBE_LOCAL_INFO_HPP
#define EDITOR_REFLECTION_PROBE_LOCAL_INFO_HPP


#include <GXCommon/GXMath.hpp>


namespace editor {

struct ReflectionProbeLocalInfo final
{
    GXAABB      _bounds {};
    float       _intensity = 1.0F;
    float       _radius = 7.0F;
};

} // namespace editor


#endif // EDITOR_REFLECTION_PROBE_LOCAL_INFO_HPP
