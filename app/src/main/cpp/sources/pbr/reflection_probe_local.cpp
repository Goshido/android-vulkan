#include <pbr/reflection_probe_local.h>


namespace pbr {

ReflectionProbeLocal::ReflectionProbeLocal ( TextureCubeRef prefilter, GXVec3 location, float size ) noexcept:
    ReflectionProbe ( eLightType::ReflectionLocal, prefilter ),
    _location ( location ),
    _size ( size )
{
    UpdateBounds ();
}

GXAABB const &ReflectionProbeLocal::GetBounds () const noexcept
{
    return _bounds;
}

GXVec3 const &ReflectionProbeLocal::GetLocation () const noexcept
{
    return _location;
}

void ReflectionProbeLocal::SetLocation ( GXVec3 const &location ) noexcept
{
    _location = location;
    UpdateBounds ();
}

float ReflectionProbeLocal::GetSize () const noexcept
{
    return _size;
}

void ReflectionProbeLocal::UpdateBounds () noexcept
{
    float const alpha = _size * 0.5F;
    GXVec3 const beta ( alpha, alpha, alpha );

    GXVec3 gamma {};
    gamma.Subtract ( _location, beta );

    _bounds.Empty ();
    _bounds.AddVertex ( gamma );

    gamma.Sum ( _location, beta );
    _bounds.AddVertex ( gamma );
}

} // namespace pbr
