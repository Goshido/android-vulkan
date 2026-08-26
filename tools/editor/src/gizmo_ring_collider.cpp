#include <precompiled_headers.hpp>
#include <gizmo_ring_collider.hpp>
#include <ring_math.hpp>
#include <sdf_size.hpp>


namespace editor {

namespace {

constexpr float MISS = std::numeric_limits<float>::max ();
constexpr uint32_t MAX_STEPS = 10U;
constexpr float HIT_SCALE = 3.0F;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

GizmoRingCollider::GizmoRingCollider ( float radius, float thickness ) noexcept:
    _radius ( radius * SDF_PIXEL_SIZE_SCALE ),
    _thickness ( -thickness * SDF_PIXEL_SIZE_SCALE )
{
    // NOTHING
}

float GizmoRingCollider::Raycast ( GXVec3 const &rayOrigin,
    GXVec3 const &rayDirection,
    GXVec3 const &ringLocation,
    GXQuat const &ringRotation,
    GXVec3 const &cameraLocation,
    GXMat3 const &cameraBasis,
    GXVec3 const &vi,
    bool billboard
) const noexcept
{
    GXMat3 basis {};
    GXVec2 sinCosAngle {};

    if ( billboard )
        RingMath::MakeBillboard ( basis, sinCosAngle, cameraBasis );
    else
        RingMath::MakeGeneral ( basis, sinCosAngle, ringRotation, cameraBasis );

    GXVec3 tmp {};
    tmp.Subtract ( ringLocation, cameraLocation );
    float const s = vi.DotProduct ( tmp );
    float const radius = s * _radius;
    float const thickness = s * _thickness;
    float const rt = radius + thickness;

    // x - current distance from SDF
    // y - maximum allowed distance
    GXVec2 alpha ( 0.0F, radius + radius + tmp.Length () );

    // x - adjustable minimal distance to consider ray vs SDF hit
    // y - ray distance has traveled
    GXVec2 beta = GXVec2::ZERO;

    // Note it's needed to use inverse standard basis. So using matrix/vector multiplication in reverse order trick.
    tmp.Subtract ( rayOrigin, ringLocation );
    GXVec3 ro {};
    basis.MultiplyMatrixVector ( ro, tmp );

    GXVec3 rd {};
    basis.MultiplyMatrixVector ( rd, rayDirection );

    GXVec3 viSDF {};
    basis.MultiplyMatrixVector ( viSDF, vi );

    float const dynamicThresholdFactor = HIT_SCALE * rd.DotProduct ( viSDF );

    bool isMiss = false;
    uint32_t steps = 0U;

    for ( ; steps < MAX_STEPS; ++steps )
    {
        tmp.Sum ( ro, beta._data[ 1U ], rd );
        alpha._data[ 0U ] = SDF ( tmp, sinCosAngle, rt, thickness );
        beta._data[ 1U ] += alpha._data[ 0U ];
        beta._data[ 0U ] = beta._data[ 1U ] * dynamicThresholdFactor;
        bool const isHit = alpha._data[ 0U ] < beta._data[ 0U ];
        isMiss = alpha._data[ 1U ] < beta._data[ 1U ];

        if ( isHit | isMiss )
        {
            break;
        }
    }

    float const cases[] = { beta._data[ 1U ], MISS };
    return cases[ static_cast<size_t> ( isMiss | ( steps >= MAX_STEPS ) ) ];
}

float GizmoRingCollider::GetRadius () const noexcept
{
    return _radius;
}

// Idea is taken from https://iquilezles.org/articles/distfunctions/
// [2026/03/16] 'sinCosAngle' accepts angles in range from 0 to pi.
// 0 - ring is single point. pi - complete ring.
// Ring is aligned to XY plane.
// Ring center is located in origin.
// The grow point is located on Y axis, positive direction: (0, radius, 0)
float GizmoRingCollider::SDF ( GXVec3 p, GXVec2 const &sinCosAngle, float radius, float thickness ) noexcept
{
    p._data[ 0U ] = std::abs ( p._data[ 0U ] );

    GXVec2 a ( radius, radius );
    a.Multiply ( a, GXVec2 ( radius, -2.0F ) );

    GXVec2 b = sinCosAngle;
    b.Multiply ( b, GXVec2 ( p._data[ 1U ], p._data[ 0U ] ) );

    float const c = p.DotProduct ( p );

    auto const &p2D = *reinterpret_cast<GXVec2 const*> ( &p );
    float const cases[] = { p2D.Length (), p2D.DotProduct ( sinCosAngle ) };
    float const d = cases[ static_cast<size_t> ( b._data[ 0U ] < b._data[ 1U ] ) ];

    return thickness + std::sqrt ( c + d * a._data[ 1U ] + a._data[ 0U ] );
}

} // namespace editor
