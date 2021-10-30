#include <gjk.h>
#include <logger.h>


namespace android_vulkan {

constexpr static uint16_t const MAXIMUM_STEPS = 16U;
constexpr static GXVec3 const INITIAL_DIRECTION ( 0.0F, 0.0F, 1.0F );

//----------------------------------------------------------------------------------------------------------------------

void GJK::Reset () noexcept
{
    ResetInternal ( INITIAL_DIRECTION );
}

bool GJK::Run ( Shape const &shapeA, Shape const &shapeB ) noexcept
{
    GXVec3 supportPoint = Shape::FindSupportPoint ( _direction, shapeA, shapeB );
    _simplex.PushPoint ( supportPoint );

    _direction = supportPoint;
    _direction.Reverse ();

    for ( _steps = 1U; _steps < MAXIMUM_STEPS; ++_steps )
    {
        supportPoint = Shape::FindSupportPoint ( _direction, shapeA, shapeB );

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

    // Example 2021-07-20: Box shape vs sphere shape could easily exceed number of iterations. Slight penetration
    // could exceed over 1024 iterations. So it's normal for GJK to not detect small penetration contacts
    // for some shape combinations.

    constexpr char const format[] =
R"__(GJK::Run - Algorithm exceeded maximum steps. Counters:
    _steps: %hu
    _testLine: %hu
    _testTriangle: %hu
    _testTetrahedron: %hu)__";

    LogWarning ( format, _steps, _testLine, _testTriangle, _testTetrahedron );
    return false;
}

} // namespace android_vulkan
