#include <ray_caster.h>
#include <logger.h>
#include <simplex.h>


namespace android_vulkan {

constexpr uint16_t const MAXIMUM_STEPS = 32U;
constexpr static float const SAME_POINT_TOLERANCE = 5.0e-4F;
constexpr float const THRESHOLD = 1.0e-3F;

//----------------------------------------------------------------------------------------------------------------------

bool RayCaster::Run ( RaycastResult& result, GXVec3 const &from, GXVec3 const &to, Shape const &shape ) noexcept
{
    constexpr GXVec3 const initialDirection ( 1.0F, 0.0F, 0.0F );
    ResetInternal ();

    float lambda = 0.0F;

    GXVec3 v {};
    v.Subtract ( from, shape.GetExtremePointWorld ( initialDirection ) );

    GXVec3 r {};
    r.Subtract ( to, from );

    result._point = from;

    for ( ; _steps < MAXIMUM_STEPS; ++_steps )
    {
        float const distance = v.SquaredLength ();
        constexpr float const threshold = THRESHOLD * THRESHOLD;

        if ( distance < threshold )
            break;

        // Note: Ray is modeled as point which is equal to current origin.
        // Because it's a point the extreme point in any direction will be that point by definition.

        GXVec3 w {};
        w.Subtract ( result._point, shape.GetExtremePointWorld ( v ) );
        float const vwDot = v.DotProduct ( w );

        if ( vwDot > 0.0F )
        {
            float const vrDot = v.DotProduct ( r );

            if ( vrDot >= 0.0F )
                return false;

            lambda -= vwDot / vrDot;

            if ( lambda > 1.0F )
                return false;

            result._point.Sum ( from, lambda, r );
            result._normal = v;
        }

        bool isPresented = false;

        for ( uint8_t i = 0U; i < _simplex._pointCount; ++i )
        {
            constexpr float const samePoint = SAME_POINT_TOLERANCE * SAME_POINT_TOLERANCE;

            if ( w.SquaredDistance ( _simplex._supportPoints[ i ] ) > samePoint )
                continue;

            isPresented = true;
            break;
        }

        if ( !isPresented )
            _simplex.PushPoint ( w );

        switch ( _simplex._pointCount )
        {
            case 1U:
                v = _simplex._supportPoints[ 0U ];
            break;

            case 2U:
                v = TestLine ();
            break;

            case 3U:
                v = TestTriangle ();
            break;

            case 4U:
                v = TestTetrahedron ( v, threshold );
            break;

            default:
                // IMPOSSIBLE
            break;
        }
    }

    result._normal.Normalize ();
    return true;
}

GXVec3 RayCaster::TestLine () noexcept
{
    // The idea is to check areas of line segment in the following order:
    //  1) inside AB segment
    //  2) behind A vertex
    // Such approach will reduce searching complexity step by step.
    // Note there was 'behind B vertex check' earlier.

    ++_testLine;

    GXVec3 const& a = _simplex._supportPoints[ 0U ];
    GXVec3 const& b = _simplex._supportPoints[ 1U ];

    GXVec3 ab {};
    // NOLINTNEXTLINE - false positively operand order.
    ab.Subtract ( b, a );

    // "o" means origin.
    GXVec3 ao ( a );
    ao.Reverse ();

    // 1: Checking behind AB segment...

    if ( ab.DotProduct ( ao ) > 0.0F )
    {
        // The projection of AO onto AB is the closest point.

        ab.Normalize ();
        GXVec3 v {};
        v.Sum ( a, -ab.DotProduct ( a ), ab );
        return v;
    }

    // 2: The last added point is the closest to the origin.
    // Make simplex to contain only last added point.
    _simplex._pointCount = 1U;
    return a;
}

GXVec3 RayCaster::TestTetrahedron ( GXVec3 const &bcdClosest, float bcdClosestDistance ) noexcept
{
    // The trick is that 'BCD triangle check' has been done earlier.

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
    //  1) ABD
    //  2) ACB
    //  3) ADC
    // The winding order is consistent and carefully managed on triangle test steps.

    GXVec3 ab {};
    // NOLINTNEXTLINE - false positively operand order.
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
        return TestTriangle ();
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
        return TestTriangle ();
    }

    GXVec3 adc {};
    adc.CrossProduct ( ad, ac );

    if ( adc.DotProduct ( ao ) < 0.0F )
    {
        // Note: Previous tests already done with BCD triangle. So CD line is already inspected.
        // Because of that the order could be: [A, C, D] or [A, D, C].
        // Selecting [A, C, D].
        _simplex._supportPoints[ 1U ] = c;
        _simplex._supportPoints[ 2U ] = d;
        _simplex._pointCount = 3U;
        return TestTriangle ();
    }

    // The origin is inside the simplex. Finding the closest triangle simplex and the closest point on that triangle...
    _simplex._pointCount = 3U;

    // ABD checking...
    Simplex simplex {};
    simplex._supportPoints[ 0U ] = _simplex._supportPoints[ 0U ];
    simplex._supportPoints[ 1U ] = _simplex._supportPoints[ 1U ];
    simplex._supportPoints[ 2U ] = _simplex._supportPoints[ 3U ];

    abd.Normalize ();

    GXVec3 v {};
    // Note OA vector is just A vector.
    v.Multiply ( abd, abd.DotProduct ( a ) );

    float dist = v.SquaredLength ();

    // ACB checking...
    acb.Normalize ();

    GXVec3 checkV {};
    // Note OA vector is just A vector.
    checkV.Multiply ( acb, acb.DotProduct ( a ) );

    float checkDist = checkV.SquaredLength ();

    if ( dist > checkDist )
    {
        dist = checkDist;
        v = checkV;

        simplex._supportPoints[ 0U ] = _simplex._supportPoints[ 0U ];
        simplex._supportPoints[ 1U ] = _simplex._supportPoints[ 2U ];
        simplex._supportPoints[ 2U ] = _simplex._supportPoints[ 1U ];
    }

    // ADC checking...
    adc.Normalize ();
    checkV.Multiply ( adc, adc.DotProduct ( a ) );
    checkDist = checkV.SquaredLength ();

    if ( dist > checkDist )
    {
        v = checkV;
        dist = checkDist;

        simplex._supportPoints[ 0U ] = _simplex._supportPoints[ 0U ];
        simplex._supportPoints[ 1U ] = _simplex._supportPoints[ 3U ];
        simplex._supportPoints[ 2U ] = _simplex._supportPoints[ 2U ];
    }

    // BCD checking...
    // Note BCD is exactly previous step. So there is no need to evaluate checkV again.

    if ( bcdClosestDistance > dist )
    {
        // New closest triangle is found.
        _simplex._pointCount = 3U;
        _simplex._supportPoints[ 0U ] = simplex._supportPoints[ 0U ];
        _simplex._supportPoints[ 1U ] = simplex._supportPoints[ 1U ];
        _simplex._supportPoints[ 2U ] = simplex._supportPoints[ 2U ];

        return v;
    }

    // BCD is the closest triangle. Rebuilding the simplex.
    constexpr size_t const shift = 3U * sizeof ( GXVec3 );
    std::memmove ( _simplex._supportPoints, _simplex._supportPoints + 1U, shift );
    return bcdClosest;
}

GXVec3 RayCaster::TestTriangle () noexcept
{
    // The trick is that 'BC line segment check' has been done earlier.

    ++_testTriangle;

    GXVec3 const& a = _simplex._supportPoints[ 0U ];
    GXVec3& b = _simplex._supportPoints[ 1U ];
    GXVec3& c = _simplex._supportPoints[ 2U ];

    // "o" means origin.
    GXVec3 ao ( a );
    ao.Reverse ();

    GXVec3 ab {};
    // NOLINTNEXTLINE - false positively operand order.
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
            // A point simplex.
            _simplex._pointCount = 1U;
            return a;
        }

        // AB line segment is a new simplex.
        // The projection of AO onto AB is the closest point.

        _simplex._pointCount = 2U;

        ab.Normalize ();
        GXVec3 v {};
        v.Sum ( a, -ab.DotProduct ( a ), ab );
        return v;
    };

    if ( edgeNormal.DotProduct ( ao ) > 0.0F )
    {
        if ( ac.DotProduct ( ao ) <= 0.0F )
            return commonCase ();

        // AC line segment is a new simplex.
        // The projection of AO onto AC is the closest point.

        // AC line simplex.
        _simplex._pointCount = 2U;
        _simplex._supportPoints[ 1U ] = c;

        ac.Normalize ();
        GXVec3 v {};
        v.Sum ( a, -ac.DotProduct ( a ), ac );
        return v;
    }

    edgeNormal.CrossProduct ( ab, abc );

    if ( edgeNormal.DotProduct ( ao ) > 0.0F )
        return commonCase ();

    // Triangle simplex case.

    // The closest point is onto ABC triangle.
    abc.Normalize ();

    GXVec3 v {};
    // Note OA vector is just A vector.
    v.Multiply ( abc, abc.DotProduct ( a ) );

    if ( abc.DotProduct ( ao ) <= 0.0F )
    {
        // The origin is behind of the triangle.
        // In order to work in tetrahedron test we must rewind the triangle.
        // Reusing "abc" variable to make swap.
        abc = b;
        b = c;
        c = abc;
    }

    return v;
}

} // namespace android_vulkan
