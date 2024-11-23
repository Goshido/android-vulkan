#include <precompiled_headers.hpp>
#include <cyrus_beck.hpp>


namespace android_vulkan {

namespace {

constexpr float COLLINEAR_TOLERANCE = 5.0e-4F;
constexpr float SAME_POINT_TOLERANCE = 1.0e-3F;

} // end of anonymous namespace

CyrusBeck::CyrusBeck () noexcept:
    _vertices {}
{
    _vertices.reserve ( 2U );
}

Vertices const &CyrusBeck::Run ( Vertices const &face,
    GXVec3 const &faceNormal,
    Vertices const &edge,
    GXVec3 const &edgeDir
) noexcept
{
    size_t const facePoints = face.size ();
    GXVec3 const &edgeOrigin = edge[ 0U ];
    GXVec3 edgeUnitVector ( edgeDir );
    edgeUnitVector.Normalize ();

    GXVec3 normal {};
    GXVec3 alpha {};

    float start = 0.0F;
    float end = 1.0F;

    for ( size_t i = 0U; i < facePoints; ++i )
    {
        GXVec3 const &aPoint = face[ i ];
        alpha.Subtract ( face[ ( i + 1U ) % facePoints ], aPoint );

        // The normal should point away from the face inner area. So changing cross product order.
        normal.CrossProduct ( alpha, faceNormal );
        normal.Normalize ();

        float const beta = normal.DotProduct ( edgeUnitVector );

        if ( std::abs ( beta ) < COLLINEAR_TOLERANCE )
        {
            // Edge is parallel to face normal. We could safely ignore it because there must be another edge face
            // combinations which will produce start and end points. It's working because we already know that there is
            // at least one intersection point so far.
            continue;
        }

        // Optimization: The operands have been swapped to eliminate minus in denominator later.
        alpha.Subtract ( aPoint, edgeOrigin );
        float const gamma = normal.DotProduct ( edgeDir );
        float const p = normal.DotProduct ( alpha ) / gamma;

        // Note the "p" is an intersection point of two parametric lines. Not the line segments of face and edge!
        // So the "p" itself can be in range [-inf, +inf]. From other hand by design "p" must be clamped
        // to [0.0, 1.0] range. The question is which point to clamp? Start point or end point? Fortunately
        // the "gamma" parameter has handy property which solves selection problem. The rules are:
        //      "gamma" < 0.0: Need to work with the start point and take maximum in range [0.0, p]
        //      "gamma" >= 0.0: Need to work with the end point and take minimum in range [p, 1.0]

        if ( gamma < 0.0F )
        {
            start = std::max ( start, p );
            continue;
        }

        end = std::min ( end, p );
    }

    _vertices.clear ();
    alpha.Sum ( edgeOrigin, start, edgeDir );
    _vertices.push_back ( alpha );

    if ( end - start < SAME_POINT_TOLERANCE )
    {
        // The end points of intersection segment is very close to each other. So consider this case
        // as one point intersection.
        return _vertices;
    }

    alpha.Sum ( edgeOrigin, end, edgeDir );
    _vertices.push_back ( alpha );
    return _vertices;
}

} // namespace android_vulkan
