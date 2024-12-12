#include <precompiled_headers.hpp>
#include <shape_sphere.hpp>


namespace android_vulkan {

[[maybe_unused]] ShapeSphere::ShapeSphere ( float radius ) noexcept:
    Shape ( eShapeType::Sphere ),
    _radius ( radius )
{
    GXVec3 r ( radius, radius, radius );
    _boundsLocal.AddVertex ( r );

    r.Reverse ();
    _boundsLocal.AddVertex ( r );

    _boundsWorld = _boundsLocal;
}

void ShapeSphere::CalculateInertiaTensor ( float mass ) noexcept
{
    // https://en.wikipedia.org/wiki/List_of_moments_of_inertia
    float const diagonal = 1.0F / ( 0.4F * mass * _radius * _radius );
    auto &m = _inertiaTensorInverse._data;

    m[ 0U ][ 0U ] = diagonal;
    m[ 1U ][ 1U ] = diagonal;
    m[ 2U ][ 2U ] = diagonal;

    m[ 0U ][ 1U ] = 0.0F;
    m[ 0U ][ 2U ] = 0.0F;
    m[ 1U ][ 0U ] = 0.0F;
    m[ 1U ][ 2U ] = 0.0F;
    m[ 2U ][ 0U ] = 0.0F;
    m[ 2U ][ 1U ] = 0.0F;
}

GXVec3 ShapeSphere::GetExtremePointWorld ( GXVec3 const &direction ) const noexcept
{
    GXVec3 location {};
    _transformWorld.GetW ( location );

    GXVec3 dir ( direction );
    dir.Normalize ();

    GXVec3 result {};
    result.Sum ( location, _radius, dir );

    return result;
}

[[maybe_unused]] float ShapeSphere::GetRadius () const noexcept
{
    return _radius;
}

void ShapeSphere::UpdateBounds () noexcept
{
    GXVec3 location {};
    _transformWorld.GetW ( location );

    _boundsWorld._min.Sum ( _boundsLocal._min, location );
    _boundsWorld._max.Sum ( _boundsLocal._max, location );
}

} // namespace android_vulkan
