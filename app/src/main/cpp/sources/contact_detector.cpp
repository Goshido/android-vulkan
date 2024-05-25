#include <contact_detector.hpp>
#include <logger.hpp>


namespace android_vulkan {

namespace {

constexpr float COLLINEAR_TOLERANCE = 5.0e-4F;
constexpr size_t INITIAL_SHAPE_POINTS = 16U;
constexpr uint16_t RAY_COUNT = 8U;
constexpr float RAY_DEVIATION_DEGREES = 6.0F;
constexpr float SAME_POINT_TOLERANCE = 1.0e-3F;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

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
    RigidBody &bodyA = *a;
    RigidBody &bodyB = *b;

    Shape const &shapeA = bodyA.GetShape ();
    Shape const &shapeB = bodyB.GetShape ();

    if ( !( shapeA.GetCollisionGroups () & shapeB.GetCollisionGroups () ) )
        return;

    if ( !shapeA.GetBoundsWorld ().IsOverlapped ( shapeB.GetBoundsWorld () ) )
        return;

    _gjk.Reset ();

    if ( !_gjk.Run ( shapeA, shapeB ) )
        return;

    _epa.Reset ();

    if ( !_epa.Run ( _gjk.GetSimplex (), shapeA, shapeB ) )
    {

#ifdef ANDROID_VULKAN_DEBUG

        NotifyEPAFail ();

#endif // ANDROID_VULKAN_DEBUG

        return;
    }

    float const restitution = shapeA.GetRestitution () * shapeB.GetRestitution ();
    float const friction = std::min ( shapeA.GetFriction (), shapeB.GetFriction () );

    GXMat3 tbn {};
    GXVec3 const n ( _epa.GetNormal () );
    tbn.From ( n );

    if ( shapeA.GetType () == eShapeType::Sphere )
    {
        _shapeAPoints.clear ();
        _shapeAPoints.emplace_back ( shapeA.GetExtremePointWorld ( n ) );
        ManifoldPoint ( contactManager, a, b, tbn, friction, restitution, _shapeAPoints.front () );
        return;
    }

    if ( shapeB.GetType () == eShapeType::Sphere )
    {
        GXVec3 reverse ( n );
        reverse.Reverse ();
        _shapeBPoints.clear ();
        _shapeBPoints.emplace_back ( shapeB.GetExtremePointWorld ( reverse ) );

        ManifoldPoint ( contactManager, a, b, tbn, friction, restitution, _shapeBPoints.front () );
        return;
    }

    CollectForwardExtremePoints ( _shapeAPoints, shapeA, tbn );
    size_t const aCount = _shapeAPoints.size ();

    if ( aCount == 1U )
    {
        ManifoldPoint ( contactManager, a, b, tbn, friction, restitution, _shapeAPoints.front () );
        return;
    }

    GXVec3 reverse ( n );
    reverse.Reverse ();
    tbn.SetZ ( reverse );
    CollectBackwardExtremePoints ( _shapeBPoints, shapeB, tbn );
    size_t const bCount = _shapeBPoints.size ();

    if ( bCount == 1U )
    {
        tbn.GetX ( reverse );
        reverse.Reverse ();
        tbn.SetX ( reverse );
        ManifoldPoint ( contactManager, b, a, tbn, friction, restitution, _shapeBPoints.front () );
        return;
    }

    // Restoring original TBN matrix.
    tbn.SetZ ( n );

    if ( aCount == 2U || bCount == 2U )
    {
        if ( aCount == 2U && bCount == 2U )
        {
            ManifoldEdgeEdge ( contactManager, a, b, tbn, friction, restitution );
            return;
        }

        ManifoldEdgeFace ( contactManager, a, b, friction, restitution );
        return;
    }

    ManifoldFaceFace ( contactManager, a, b, friction, restitution );
}

ContactDetector::FirstContactData ContactDetector::AllocateFirstContact ( ContactManager &contactManager,
    RigidBodyRef const &a,
    RigidBodyRef const &b,
    float friction,
    float restitution
) const noexcept
{
    ContactManifold &manifold = contactManager.AllocateContactManifold ();
    manifold._bodyA = a;
    manifold._bodyB = b;

    manifold._epaSteps = _epa.GetSteps ();
    manifold._gjkSteps = _gjk.GetSteps ();

    Contact &contact = contactManager.AllocateContact ( manifold );
    contact._friction = friction;
    contact._restitution = restitution;

    return std::make_pair ( &manifold, &contact );
}

void ContactDetector::CollectBackwardExtremePoints ( Vertices &vertices,
    Shape const &shape,
    GXMat3 const &tbn
) noexcept
{
    vertices.clear ();
    auto const end = _rays.crend ();

    for ( auto i = _rays.crbegin (); i != end; ++i )
    {
        AppendExtremePoint ( vertices, shape, tbn, *i );
    }
}

void ContactDetector::CollectForwardExtremePoints ( Vertices &vertices, Shape const &shape, GXMat3 const &tbn ) noexcept
{
    vertices.clear ();

    for ( auto const &ray : _rays )
    {
        AppendExtremePoint ( vertices, shape, tbn, ray );
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
        GXVec3 &ray = _rays.emplace_back ( std::cos ( angle ), std::sin ( angle ), 1.0F );
        ray.Multiply ( ray, s );
    }
}

void ContactDetector::ManifoldEdgeEdge ( ContactManager &contactManager,
    RigidBodyRef const &a,
    RigidBodyRef const &b,
    GXMat3 const &tbn,
    float friction,
    float restitution
) noexcept
{
    auto [manifold, firstContact] = AllocateFirstContact ( contactManager, a, b, friction, restitution );
    tbn.GetX ( firstContact->_tangent );
    tbn.GetY ( firstContact->_bitangent );
    tbn.GetZ ( firstContact->_normal );

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
    float const penetration = _epa.GetDepth ();

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

        firstContact->_pointA.Sum ( _shapeAPoints[ 0U ], proj._data[ 0U ], aDir );
        firstContact->_pointB.Sum ( firstContact->_pointA, penetration, firstContact->_normal );
        firstContact->_penetration = penetration;

        if ( std::abs ( proj._data[ 0U ] - proj._data[ 1U ] ) < SAME_POINT_TOLERANCE )
        {
            // The end points of intersection segment is very close to each other. So consider this case
            // as one point intersection.
            return;
        }

        Contact &anotherContact = contactManager.AllocateContact ( *manifold );
        anotherContact._friction = friction;
        anotherContact._restitution = restitution;
        anotherContact._pointA.Sum ( _shapeAPoints[ 0U ], proj._data[ 1U ], aDir );
        anotherContact._pointB.Sum ( anotherContact._pointA, penetration, firstContact->_normal );
        anotherContact._penetration = penetration;

        anotherContact._tangent = firstContact->_tangent;
        anotherContact._bitangent = firstContact->_bitangent;
        anotherContact._normal = firstContact->_normal;

        contactManager.Warm ( *manifold );
        return;
    }

    // Edges don't lie on each other. Also by design edges must have common intersection point. So we can use
    // plane of edges as projection plane to solve intersection point for 2D case.

    GXVec3 tmp {};
    tmp.CrossProduct ( aDir, bDir );
    GXVec3 anotherAxis {};
    anotherAxis.CrossProduct ( tmp, aDir );

    auto project = [ & ] ( GXVec2* dst, Vertices const &src, GXVec3 const &xAxis, GXVec3 const &yAxis ) noexcept {
        GXVec3 const &s0 = src[ 0U ];
        GXVec3 const &s1 = src[ 1U ];

        GXVec2 &d0 = dst[ 0U ];
        d0._data[ 0U ] = xAxis.DotProduct ( s0 );
        d0._data[ 1U ] = yAxis.DotProduct ( s0 );

        GXVec2 &d1 = dst[ 1U ];
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

    firstContact->_pointB.Sum ( _shapeBPoints[ 0U ], ba.DotProduct ( n ) / n.DotProduct ( betaProj ), beta );
    firstContact->_pointA.Sum ( firstContact->_pointB, penetration, firstContact->_normal );
    firstContact->_penetration = penetration;

    contactManager.Warm ( *manifold );
}

void ContactDetector::ManifoldEdgeFace ( ContactManager &contactManager,
    RigidBodyRef const &a,
    RigidBodyRef const &b,
    float friction,
    float restitution
) noexcept
{
    FirstContactData firstContactData {};

    Vertices const* e;
    Vertices const* f;

    if ( _shapeAPoints.size () == 2U )
    {
        e = &_shapeAPoints;
        f = &_shapeBPoints;
        firstContactData = AllocateFirstContact ( contactManager, a, b, friction, restitution );
    }
    else
    {
        e = &_shapeBPoints;
        f = &_shapeAPoints;
        firstContactData = AllocateFirstContact ( contactManager, b, a, friction, restitution );
    }

    auto &[manifold, firstContact] = firstContactData;

    Vertices const &edge = *e;
    Vertices const &face = *f;

    GXVec3 edgeDir {};
    edgeDir.Subtract ( edge[ 1U ], edge[ 0U ] );

    GXVec3 ab {};
    ab.Subtract ( face[ 1U ], face[ 0U ] );

    GXVec3 ac {};
    ac.Subtract ( face[ 2U ], face[ 0U ] );

    GXVec3 faceNormal {};
    faceNormal.CrossProduct ( ab, ac );
    faceNormal.Normalize ();

    Vertices const &vertices = _cyrusBeck.Run ( face, faceNormal, edge, edgeDir );

    GXVec3 alpha {};
    alpha.Subtract ( face[ 0U ], vertices[ 0U ] );
    constexpr float const SAME_POINT = SAME_POINT_TOLERANCE * SAME_POINT_TOLERANCE;
    float penetration = faceNormal.DotProduct ( alpha );
    bool firstContactUsed = false;

    auto write = [ & ] ( Contact &contact, GXVec3 const &v, float penetration ) noexcept {
        contact._pointA = v;
        contact._pointB.Sum ( v, penetration, faceNormal );
        contact._penetration = penetration;

        GXVec3 &normal = contact._normal;
        normal = faceNormal;
        normal.Reverse ();

        GXMat3 basis {};
        basis.From ( normal );

        basis.GetX ( contact._tangent );
        basis.GetY ( contact._bitangent );
    };

    if ( penetration > 0.0F && alpha.SquaredLength () > SAME_POINT )
    {
        write ( *firstContact, vertices[ 0U ], penetration );
        firstContactUsed = true;
    }

    if ( vertices.size () < 2U )
    {
        contactManager.Warm ( *manifold );
        return;
    }

    alpha.Subtract ( face[ 0U ], vertices[ 1U ] );
    penetration = faceNormal.DotProduct ( alpha );

    if ( penetration <= 0.0F || alpha.SquaredLength () <= SAME_POINT )
    {
        contactManager.Warm ( *manifold );
        return;
    }

    Contact &targetContact = firstContactUsed ?
        contactManager.AllocateContact ( *firstContactData.first ) : *firstContact;

    targetContact._friction = friction;
    targetContact._restitution = restitution;
    write ( targetContact, vertices[ 1U ], penetration );

    contactManager.Warm ( *manifold );
}

void ContactDetector::ManifoldFaceFace ( ContactManager &contactManager,
    RigidBodyRef const &a,
    RigidBodyRef const &b,
    float friction,
    float restitution
) noexcept
{
    GXVec3 ab {};
    ab.Subtract ( _shapeAPoints[ 1U ], _shapeAPoints[ 0U ] );

    GXVec3 ac {};
    ac.Subtract ( _shapeAPoints[ 2U ], _shapeAPoints[ 0U ] );

    GXVec3 aNormal {};
    aNormal.CrossProduct ( ab, ac );
    aNormal.Normalize ();

    ab.Subtract ( _shapeBPoints[ 1U ], _shapeBPoints[ 0U ] );
    ac.Subtract ( _shapeBPoints[ 2U ], _shapeBPoints[ 0U ] );

    GXVec3 bNormal {};
    bNormal.CrossProduct ( ab, ac );
    bNormal.Normalize();

    SutherlandHodgmanResult const &result = _sutherlandHodgman.Run ( _shapeAPoints, aNormal, _shapeBPoints, bNormal );

    if ( result.empty () )
    {
        // Two bodies just touching, not penetrating.
        return;
    }

    auto [manifold, firstContact] = AllocateFirstContact ( contactManager, a, b, friction, restitution );
    bool isFirst = true;

    auto makeBasis = [] ( Contact &contact, GXVec3 const &aPoint, GXVec3 const &bPoint ) noexcept {
        GXVec3 &normal = contact._normal;
        normal.Subtract ( aPoint, bPoint );
        normal.Normalize ();

        GXMat3 basis {};
        basis.From ( normal );
        basis.GetX ( contact._tangent );
        basis.GetY ( contact._bitangent );
    };

    for ( auto const &[aPoint, bPoint] : result )
    {
        if ( isFirst )
        {
            firstContact->_pointA = aPoint;
            firstContact->_pointB = bPoint;
            firstContact->_penetration = aPoint.Distance ( bPoint );
            makeBasis ( *firstContact, aPoint, bPoint );
            isFirst = false;
            continue;
        }

        Contact &anotherContact = contactManager.AllocateContact ( *manifold );
        anotherContact._friction = friction;
        anotherContact._restitution = restitution;
        anotherContact._pointA = aPoint;
        anotherContact._pointB = bPoint;
        anotherContact._penetration = aPoint.Distance ( bPoint );
        makeBasis ( anotherContact, aPoint, bPoint );
    }

    contactManager.Warm ( *manifold );
}

void ContactDetector::ManifoldPoint ( ContactManager &contactManager,
    RigidBodyRef const &a,
    RigidBodyRef const &b,
    GXMat3 const &tbn,
    float friction,
    float restitution,
    GXVec3 const &vertex
) noexcept
{
    auto [manifold, contact] = AllocateFirstContact ( contactManager, a, b, friction, restitution );

    tbn.GetX ( contact->_tangent );
    tbn.GetY ( contact->_bitangent );
    tbn.GetZ ( contact->_normal );

    contact->_penetration = _epa.GetDepth ();
    contact->_pointA = vertex;
    contact->_pointB.Sum ( contact->_pointA, -contact->_penetration, contact->_normal );
    contactManager.Warm ( *manifold );
}

[[maybe_unused]] void ContactDetector::NotifyEPAFail () noexcept
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

void ContactDetector::AppendExtremePoint ( Vertices &vertices,
    Shape const &shape,
    GXMat3 const &tbn,
    GXVec3 const &ray
) noexcept
{
    GXVec3 d {};
    tbn.MultiplyVectorMatrix ( d, ray );
    GXVec3 const p = shape.GetExtremePointWorld ( d );
    auto const end = vertices.crend ();

    auto const findResult = std::find_if ( vertices.crbegin (),
        end,
        [ & ] ( GXVec3 const &v ) noexcept -> bool {
            return std::memcmp ( &p, &v, sizeof ( p ) ) == 0;
        }
    );

    if ( findResult == end )
    {
        // Unique vertex was found.
        vertices.push_back ( p );
    }
}

} // namespace android_vulkan
