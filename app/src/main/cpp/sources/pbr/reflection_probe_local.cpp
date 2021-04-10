#include <pbr/reflection_probe_local.h>


namespace pbr {

ReflectionProbeLocal::ReflectionProbeLocal ( TextureCubeRef prefilter, GXVec3 location, float size ) noexcept:
    ReflectionProbe ( eLightType::ReflectionLocal, prefilter ),
    _location ( location ),
    _size ( size )
{
    float const alpha = _size * 0.5F;
    GXVec3 const beta ( alpha, alpha, alpha );

    GXVec3 gamma;
    gamma.Substract ( _location, beta );
    _bounds.AddVertex ( gamma );

    gamma.Sum ( _location, beta );
    _bounds.AddVertex ( gamma );
}

GXAABB const& ReflectionProbeLocal::GetBounds () const
{
    return _bounds;
}

GXVec3 const& ReflectionProbeLocal::GetLocation () const
{
    return _location;
}

float ReflectionProbeLocal::GetSize () const
{
    return _size;
}

} // namespace pbr
