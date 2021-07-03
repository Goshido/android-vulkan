#include <sutherland_hodgman.h>


namespace android_vulkan {

constexpr static size_t const DEFAULT_CAPACITY = 16U;

SutherlandHodgman::SutherlandHodgman () noexcept:
    _clipPoints {},
    _result {},
    _windowPoints {},
    _workingPoints {}
{
    _clipPoints.reserve ( DEFAULT_CAPACITY );
    _result.reserve ( DEFAULT_CAPACITY );
    _windowPoints.reserve ( DEFAULT_CAPACITY );
    _workingPoints.reserve ( DEFAULT_CAPACITY );
}

Vertices const& SutherlandHodgman::Run ( Vertices const &shapeAPoints,
    GXVec3 const &shapeANormal,
    Vertices const &shapeBPoints,
    float penetration
) noexcept
{
    GXVec3 xAxis {};
    xAxis.Subtract ( shapeAPoints[ 1U ], shapeAPoints[ 0U ] );
    xAxis.Normalize ();

    GXVec3 yAxis {};
    yAxis.CrossProduct ( shapeANormal, xAxis );

    Project ( _windowPoints, shapeAPoints, xAxis, yAxis );
    Project ( _clipPoints, shapeBPoints, xAxis, yAxis );

    GXVec2 edge {};
    GXVec2 clipEdge {};
    GXVec2 clipNormal {};
    GXVec2 alpha {};
    GXVec2 ab {};
    GXVec2 ac {};

    size_t const windowPoints = _windowPoints.size ();

    for ( size_t i = 0U; i < windowPoints; ++i )
    {
        _workingPoints.swap ( _clipPoints );
        _clipPoints.clear ();

        GXVec2 const& clipEdgeOrigin = _windowPoints[ i ];
        clipEdge.Subtract ( _windowPoints[ ( i + 1 ) % windowPoints ], clipEdgeOrigin );

        size_t const vertexCount = _workingPoints.size ();

        clipNormal._data[ 0U ] = -clipEdge._data[ 1U ];
        clipNormal._data[ 1U ] = clipEdge._data[ 0U ];

        for ( size_t j = 0U; j < vertexCount; ++j )
        {
            GXVec2 const& current = _workingPoints[ j ];
            GXVec2 const& next = _workingPoints[ ( j + 1U ) % vertexCount ];

            // Optimization: The operands have been swapped to eliminate minus in denominator later.
            ab.Subtract ( clipEdgeOrigin, current );
            ac.Subtract ( next, clipEdgeOrigin );

            float const baTest = clipNormal.DotProduct ( ab );
            float const acTest = clipNormal.DotProduct ( ac );

            auto addIntersection = [ & ] () noexcept {
                edge.Subtract ( next, current );

                // Note we swapped "ab" direction before. So no need to take negative value in denominator.
                alpha.Sum ( current, baTest / clipNormal.DotProduct ( edge ), edge );
                _clipPoints.push_back ( alpha );
            };

            if ( acTest > 0.0F )
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

    // Restoring 3D coordinates.

    GXVec3 originOffset {};
    originOffset.Multiply ( shapeANormal, shapeANormal.DotProduct ( shapeAPoints[ 0U ] ) );
    originOffset.Sum ( originOffset, penetration, shapeANormal );

    GXVec3 beta {};

    for ( auto const& v : _clipPoints )
    {
        beta.Sum ( originOffset, v._data[ 0U ], xAxis );
        beta.Sum ( beta, v._data[ 1U ], yAxis );
        _result.push_back ( beta );
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
