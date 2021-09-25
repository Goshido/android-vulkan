#include <sutherland_hodgman.h>
#include <logger.h>


namespace android_vulkan {

constexpr static size_t const DEFAULT_CAPACITY = 16U;

SutherlandHodgman::SutherlandHodgman () noexcept
{
    _clipPoints.reserve ( DEFAULT_CAPACITY );
    _result.reserve ( DEFAULT_CAPACITY );
    _windowPoints.reserve ( DEFAULT_CAPACITY );
    _workingPoints.reserve ( DEFAULT_CAPACITY );
}

SutherlandHodgmanResult const& SutherlandHodgman::Run ( Vertices const &shapeAPoints,
    GXVec3 const &shapeANormal,
    Vertices const &shapeBPoints,
    GXVec3 const &shapeBNormal
) noexcept
{
    GXVec3 xAxis {};
    xAxis.Subtract ( shapeAPoints[ 1U ], shapeAPoints[ 0U ] );
    xAxis.Normalize ();

    GXVec3 yAxis {};
    yAxis.CrossProduct ( shapeANormal, xAxis );

    Project ( _windowPoints, shapeAPoints, xAxis, yAxis );
    Project ( _clipPoints, shapeBPoints, xAxis, yAxis );

    size_t const windowPoints = _windowPoints.size ();

    for ( size_t i = 0U; i < windowPoints; ++i )
    {
        _workingPoints.swap ( _clipPoints );
        _clipPoints.clear ();

        GXVec2 const& clipEdgeOrigin = _windowPoints[ i ];
        GXVec2 clipEdge {};
        clipEdge.Subtract ( _windowPoints[ ( i + 1 ) % windowPoints ], clipEdgeOrigin );

        size_t const vertexCount = _workingPoints.size ();

        GXVec2 clipNormal {};
        clipNormal._data[ 0U ] = -clipEdge._data[ 1U ];
        clipNormal._data[ 1U ] = clipEdge._data[ 0U ];

        for ( size_t j = 0U; j < vertexCount; ++j )
        {
            GXVec2 const& current = _workingPoints[ j ];
            GXVec2 const& next = _workingPoints[ ( j + 1U ) % vertexCount ];

            // Optimization: The operands have been swapped to eliminate minus in denominator later.
            GXVec2 ab {};
            ab.Subtract ( clipEdgeOrigin, current );

            GXVec2 ac {};
            ac.Subtract ( next, clipEdgeOrigin );

            float const baTest = clipNormal.DotProduct ( ab );
            float const acTest = clipNormal.DotProduct ( ac );

            auto addIntersection = [ & ] () noexcept {
                GXVec2 edge {};
                edge.Subtract ( next, current );

                // Note we swapped "ab" direction before. So no need to take negative value in denominator.
                GXVec2 alpha {};
                alpha.Sum ( current, baTest / clipNormal.DotProduct ( edge ), edge );
                _clipPoints.push_back ( alpha );
            };

            if ( acTest >= 0.0F )
            {
                if ( baTest > 0.0F )
                    addIntersection ();

                _clipPoints.push_back ( next );
                continue;
            }

            if ( baTest > 0.0F )
                continue;

            addIntersection ();
        }
    }

    _result.clear ();
    _result.reserve ( _clipPoints.size () );

    // Restoring 3D points.

    GXVec3 originAOffset {};
    originAOffset.Multiply ( shapeANormal, shapeANormal.DotProduct ( shapeAPoints[ 0U ] ) );

    GXVec3 const& b0 = shapeBPoints[ 0U ];

    for ( auto const& v : _clipPoints )
    {
        // Restoring 3D point on shape A...
        GXVec3 alpha {};
        alpha.Sum ( originAOffset, v._data[ 0U ], xAxis );
        alpha.Sum ( alpha, v._data[ 1U ], yAxis );

        // Restoring 3D point on shape B...
        GXVec3 beta {};
        beta.Subtract ( b0, alpha );

        float const dot = shapeBNormal.DotProduct ( beta );

        if ( dot < 0.0F )
        {
            // Point from shape A is in front of shape B. Skipping...
            continue;
        }

        GXVec3 gamma {};
        gamma.Sum ( alpha, dot, shapeBNormal );

        _result.emplace_back ( alpha, gamma );
    }

    return _result;
}

void SutherlandHodgman::Project ( Projection &dst,
    Vertices const &src,
    GXVec3 const &xAxis,
    GXVec3 const &yAxis
) noexcept
{
    dst.clear ();
    dst.reserve ( src.size () );

    for ( auto const& v : src )
    {
        dst.emplace_back ( xAxis.DotProduct ( v ), yAxis.DotProduct ( v ) );
    }
}

} // namespace android_vulkan
