#include <precompiled_headers.hpp>
#include <gizmo_cylinder_collider.hpp>


namespace editor {

namespace {

constexpr float MISS = std::numeric_limits<float>::max ();
constexpr float EPSILON = 10.0F * std::numeric_limits<float>::denorm_min ();

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

GizmoCylinderCollider::GizmoCylinderCollider ( float radius, float length ) noexcept:
    _w ( radius * radius ),
    _u ( length )
{
    // NOTHING
}

float GizmoCylinderCollider::Raycast ( GXVec3 const &rayOrigin,
    GXVec3 const &rayDirection,
    GXVec3 cylinderCapLocation,
    GXVec3 const &cylinderAxis
) const noexcept
{
    // See <repo>/docs/ray-cylinder-intersection.md
    GXVec3 x {};
    x.Subtract ( rayOrigin, cylinderCapLocation );
    float const delta = rayDirection.DotProduct ( cylinderAxis );
    float const beta = x.DotProduct ( cylinderAxis );

    GXVec3 i {};
    i.Sum ( rayDirection, -delta, cylinderAxis );
    float const a = i.DotProduct ( i );

    if ( std::abs ( a ) <= EPSILON ) [[unlikely]]
        return MISS;

    float cases[] = { MISS, 0.0F };
    float &t = cases[ 0U ];
    float &tmp = cases[ 1U ];

    GXVec3 j {};
    j.Sum ( x, -beta, cylinderAxis );
    float const k = i.DotProduct ( j );
    float const c = j.DotProduct ( j ) - _w;
    float const d1 = k * k - a * c;

    if ( d1 >= 0.0F )
    {
        float const iota = std::sqrt ( d1 );
        float const zeta = -k;
        float const gamma = 1.0F / a;
        tmp = ( zeta + iota ) * gamma;

        float m = beta + tmp * delta;
        t = cases[ static_cast<size_t> ( ( m >= 0.0F ) & ( m <= _u ) ) ];

        tmp = ( zeta - iota ) * gamma;
        m = beta + tmp * delta;
        t = cases[ static_cast<size_t> ( ( tmp < t ) & ( m >= 0.0F ) & ( m <= _u ) ) ];
    }

    float const gamma = 1.0F / delta;
    tmp = -beta * gamma;

    GXVec3 f {};
    f.Sum ( x, tmp, rayDirection );
    t = cases[ static_cast<size_t> ( ( tmp < t ) & ( f.DotProduct ( f ) < _w ) ) ];

    cylinderCapLocation.Sum ( cylinderCapLocation, _u, cylinderAxis );
    x.Subtract ( rayOrigin, cylinderCapLocation );
    tmp = -gamma * x.DotProduct ( cylinderAxis );
    f.Sum ( x, tmp, rayDirection );
    return cases[ static_cast<size_t> ( ( tmp < t ) & ( f.DotProduct ( f ) < _w ) ) ];
}

} // namespace editor
