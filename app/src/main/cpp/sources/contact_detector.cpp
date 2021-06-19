#include <contact_detector.h>
#include <logger.h>


namespace android_vulkan {

constexpr static size_t const INITIAL_SHAPE_POINTS = 16U;
constexpr static uint16_t const RAY_COUNT = 5U;
constexpr static float const RAY_DEVIATION_DEGREES = 6.0F;

ContactDetector::ContactDetector () noexcept:
    _epa {},
    _gjk {},
    _rays {},
    _shapeAPoints {},
    _shapeBPoints {}
{
    _shapeAPoints.reserve ( INITIAL_SHAPE_POINTS );
    _shapeAPoints.reserve ( INITIAL_SHAPE_POINTS );

    GenerateRays ();
}

void ContactDetector::Check ( RigidBodyRef const &a, RigidBodyRef const &b, ContactManager &contactManager ) noexcept
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
    GXVec3 direction ( _epa.GetNormal () );
    tbn.From ( direction );
    CollectExtremePoints ( _shapeAPoints, shapeA, tbn );
    size_t const aCount = _shapeAPoints.size ();

    if ( aCount == 1U )
    {
        ManifoldPoint ( contactManager, a, b, tbn, _shapeAPoints.front () );
        return;
    }

    direction.Reverse ();
    tbn.SetZ ( direction );
    CollectExtremePoints ( _shapeBPoints, shapeB, tbn );
    size_t const bCount = _shapeBPoints.size ();

    if ( bCount == 1U )
    {
        ManifoldPoint ( contactManager, b, a, tbn, _shapeBPoints.front () );
        return;
    }

    if ( aCount == 2U && bCount == 2U )
    {
        ManifoldEdgeEdge ( contactManager, a, b, tbn );
        return;
    }

    // TODO
}

void ContactDetector::CollectExtremePoints ( Vertices &vertices, Shape const &shape, GXMat3 const &tbn ) noexcept
{
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
    ContactManifold& manifold = contactManager.AllocateContactManifold ();
    manifold._bodyA = a;
    manifold._bodyB = b;
    manifold._epaSteps = _epa.GetSteps ();
    manifold._gjkSteps = _gjk.GetSteps ();

    Contact& contact = contactManager.AllocateContact ( manifold );
    contact._penetration = _epa.GetDepth ();
    tbn.GetX ( contact._tangent );
    tbn.GetY ( contact._bitangent );
    tbn.GetZ ( contact._normal );

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
    project ( projA, _shapeAPoints, contact._tangent, contact._bitangent );

    GXVec2 projB[ 2U ] {};
    project ( projB, _shapeBPoints, contact._tangent, contact._bitangent );

    GXVec2 alpha {};
    alpha.Subtract ( projA[ 1U ], projA[ 0U ] );

    GXVec2 beta {};
    beta.Subtract ( projB[ 1U ], projB[ 0U ] );

    GXVec2 const n ( -alpha._data[ 1U ], alpha._data[ 0U ] );
    float const gamma = n.DotProduct ( beta );

    if ( gamma == 0.0F )
    {
        // The edges either lie on each other or we need to change projection plane to find one point intersection.
        // TODO
        return;
    }

    // The edges have one point intersection.

    GXVec2 ba {};
    ba.Subtract ( projA[ 0U ], projB[ 0U ] );

    GXVec3 const& bStart = _shapeBPoints[ 0U ];

    GXVec3 v {};
    v.Subtract ( _shapeBPoints[ 1U ], bStart );

    contact._point.Sum ( bStart, ba.DotProduct ( n ) / gamma, v );
}

[[maybe_unused]] void ContactDetector::ManifoldEdgeFace ( ContactManager &/*contactManager*/,
    RigidBodyRef const &/*a*/,
    RigidBodyRef const &/*b*/,
    GXMat3 const &/*tbn*/,
    Vertices &/*aVertices*/,
    Vertices &/*bVertices*/
) noexcept
{
    // TODO
    LogDebug ( "ContactDetector::ManifoldEdgeFace - From %p", this );
}

[[maybe_unused]] void ContactDetector::ManifoldFaceFace ( ContactManager &/*contactManager*/,
    RigidBodyRef const &/*a*/,
    RigidBodyRef const &/*b*/,
    GXMat3 const &/*tbn*/
) noexcept
{
    // TODO
    LogDebug ( "ContactDetector::ManifoldFaceFace - From %p", this );
}

void ContactDetector::ManifoldPoint ( ContactManager &contactManager,
    RigidBodyRef const &a,
    RigidBodyRef const &b,
    GXMat3 const &tbn,
    GXVec3 const &vertex
) noexcept
{
    // TODO swap if needed.

    ContactManifold& manifold = contactManager.AllocateContactManifold ();
    manifold._bodyA = a;
    manifold._bodyB = b;
    manifold._epaSteps = _epa.GetSteps ();
    manifold._gjkSteps = _gjk.GetSteps ();

    Contact& contact = contactManager.AllocateContact ( manifold );
    contact._point = vertex;
    contact._penetration = _epa.GetDepth ();
    tbn.GetX ( contact._tangent );
    tbn.GetY ( contact._bitangent );
    tbn.GetZ ( contact._normal );
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

} // namespace android_vulkan
