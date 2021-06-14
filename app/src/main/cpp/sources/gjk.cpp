#include <gjk.h>
#include <logger.h>


namespace android_vulkan {

constexpr static uint16_t const MAXIMUM_STEPS = 16U;
constexpr static GXVec3 const INITIAL_DIRECTION ( 0.0F, 0.0F, 1.0F );

GJK::GJK () noexcept:
    _direction ( INITIAL_DIRECTION ),
    _simplex {},
    _steps ( 0U ),
    _testLine ( 0U ),
    _testTetrahedron ( 0U ),
    _testTriangle ( 0U )
{
    // NOTHING
}

[[maybe_unused]] uint16_t GJK::GetSteps () const noexcept
{
    return _steps;
}

[[maybe_unused]] uint16_t GJK::GetTestLines () const noexcept
{
    return _testLine;
}

[[maybe_unused]] uint16_t GJK::GetTestTetrahedrons () const noexcept
{
    return _testTetrahedron;
}

[[maybe_unused]] uint16_t GJK::GetTestTriangles () const noexcept
{
    return _testTriangle;
}

bool GJK::Run ( Shape &shapeA, Shape &shapeB ) noexcept
{
    GXVec3 supportPoint = Simplex::FindSupportPoint ( _direction, shapeA, shapeB );
    _simplex.PushPoint ( supportPoint );

    _direction = supportPoint;
    _direction.Reverse ();

    for ( _steps = 1U; _steps < MAXIMUM_STEPS; ++_steps )
    {
        supportPoint = Simplex::FindSupportPoint ( _direction, shapeA, shapeB );

        if ( supportPoint.DotProduct ( _direction ) < 0.0F )
            return false;

        _simplex.PushPoint ( supportPoint );

        switch ( _simplex._pointCount )
        {
            case 2U:
            {
                LineTest ();
            }
            break;

            case 3U:
            {
                TriangleTest ();
            }
            break;

            case 4U:
            {
                if ( TetrahedronTest () )
                {
                    return true;
                }
            }
            break;

            default:
                // IMPOSSIBLE
            break;
        }
    }

    constexpr char const format[] =
R"__(GJK::Run - Algorithm exceeded maximum steps. Counters:
    _steps: %hhu
    _testLine: %hhu
    _testTriangle: %hhu
    _testTetrahedron: %hhu)__";

    LogWarning ( format, _steps, _testLine, _testTriangle, _testTetrahedron );
    return false;
}

void GJK::LineTest () noexcept
{
    ++_testLine;

    GXVec3 const& a = _simplex._supportPoints[ 0U ];
    GXVec3 const& b = _simplex._supportPoints[ 1U ];

    GXVec3 ab {};
    ab.Subtract ( b, a );       // NOLINT

    // "o" means origin.
    GXVec3 ao ( a );
    ao.Reverse ();

    if ( ab.DotProduct ( ao ) > 0.0F )
    {
        // The closest point to the origin is in somewhere in line segment AB.
        // The direction is a perpendicular towards to the origin from line segment AB.
        GXVec3 ortho {};
        ortho.CrossProduct ( ab, ao );
        _direction.CrossProduct ( ortho, ab );
        return;
    }

    // The last added point is the closest to the origin.
    // The direction is AO.
    // Make simplex to contain only last added point.
    _simplex._pointCount = 1U;
    _direction = ao;
}

bool GJK::TetrahedronTest () noexcept
{
    ++_testTetrahedron;

    GXVec3 const& a = _simplex._supportPoints[ 0U ];
    GXVec3 const& b = _simplex._supportPoints[ 1U ];
    GXVec3 const& c = _simplex._supportPoints[ 2U ];
    GXVec3 const& d = _simplex._supportPoints[ 3U ];

    // "o" means origin.
    GXVec3 ao ( a );
    ao.Reverse ();

    // The idea is to determine which of the 3 triangles is closest to the origin.
    // A little insight: on previous step we already checked BCD triangle. So it's needed to check:
    //      1) ABD
    //      2) ACB
    //      3) ADC
    // The winding order is consistent and carefully managed on triangle test steps.

    GXVec3 ab {};
    // Android Studio treats next instruction as incorrect parameter order mistake.
    // NOLINTNEXTLINE
    ab.Subtract ( b, a );

    GXVec3 ad {};
    ad.Subtract ( d, a );

    GXVec3 abd {};
    abd.CrossProduct ( ab, ad );

    if ( abd.DotProduct ( ao ) < 0.0F )
    {
        // Note: Previous tests already done with BCD triangle. So BD line is already inspected.
        // Because of that the order could be: [A, B, D] or [A, D, B].
        // Selecting [A, B, D].
        _simplex._pointCount = 3U;
        _simplex._supportPoints[ 2U ] = d;
        TriangleTest ();
        return false;
    }

    GXVec3 ac {};
    ac.Subtract ( c, a );

    GXVec3 acb {};
    acb.CrossProduct ( ac, ab );

    if ( acb.DotProduct ( ao ) < 0.0F )
    {
        // Note: Previous tests already done with BCD triangle. So BC line is already inspected.
        // Because of that the order could be: [A, B, C] or [A, C, B].
        // Selecting [A, B, C].
        _simplex._pointCount = 3U;
        TriangleTest ();
        return false;
    }

    GXVec3 adc {};
    adc.CrossProduct ( ad, ac );

    if ( adc.DotProduct ( ao ) >= 0.0F )
    {
        // The origin inside the simplex. Two shapes are intersecting.
        return true;
    }

    // Note: Previous tests already done with BCD triangle. So CD line is already inspected.
    // Because of that the order could be: [A, C, D] or [A, D, C].
    // Selecting [A, C, D].
    _simplex._supportPoints[ 1U ] = c;
    _simplex._supportPoints[ 2U ] = d;
    _simplex._pointCount = 3U;
    TriangleTest ();
    return false;
}

void GJK::TriangleTest () noexcept
{
    ++_testTriangle;

    GXVec3 const& a = _simplex._supportPoints[ 0U ];
    GXVec3& b = _simplex._supportPoints[ 1U ];
    GXVec3& c = _simplex._supportPoints[ 2U ];

    // "o" means origin.
    GXVec3 ao ( a );
    ao.Reverse ();

    GXVec3 ab {};
    // Android Studio treats next instruction as incorrect parameter order mistake.
    // NOLINTNEXTLINE
    ab.Subtract ( b, a );

    GXVec3 ac {};
    ac.Subtract ( c, a );

    GXVec3 abc {};
    abc.CrossProduct ( ab, ac );

    GXVec3 edgeNormal {};
    edgeNormal.CrossProduct ( abc, ac );

    auto commonCase = [ & ] () noexcept {
        if ( ab.DotProduct ( ao ) <= 0.0F )
        {
            // A point simplex. The direction is AO.
            _simplex._pointCount = 1U;
            _direction = ao;
            return;
        }

        // The closest point to the origin is in somewhere in line segment AB.
        // The direction is a perpendicular towards to the origin from line segment AB.
        _simplex._pointCount = 2U;
        GXVec3 ortho {};
        ortho.CrossProduct ( ab, ao );
        _direction.CrossProduct ( ortho, ab );
    };

    if ( edgeNormal.DotProduct ( ao ) > 0.0F )
    {
        if ( ac.DotProduct ( ao ) <= 0.0F )
        {
            commonCase ();
            return;
        }

        // AC line simplex.
        _simplex._pointCount = 2U;
        _simplex._supportPoints[ 1U ] = c;

        GXVec3 ortho {};
        ortho.CrossProduct ( ac, ao );
        _direction.CrossProduct ( ortho, ac );

        return;
    }

    edgeNormal.CrossProduct ( ab, abc );

    if ( edgeNormal.DotProduct ( ao ) > 0.0F )
    {
        commonCase ();
        return;
    }

    // Triangle simplex cases.
    _direction = abc;

    if ( abc.DotProduct ( ao ) > 0.0F )
    {
        // The origin in front of the triangle.
        return;
    }

    // The origin is behind of the triangle.
    // So the direction is opposite to triangle normal.
    // In order to work in tetrahedron test we must rewind the triangle.
    // Reusing "abc" variable to make swap.
    abc = b;
    b = c;
    c = abc;
    _direction.Reverse ();
}

} // namespace android_vulkan
