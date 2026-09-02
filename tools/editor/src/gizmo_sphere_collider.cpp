#include <precompiled_headers.hpp>
#include <gizmo_sphere_collider.hpp>
#include <sdf_size.hpp>


namespace editor {

namespace {

constexpr float MISS = std::numeric_limits<float>::max ();

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

GizmoSphereCollider::GizmoSphereCollider ( float radius ) noexcept:
    _radius ( radius * SDF_PIXEL_SIZE_SCALE )
{
    // NOTHING
}

float GizmoSphereCollider::Raycast ( GXVec3 const &rayDirection,
    GXVec3 const &sphereLocation,
    GXVec3 const &cameraLocation,
    GXVec3 const &vi
) const noexcept
{
    GXVec3 alpha {};
    alpha.Subtract ( sphereLocation, cameraLocation );
    float const r = _radius * vi.DotProduct ( alpha );
    float const tca = alpha.DotProduct ( rayDirection );
    float const d2 = alpha.DotProduct ( alpha ) - tca * tca;
    float const r2 = r * r;

    if ( ( tca < 0.0F ) | ( d2 > r2 ) )
        return MISS;

    float const thc = std::sqrt ( r2 - d2 );
    GXVec2 tRay ( tca, tca );
    tRay.Sum ( tRay, GXVec2 ( -thc, thc ) );

    float const cases[] = { tRay._data[ 0U ], MISS };
    tRay._data[ 0U ] = cases[ static_cast<size_t> ( ( tRay._data[ 0U ] < 0.0F ) | ( tRay._data[ 1U ] < 0.0F ) ) ];
    return tRay._data[ static_cast<size_t> ( tRay._data[ 0U ] >= tRay._data[ 1U ] ) ];
}

} // namespace editor
