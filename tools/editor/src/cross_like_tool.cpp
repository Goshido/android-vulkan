#include <precompiled_headers.hpp>
#include <cross_like_tool.hpp>


namespace editor {

namespace {

constexpr float             COLINEAR_THRESHOLD = 1.0e-4F;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

bool CrossLikeTool::FlipTest ( SDFBoxWithFlip const &plane, GXVec3 const &cameraLocation ) noexcept
{
    GXVec3 alpha {};
    alpha.Subtract ( plane.GetLocationWorld (), cameraLocation );

    GXVec3 beta {};
    plane.GetRotationWorld ().GetRight ( beta );

    return beta.DotProduct ( alpha ) > 0.0F;
}

void CrossLikeTool::HidePlane ( SDFBoxWithFlip &plane,
    SDFLineSegmentWithFlip &lineA,
    SDFLineSegmentWithFlip &lineB
) noexcept
{
    plane.Hide ();
    lineA.Hide ();
    lineB.Hide ();
}

std::optional<float> CrossLikeTool::ResolveAxisScalarDistance ( GXVec3 const &axisOrigin,
    GXVec3 const &axisDirection,
    GXVec3 const &cameraLocation,
    GXVec3 const &cameraForward
) noexcept
{
    // See <repo>/docs/gizmo-rendering.md#inter-axis
    if ( 1.0F - std::abs ( cameraForward.DotProduct ( axisDirection ) ) < COLINEAR_THRESHOLD )
        return std::nullopt;

    GXVec3 alpha {};
    alpha.CrossProduct ( axisDirection, cameraForward );

    GXVec3 omega {};
    omega.CrossProduct ( cameraForward, alpha );

    alpha.Subtract ( cameraLocation, axisOrigin );
    return std::optional<float> { alpha.DotProduct ( omega ) / axisDirection.DotProduct ( omega ) };
}

std::optional<GXVec3> CrossLikeTool::ResolvePlaneIntersection ( GXVec3 const &planeOrigin,
    GXVec3 const &planeNormal,
    GXVec3 const &cameraLocation,
    GXVec3 const &cameraForward
) noexcept
{
    // See <repo>/docs/gizmo-rendering.md#inter-plane
    float const n = planeNormal.DotProduct ( cameraForward );

    if ( 1.0F - std::abs ( n ) < COLINEAR_THRESHOLD )
        return std::nullopt;

    GXVec3 alpha {};
    alpha.Subtract ( planeOrigin, cameraLocation );
    alpha.Sum ( cameraLocation, alpha.DotProduct ( planeNormal ) / n, cameraForward );
    return std::optional<GXVec3> { std::move ( alpha ) };
}

} // namespace editor
