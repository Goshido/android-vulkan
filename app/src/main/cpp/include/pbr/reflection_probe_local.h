#ifndef PBR_REFLECTION_PROBE_LOCAL_H
#define PBR_REFLECTION_PROBE_LOCAL_H


#include "reflection_probe.h"


namespace pbr {

class ReflectionProbeLocal final : public ReflectionProbe
{
    private:
        GXVec3      _location;
        float       _size;

        GXAABB      _bounds;

    public:
        ReflectionProbeLocal () = delete;

        ReflectionProbeLocal ( ReflectionProbeLocal const & ) = delete;
        ReflectionProbeLocal& operator = ( ReflectionProbeLocal const & ) = delete;

        ReflectionProbeLocal ( ReflectionProbeLocal && ) = delete;
        ReflectionProbeLocal& operator = ( ReflectionProbeLocal && ) = delete;

        explicit ReflectionProbeLocal ( TextureCubeRef prefilter, GXVec3 location, float size ) noexcept;

        ~ReflectionProbeLocal () override = default;

        [[nodiscard]] GXAABB const& GetBounds () const;
        [[nodiscard]] GXVec3 const& GetLocation () const;
        [[nodiscard]] float GetSize () const;
};

} // namespace pbr


#endif // PBR_REFLECTION_PROBE_LOCAL_H
