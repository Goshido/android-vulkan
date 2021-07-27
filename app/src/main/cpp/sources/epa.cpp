#include <epa.h>
#include <logger.h>

GX_DISABLE_COMMON_WARNINGS

#include <map>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

constexpr static size_t const INITIAL_VERTICES = 1024U;
constexpr static size_t const INITIAL_EDGES = INITIAL_VERTICES * 4U;
constexpr static size_t const INITIAL_FACES = INITIAL_VERTICES * 4U;
constexpr static uint16_t const MAXIMUM_STEPS = 16U;
constexpr static float const TOLERANCE = 1.0e-3F;

//----------------------------------------------------------------------------------------------------------------------

Face::Face ( size_t a, size_t b, size_t c, Vertices const &vertices ) noexcept:
    _a ( a ),
    _b ( b ),
    _c ( c ),
    _normal {}
{
    GXVec3 const& pointA = vertices[ a ];

    GXVec3 ab {};
    ab.Subtract ( vertices[ b ], pointA );

    GXVec3 ac {};
    ac.Subtract ( vertices[ c ], pointA );

    _normal.CrossProduct ( ab, ac );
    _normal.Normalize ();
}

//----------------------------------------------------------------------------------------------------------------------

EPA::EPA () noexcept:
    _depth ( 0.0F ),
    _normal ( 1.0F, 0.0F, 0.0F ),
    _edges {},
    _faces {},
    _vertices {},
    _steps ( 0U )
{
    _edges.reserve ( INITIAL_EDGES );
    _faces.reserve ( INITIAL_FACES );
    _vertices.reserve ( INITIAL_VERTICES );
}

float EPA::GetDepth () const noexcept
{
    return _depth;
}

GXVec3 const& EPA::GetNormal () const noexcept
{
    return _normal;
}

uint16_t EPA::GetSteps () const noexcept
{
    return _steps;
}

uint16_t EPA::GetEdgeCount () const noexcept
{
    return _edges.size ();
}

uint16_t EPA::GetFaceCount () const noexcept
{
    return _faces.size ();
}

uint16_t EPA::GetVertexCount () const noexcept
{
    return _vertices.size ();
}

void EPA::Reset () noexcept
{
    _steps = 0U;
    _vertices.clear ();
    _faces.clear ();
}

bool EPA::Run ( Simplex const &simplex, Shape const &shapeA, Shape const &shapeB ) noexcept
{
    CreatePolytope ( simplex );
    auto [closestFace, distance] = FindClosestFace ();

    for ( ; _steps < MAXIMUM_STEPS; ++_steps )
    {
        Face const& face = _faces[ closestFace ];
        GXVec3 const supportPoint = Shape::FindSupportPoint ( face._normal, shapeA, shapeB );

        GXVec3 sp {};
        sp.Multiply ( supportPoint, 1.0e+3F );

        if ( std::abs ( face._normal.DotProduct ( supportPoint ) - distance ) < TOLERANCE )
        {
            _normal = face._normal;
            _depth = distance;
            return true;
        }

        // Note we don't care about unused vertices which could be produced while polytope reconstruction process.
        // So the algorithm is greedy in terms of memory consumption.
        // But same time it doesn't require to remove unused vertices and adjust indices inside Face data structures.

        _edges.clear ();

        // Optimization: While traversing through all faces we could collect the closest back facing one for free.
        // This will allow to reduce computation amount later. The idea is to find the closest face in the list
        // of new faces. And then to compare this face with the closest back facing one.

        distance = FLT_MAX;
        GXVec3 const removeNormal = face._normal;

        for ( size_t i = 0U; i < _faces.size (); )
        {
            Face& f = _faces[ i ];

            GXVec3 probe {};
            probe.Subtract ( supportPoint, _vertices[ f._a ] );

            if ( probe.DotProduct ( f._normal ) > 0.0F )
            {
                // Front facing face.

                SolveEdge ( f._a, f._b );
                SolveEdge ( f._b, f._c );
                SolveEdge ( f._c, f._a );

                // Remove face "f" from "_faces" by moving last face in the "_faces" to place where "f" is.
                // Note the "i" must not change because target face will be just moved one.
                // If the face to remove is the last one this algorithm will work as well.
                f = _faces.back ();
                _faces.pop_back ();
                continue;
            }

            // Back facing face.

            float const d = f._normal.DotProduct ( _vertices[ f._a ] );

            if ( d < distance )
            {
                distance = d;
                closestFace = i;
            }

            ++i;
        }

        // Right now "_faces" contains only back facing faces.
        size_t backFacingFaces = _faces.size ();

        // Also "_edges" contains only edges from which could be safely created new faces with
        // the "supportPoint" vertex.

        size_t const edgeCount = _edges.size ();

        size_t const idx = _vertices.size ();
        _vertices.push_back ( supportPoint );

        for ( size_t i = 0U; i < edgeCount; ++i )
        {
            Edge const& edge = _edges[ i ];
            Face const& f = _faces.emplace_back ( edge.first, edge.second, idx, _vertices );
            float const d = f._normal.DotProduct ( _vertices[ f._a ] );

            if ( d >= distance )
                continue;

            // New front facing face is the closest to the origin.
            distance = d;
            closestFace = backFacingFaces + i;
        }
    }

    constexpr char const format[] =
R"__(EPA::Run - Algorithm exceeded maximum steps. Counters:
    _steps: %hu
    vertices: %zu
    faces: %zu
    edges: %zu)__";

    LogWarning ( format, _steps, _vertices.size (), _faces.size (), _edges.size () );
    return false;
}

void EPA::CreatePolytope ( Simplex const &simplex ) noexcept
{
    // The simplex building was followed winding order rule.
    // The vertex order was "d", "c", "b", "a"
    //  where:
    //      "a" - simplex._supportPoints[ 0U ]
    //      "b" - simplex._supportPoints[ 1U ]
    //      "c" - simplex._supportPoints[ 2U ]
    //      "d" - simplex._supportPoints[ 3U ]
    // Also "bcd" - triangle was oriented to the origin: "cb" x "cd".
    // Point "a" lies in front of "bcd" triangle.
    // We need to build polytope from simplex where all normals will direct outside the origin.
    // Also we must preserve winding order.
    // For example triangle "xyz". "x", "y" and "z" must be specified such way that "xy" x "xz" will give us the normal
    // outside the origin.

    _vertices.push_back ( simplex._supportPoints[ 0U ] );
    _vertices.push_back ( simplex._supportPoints[ 1U ] );
    _vertices.push_back ( simplex._supportPoints[ 2U ] );
    _vertices.push_back ( simplex._supportPoints[ 3U ] );

    // "bdc"
    _faces.emplace_back ( 1U, 3U, 2U, _vertices );

    // "adb"
    _faces.emplace_back ( 0U, 3U, 1U, _vertices );

    // "acd"
    _faces.emplace_back ( 0U, 2U, 3U, _vertices );

    // "abc"
    _faces.emplace_back ( 0U, 1U, 2U, _vertices );
}

void EPA::SolveEdge ( size_t a, size_t b ) noexcept
{
    // The edge "ab" must be appended if there is no "ba" version in the "_edges".
    // Otherwise "ab" edge must be removed from "_edges".

    auto findResult = std::find ( _edges.begin (), _edges.end (), std::make_pair ( b, a ) );

    if ( findResult == _edges.end() )
    {
        // There is no "ba" version. So append the edge "ab".
        _edges.emplace_back ( a, b );
        return;
    }

    // Edge "ba" was found. Remove it by moving last edge to "ba" place.
    // If "ba" edge is the last one this algorithm will work as well.
    *findResult = _edges.back ();
    _edges.pop_back ();
}

EPA::FindResult EPA::FindClosestFace () noexcept
{
    GXVec3 const* vertices = _vertices.data ();

    Face const* faces = _faces.data ();
    size_t const faceCount = _faces.size ();

    Face const& firstFace = faces[ 0U ];

    FindResult result = std::make_pair ( 0U, firstFace._normal.DotProduct ( vertices[ firstFace._a ] ) );
    auto& [closestFace, distance] = result;

    for ( size_t i = 1U; i < faceCount; ++i )
    {
        Face const& face = faces[ i ];
        float const d = face._normal.DotProduct ( vertices[ face._a ] );

        if ( d >= distance )
            continue;

        closestFace = i;
        distance = d;
    }

    return result;
}

} // namespace android_vulkan
