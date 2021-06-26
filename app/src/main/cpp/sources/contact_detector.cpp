#include <contact_detector.h>
#include <logger.h>


namespace android_vulkan {

constexpr static float const COLLINEAR_TOLERANCE = 1.0e-2F;
constexpr static float const FACE_PERPENDICULAR_TOLERANCE = 1.0e-3F;
constexpr static size_t const INITIAL_CLIP_POINTS = 8U;
constexpr static size_t const INITIAL_SHAPE_POINTS = 16U;
constexpr static uint16_t const RAY_COUNT = 5U;
constexpr static float const RAY_DEVIATION_DEGREES = 6.0F;
constexpr static float const SAME_POINT_TOLERANCE = 1.0e-3F;

ContactDetector::ContactDetector () noexcept:
    _clipPoints {},
    _epa {},
    _gjk {},
    _rays {},
    _shapeAPoints {},
    _shapeBPoints {},
    _workingPoints {}
{
    _clipPoints.reserve ( INITIAL_CLIP_POINTS );
    _workingPoints.reserve ( INITIAL_CLIP_POINTS );

    _shapeAPoints.reserve ( INITIAL_SHAPE_POINTS );
    _shapeAPoints.reserve ( INITIAL_SHAPE_POINTS );

    GenerateRays ();
}

void ContactDetector::Check ( ContactManager &contactManager, RigidBodyRef const &a, RigidBodyRef const &b ) noexcept
{
    RigidBody& bodyA = *a;
    RigidBody& bodyB = *b;

    if ( bodyA.IsKinematic () && bodyB.IsKinematic () )
        return;

    Shape const& shapeA = bodyA.GetShape ();
    Shape const& shapeB = bodyB.GetShape ();

    _gjk.Reset ();

    if ( !_gjk.Run ( shapeA, shapeB ) )
        return;

    _epa.Reset ();

    if ( !_epa.Run ( _gjk.GetSimplex (), shapeA, shapeB ) )
    {
        NotifyEPAFail ();
        return;
    }

    GXMat3 tbn {};
    GXVec3 const n ( _epa.GetNormal () );
    tbn.From ( n );
    CollectExtremePoints ( _shapeAPoints, shapeA, tbn );
    size_t const aCount = _shapeAPoints.size ();

    if ( aCount == 1U )
    {
        ManifoldPoint ( contactManager, a, b, tbn, _shapeAPoints.front () );
        return;
    }

    GXVec3 reverse ( n );
    reverse.Reverse ();
    tbn.SetZ ( reverse );
    CollectExtremePoints ( _shapeBPoints, shapeB, tbn );
    size_t const bCount = _shapeBPoints.size ();

    if ( bCount == 1U )
    {
        tbn.GetX ( reverse );
        reverse.Reverse ();
        tbn.SetX ( reverse );
        ManifoldPoint ( contactManager, b, a, tbn, _shapeBPoints.front () );
        return;
    }

    // Restoring original TBN matrix.
    tbn.SetZ ( n );

    if ( aCount == 2U || bCount == 2U )
    {
        if ( aCount == 2U && bCount == 2U )
        {
            ManifoldEdgeEdge ( contactManager, a, b, tbn );
            return;
        }

        ManifoldEdgeFace ( contactManager, a, b, tbn );
        return;
    }

    ManifoldFaceFace ( contactManager, a, b, tbn );
}

ContactDetector::FirstContactData ContactDetector::AllocateFirstContact ( ContactManager &contactManager,
    RigidBodyRef const &a,
    RigidBodyRef const &b,
    GXMat3 const &tbn
) const noexcept
{
    ContactManifold& manifold = contactManager.AllocateContactManifold ();
    manifold._bodyA = a;
    manifold._bodyB = b;
    manifold._epaSteps = _epa.GetSteps ();
    manifold._gjkSteps = _gjk.GetSteps ();

    Contact& contact = contactManager.AllocateContact ( manifold );
    tbn.GetX ( contact._tangent );
    tbn.GetY ( contact._bitangent );
    tbn.GetZ ( contact._normal );
    contact._penetration = _epa.GetDepth ();

    return std::make_pair ( &manifold, &contact );
}

void ContactDetector::CollectExtremePoints ( Vertices &vertices, Shape const &shape, GXMat3 const &tbn ) noexcept
{
    // TODO sphere case is trivial.

    vertices.clear ();
    GXVec3 d {};

    for ( auto const& ray : _rays )
    {
        tbn.MultiplyVectorMatrix ( d, ray );
        GXVec3 const p = shape.GetExtremePointWorld ( d );
        auto const end = vertices.cend ();

        auto findResult = std::find_if ( vertices.cbegin (),
            end,
            [ & ] ( GXVec3 const &v ) noexcept -> bool {
                return std::memcmp ( &p, &v, sizeof ( p ) ) == 0;
            }
        );

        if ( findResult != end )
            continue;

        // Unique vertex was found.
        vertices.push_back ( p );
    }
}

void ContactDetector::GenerateRays () noexcept
{
    static_assert ( RAY_DEVIATION_DEGREES <= 90.0F, "RAY_DEVIATION_DEGREES must be less or equal than 90.0F degrees" );

    constexpr float const delta = GX_MATH_DOUBLE_PI / static_cast<float> ( RAY_COUNT );
    float angle = GXDegToRad ( RAY_DEVIATION_DEGREES );

    float const radius = std::sin ( angle );
    GXVec3 const s ( radius, radius, std::cos ( angle ) );
    _rays.reserve ( RAY_COUNT );

    for ( uint16_t i = 0U; i < RAY_COUNT; ++i )
    {
        angle = delta * static_cast<float> ( i );
        GXVec3& ray = _rays.emplace_back ( std::cos ( angle ), std::sin ( angle ), 1.0F );
        ray.Multiply ( ray, s );
    }
}

void ContactDetector::ManifoldEdgeEdge ( ContactManager &contactManager,
    RigidBodyRef const &a,
    RigidBodyRef const &b,
    GXMat3 const &tbn
) noexcept
{
    FirstContactData firstContactData = AllocateFirstContact ( contactManager, a, b, tbn );
    Contact& firstContact = *firstContactData.second;

    GXVec3 alpha {};
    alpha.Subtract ( _shapeAPoints[ 1U ], _shapeAPoints[ 0U ] );
    float const lenA = alpha.Length ();
    float const invLenA = 1.0F / lenA;

    GXVec3 aDir {};
    aDir.Multiply ( alpha, invLenA );

    GXVec3 beta {};
    beta.Subtract ( _shapeBPoints[ 1U ], _shapeBPoints[ 0U ] );

    GXVec3 bDir { beta };
    bDir.Normalize ();

    if ( 1.0F - std::abs ( aDir.DotProduct ( bDir ) ) < COLLINEAR_TOLERANCE )
    {
        // The edges lie on each other, i.e. they're collinear vectors. Can be one or two intersection points.
        // Project second segment on first segment. Then clip points.

        GXVec3 ab0 {};
        ab0.Subtract ( _shapeBPoints[ 0U ], _shapeAPoints[ 0U ] );

        GXVec3 ab1 {};
        ab1.Subtract ( _shapeBPoints[ 1U ], _shapeAPoints[ 0U ] );

        // Note dot product is: ||a|| ||b|| cos ( angle ). Need to remove ||a|| component and clamp result
        // in range [0.0F, lenA]. This will allow us to restore end points of overlap section in term
        // of vector "aDir".

        GXVec2 proj ( alpha.DotProduct ( ab0 ), alpha.DotProduct ( ab1 ) );
        proj.Multiply ( proj, invLenA );
        proj._data[ 0U ] = std::clamp ( proj._data[ 0U ], 0.0F, lenA );
        proj._data[ 1U ] = std::clamp ( proj._data[ 1U ], 0.0F, lenA );

        firstContact._point.Sum ( _shapeAPoints[ 0U ], proj._data[ 0U ], aDir );

        if ( std::abs ( proj._data[ 0U ] - proj._data[ 1U ] ) < SAME_POINT_TOLERANCE )
        {
            // The end points of intersection segment is very close to each other. So consider this case
            // as one point intersection.
            return;
        }

        Contact& anotherContact = AllocateAnotherContact ( contactManager, firstContactData );
        anotherContact._point.Sum ( _shapeAPoints[ 0U ], proj._data[ 1U ], aDir );
        return;
    }

    // Edges don't lie on each other. Also by design edges must have common intersection point. So we can use
    // plane of edges as projection plane to solve intersection point for 2D case.

    GXVec3 tmp {};
    tmp.CrossProduct ( aDir, bDir );
    GXVec3 anotherAxis {};
    anotherAxis.CrossProduct ( tmp, aDir );

    auto project = [ & ] ( GXVec2* dst, Vertices const &src, GXVec3 const &xAxis, GXVec3 const &yAxis ) noexcept {
        GXVec3 const& s0 = src[ 0U ];
        GXVec3 const& s1 = src[ 1U ];

        GXVec2& d0 = dst[ 0U ];
        d0._data[ 0U ] = xAxis.DotProduct ( s0 );
        d0._data[ 1U ] = yAxis.DotProduct ( s0 );

        GXVec2& d1 = dst[ 1U ];
        d1._data[ 0U ] = xAxis.DotProduct ( s1 );
        d1._data[ 1U ] = yAxis.DotProduct ( s1 );
    };

    GXVec2 projA[ 2U ] {};
    project ( projA, _shapeAPoints, aDir, anotherAxis );

    GXVec2 projB[ 2U ] {};
    project ( projB, _shapeBPoints, aDir, anotherAxis );

    GXVec2 ba {};
    ba.Subtract ( projA[ 0U ], projB[ 0U ] );

    GXVec2 alphaProj {};
    alphaProj.Subtract ( projA[ 1U ], projA[ 0U ] );
    GXVec2 n ( -alphaProj._data[ 1U ], alphaProj._data[ 0U ] );

    GXVec2 betaProj {};
    betaProj.Subtract ( projB[ 1U ], projB[ 0U ] );

    firstContact._point.Sum ( _shapeBPoints[ 0U ], ba.DotProduct ( n ) / n.DotProduct ( betaProj ), beta );
}

void ContactDetector::ManifoldEdgeFace ( ContactManager &contactManager,
    RigidBodyRef const &a,
    RigidBodyRef const &b,
    GXMat3 const &tbn
) noexcept
{
    FirstContactData firstContactData = AllocateFirstContact ( contactManager, a, b, tbn );
    Contact& firstContact = *firstContactData.second;

    Vertices const* e;
    Vertices const* f;

    if ( _shapeAPoints.size () == 2U )
    {
        e = &_shapeAPoints;
        f = &_shapeBPoints;
    }
    else
    {
        e = &_shapeBPoints;
        f = &_shapeAPoints;
    }

    Vertices const& edge = *e;
    Vertices const& face = *f;

    GXVec3 edgeDir {};
    edgeDir.Subtract ( edge[ 1U ], edge[ 0U ] );

    GXVec3 ab {};
    ab.Subtract ( face[ 1U ], face[ 0U ] );

    GXVec3 ac {};
    ac.Subtract ( face[ 2U ], face[ 0U ] );

    GXVec3 faceNormal;
    faceNormal.CrossProduct ( ab, ac );
    faceNormal.Normalize ();

    if ( std::abs ( faceNormal.DotProduct ( edgeDir ) ) >= FACE_PERPENDICULAR_TOLERANCE )
    {
        // The edge pierces the face. There is only one contact point.
        // So we need to find ray vs plane intersection point.
        // https://www.cs.princeton.edu/courses/archive/fall00/cs426/lectures/raycast/sld017.htm

        float const d = faceNormal.DotProduct ( face[ 1U ] );
        float const t = ( d - edge[ 0U ].DotProduct ( faceNormal ) ) / edgeDir.DotProduct ( faceNormal );
        firstContact._point.Sum ( edge[ 0U ], t, edgeDir );

        return;
    }

    // Cyrus-Beck algorithm. The implementation is based on ideas from
    // https://www.geeksforgeeks.org/line-clipping-set-2-cyrus-beck-algorithm/

    size_t const facePoints = std::size ( face );
    GXVec3 const& edgeOrigin = edge[ 0U ];
    GXVec3 edgeUnitVector ( edgeDir );
    edgeUnitVector.Normalize ();

    GXVec3 ortho ( faceNormal );
    GXVec3 alpha {};

    float start = 0.0F;
    float end = 1.0F;

    for ( size_t i = 0U; i < facePoints; ++i )
    {
        GXVec3 const& aPoint = face[ i ];
        ab.Subtract ( face[ ( i + 1U ) % facePoints ], aPoint );

        // The normal should point away from the face inner area. So changing cross product order.
        faceNormal.CrossProduct ( ab, ortho );
        faceNormal.Normalize ();

        float const beta = faceNormal.DotProduct ( edgeUnitVector );

        if ( std::abs ( beta ) < COLLINEAR_TOLERANCE )
        {
            // Edge is parallel to face normal. We could safely ignore it because there must be another edge face
            // combinations which will produce start and end points. It's working because we already know that there is
            // at least one intersection point so far.
            continue;
        }

        // Optimization: The operands have been swapped to eliminate minus in denominator later.
        alpha.Subtract ( aPoint, edgeOrigin );
        float const gamma = faceNormal.DotProduct ( edgeDir );
        float const p = faceNormal.DotProduct ( alpha ) / gamma;

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

    firstContact._point.Sum ( edgeOrigin, start, edgeDir );

    if ( end - start < SAME_POINT_TOLERANCE )
    {
        // The end points of intersection segment is very close to each other. So consider this case
        // as one point intersection.
        return;
    }

    Contact& anotherContact = AllocateAnotherContact ( contactManager, firstContactData );
    anotherContact._point.Sum ( edgeOrigin, end, edgeDir );
}

void ContactDetector::ManifoldFaceFace ( ContactManager &contactManager,
    RigidBodyRef const &a,
    RigidBodyRef const &b,
    GXMat3 const &tbn
) noexcept
{
    FirstContactData firstContactData = AllocateFirstContact ( contactManager, a, b, tbn );
    Contact& firstContact = *firstContactData.second;

    GXVec3 ab {};
    ab.Subtract ( _shapeAPoints[ 1U ], _shapeAPoints[ 0U ] );

    GXVec3 ac {};
    ab.Subtract ( _shapeAPoints[ 2U ], _shapeAPoints[ 0U ] );

    GXVec3 aNormal {};
    aNormal.CrossProduct ( ab, ac );

    ab.Subtract ( _shapeBPoints[ 1U ], _shapeBPoints[ 0U ] );

    if ( std::abs ( aNormal.DotProduct ( ab ) ) > COLLINEAR_TOLERANCE )
    {
        // Shapes don't lay in same plane.
        // TODO
        return;
    }

    // Both shapes lay in same plane.

    // Sutherland–Hodgman algorithm. The implementation is based on ideas from
    // https://en.wikipedia.org/wiki/Sutherland%E2%80%93Hodgman_algorithm

    size_t const aPoints = _shapeAPoints.size ();
    size_t const bPoints = _shapeBPoints.size ();

    _clipPoints.clear ();
    _clipPoints.reserve ( bPoints );

    _workingPoints.clear ();
    _clipPoints.reserve ( bPoints );

    _clipPoints.assign ( _shapeBPoints.cbegin (), _shapeBPoints.cend () );
    GXVec3 clipEdge {};

    GXVec3 edge {};
    GXVec3 clipNormal {};
    GXVec3 alpha {};

    for ( size_t i = 0U; i < aPoints; ++i )
    {
        _workingPoints.swap ( _clipPoints );
        _clipPoints.clear ();

        GXVec3 const& clipEdgeOrigin = _shapeAPoints[ i ];
        clipEdge.Subtract ( _shapeAPoints[ ( i + 1 ) % aPoints ], clipEdgeOrigin );

        size_t const vertexCount = _workingPoints.size ();
        clipNormal.CrossProduct ( aNormal, clipEdge );

        for ( size_t j = 0U; j < vertexCount; ++j )
        {
            GXVec3 const& current = _workingPoints[ j ];
            GXVec3 const& next = _workingPoints[ ( j + 1U ) % vertexCount ];

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

    size_t const contactCount = _clipPoints.size ();
    firstContact._point = _clipPoints[ 0U ];

    for ( size_t i = 1U; i < contactCount; ++i )
    {
        Contact& anotherContact = AllocateAnotherContact ( contactManager, firstContactData );
        anotherContact._point = _clipPoints[ i ];
    }
}

void ContactDetector::ManifoldPoint ( ContactManager &contactManager,
    RigidBodyRef const &a,
    RigidBodyRef const &b,
    GXMat3 const &tbn,
    GXVec3 const &vertex
) noexcept
{
    FirstContactData data = AllocateFirstContact ( contactManager, a, b, tbn );
    Contact& contact = *data.second;
    contact._point = vertex;
}

void ContactDetector::NotifyEPAFail () noexcept
{
    constexpr char const format[] =
R"__(ContactDetector::NotifyEPAFail - Can't find penetration depth and separation normal.
    >>> EPA:
        steps:  %hhu
        vertices: %hhu
        edges: %hhu
        faces: %hhu
    >>> GJK:
        steps: %hhu
        lines: %hhu
        triangles: %hhu
        tetrahedrons: %hhu)__";

    LogWarning ( format,
        _epa.GetSteps (),
        _epa.GetVertexCount (),
        _epa.GetEdgeCount (),
        _epa.GetFaceCount (),
        _gjk.GetSteps (),
        _gjk.GetTestLines (),
        _gjk.GetTestTriangles (),
        _gjk.GetTestTetrahedrons ()
    );
}

Contact& ContactDetector::AllocateAnotherContact ( ContactManager &contactManager,
    FirstContactData &firstContactData
) noexcept
{
    auto& [manifold, contact] = firstContactData;

    Contact& result = contactManager.AllocateContact ( *manifold );
    result._tangent = contact->_tangent;
    result._bitangent = contact->_bitangent;
    result._normal = contact->_normal;
    result._penetration = contact->_penetration;

    return result;
}

} // namespace android_vulkan
