#include <contact_detector.h>
#include <logger.h>


namespace android_vulkan {

constexpr static float const COLLINEAR_TOLERANCE = 1.0e-2F;
constexpr static float const FACE_PERPENDICULAR_TOLERANCE = 1.0e-3F;
constexpr static size_t const INITIAL_SHAPE_POINTS = 16U;
constexpr static uint16_t const RAY_COUNT = 8U;
constexpr static float const RAY_DEVIATION_DEGREES = 6.0F;
constexpr static float const SAME_POINT_TOLERANCE = 1.0e-3F;

ContactDetector::ContactDetector () noexcept:
    _cyrusBeck {},
    _epa {},
    _gjk {},
    _rays {},
    _shapeAPoints {},
    _shapeBPoints {},
    _sutherlandHodgman {}
{
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

        // Note: It's needed to clamp "t" because in 3D there is a case when edge is completely in one side
        // relative to the face.
        firstContact._point.Sum ( edge[ 0U ], std::clamp ( t, 0.0F, 1.0F ), edgeDir );

        return;
    }

    Vertices const& vertices = _cyrusBeck.Run ( face, faceNormal, edge, edgeDir );
    firstContact._point = vertices[ 0U ];

    if ( vertices.size () < 2U )
        return;

    Contact& anotherContact = AllocateAnotherContact ( contactManager, firstContactData );
    anotherContact._point = vertices[ 1U ];
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
    ac.Subtract ( _shapeAPoints[ 2U ], _shapeAPoints[ 0U ] );

    GXVec3 aNormal {};
    aNormal.CrossProduct ( ab, ac );
    aNormal.Normalize ();

    ab.Subtract ( _shapeBPoints[ 1U ], _shapeBPoints[ 0U ] );
    ab.Normalize ();

    if ( std::abs ( aNormal.DotProduct ( ab ) ) <= COLLINEAR_TOLERANCE )
    {
        // Both shapes lay in same plane.
        Vertices const& result = _sutherlandHodgman.Run ( _shapeAPoints,
            aNormal,
            _shapeBPoints,
            -firstContact._penetration
        );

        firstContact._point = result[ 0U ];
        size_t const count = result.size ();

        for ( size_t i = 1U; i < count; ++i )
        {
            Contact& anotherContact = AllocateAnotherContact ( contactManager, firstContactData );
            anotherContact._point = result[ i ];
        }

        return;
    }

    // Shapes don't lay in same plane.
    size_t const bPoints = _shapeBPoints.size ();

    // Precompute "d" parameter for ray vs plane intersection.
    float const d = aNormal.DotProduct ( _shapeAPoints[ 0U ] );

    GXVec3 featurePoints[2U] {};
    size_t featurePointCount = 0U;
    GXVec3 alpha {};

    for ( size_t i = 0U; i < bPoints; ++i )
    {
        GXVec3 const& current = _shapeBPoints[ i ];
        GXVec3 const& next = _shapeBPoints[ ( i + 1U ) % bPoints ];

        ab.Subtract ( next, current );
        float const beta = ab.DotProduct ( aNormal );

        if ( beta == 0.0F )
        {
            // Edge is parallel to a-shape. Ignore it because there are at least tho edges which must intersect the
            // A face plane.
            continue;
        }

        // The edge line pierces the face plane.
        // So we need to find ray vs plane intersection point.
        // https://www.cs.princeton.edu/courses/archive/fall00/cs426/lectures/raycast/sld017.htm
        float const t = ( d - current.DotProduct ( aNormal ) ) / beta;

        if ( t < 0.0F || t > 1.0F )
            continue;

        alpha.Sum ( current, t, ab );

        if ( featurePointCount == 0U )
        {
            featurePoints[ featurePointCount++ ] = alpha;
            continue;
        }

        constexpr float const limit = SAME_POINT_TOLERANCE * SAME_POINT_TOLERANCE;

        if ( alpha.SquaredDistance ( featurePoints[ 0U ] ) < limit )
        {
            // Edge vertex lies exactly on face plane. Ignore such case.
            continue;
        }

        featurePoints[ featurePointCount++ ] = alpha;

        if ( featurePointCount != 2U )
            continue;

        break;
    }

    if ( featurePointCount == 1U )
    {
        // It's the case when face B touches face A vis only one vertex.
        firstContact._point = featurePoints[ 0U ];
        return;
    }

    // Two feature points create feature line segment. So at this point we able to reclassify the original
    // face vs face task to edge vs face task.

    _shapeBPoints.clear ();
    _shapeBPoints.push_back ( featurePoints[ 0U ] );
    _shapeBPoints.push_back ( featurePoints[ 1U ] );

    ab.Subtract ( featurePoints[ 1U ], featurePoints[ 0U ] );

    Vertices const& vertices = _cyrusBeck.Run ( _shapeAPoints, aNormal, _shapeBPoints, ab );
    firstContact._point = vertices[ 0U ];

    if ( vertices.size () < 2U )
        return;

    Contact& anotherContact = AllocateAnotherContact ( contactManager, firstContactData );
    anotherContact._point = vertices[ 1U ];
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
