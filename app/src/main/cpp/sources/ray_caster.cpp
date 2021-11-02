#include <ray_caster.h>
#include <simplex.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

constexpr float const THRESHOLD = 1.0e-3F;
constexpr uint16_t const MAXIMUM_STEPS = 32U;

//----------------------------------------------------------------------------------------------------------------------

RayCaster::ClosestInHullHandler const RayCaster::_handlers[ 4U ] =
{
    &RayCaster::GetClosestInHullPoint,
    &RayCaster::GetClosestInHullLine,
    &RayCaster::GetClosestInHullTriangle,
    &RayCaster::GetClosestInHullTetrahedron
};

bool RayCaster::Run ( RaycastResult& result, GXVec3 const &from, GXVec3 const &to, Shape const &shape ) noexcept
{
    constexpr GXVec3 const initialDirection ( 1.0F, 0.0F, 0.0F );
    ResetInternal ( initialDirection );

    float lambda = 0.0F;

    GXVec3 v {};
    v.Subtract ( from, shape.GetExtremePointWorld ( initialDirection ) );

    GXVec3 r {};
    r.Subtract ( to, from );

    result._point = from;
    bool hasIntersect = true;

    for ( ; _steps < MAXIMUM_STEPS; ++_steps )
    {
        if ( v.SquaredLength () < THRESHOLD )
            break;

        // Note: Ray is modeled as point of current origin.
        // Because it's a point that extreme point in any direction will be that point.

        GXVec3 w {};
        w.Subtract ( result._point, shape.GetExtremePointWorld ( v ) );

        float const vwDot = v.DotProduct ( w );

        if ( vwDot > 0.0F )
        {
            float const vrDot = v.DotProduct ( r );

            if ( vrDot >= 0.0F )
            {
                hasIntersect = false;
                break;
            }

            lambda -= vwDot / vrDot;
            result._point.Sum ( from, lambda, r );
            result._normal = v;
        }

        if ( _simplex._pointCount == 4U )
            GXVec3 const stop {};

        _simplex.PushPoint ( w );

        switch ( _simplex._pointCount )
        {
            case 2U:
                LineTest ();
            break;

            case 3U:
                TriangleTest ();
            break;

            case 4U:
                TetrahedronTest ();
            break;

            default:
                // NOTHING
            break;
        }

        assert ( _simplex._pointCount > 0U && _simplex._pointCount < 5U );

        ClosestInHullHandler const handler = _handlers[ _simplex._pointCount - 1U ];
        v = handler ( _simplex );
    }

    if ( !hasIntersect )
        return false;

    result._normal.Normalize ();
    return true;
}

GXVec3 RayCaster::GetClosestInHullLine ( Simplex const &simplex ) noexcept
{
    // The idea is to check areas of line segment in the following order:
    //  1) behind A vertex
    //  2) behind B vertex
    //  3) inside AB segment
    // Such approach will reduce searching complexity step by step.

    GXVec3 const& a = simplex._supportPoints[ 0U ];
    GXVec3 const& b = simplex._supportPoints[ 1U ];

    GXVec3 ab {};
    // NOLINTNEXTLINE - false positively operand order.
    ab.Subtract ( b, a );

    // 1: Checking behind A vertex...

    if ( ab.DotProduct ( a ) >= 0.0F )
    {
        // The A vertex is the closest point.
        return a;
    }

    // 2: Checking behind B vertex...

    if ( ab.DotProduct ( b ) <= 0.0F )
    {
        // The B vertex is the closest point.
        return b;
    }

    // 3: Closest point is onto AB segment.

    ab.Normalize ();
    GXVec3 v {};
    v.Sum ( a, -ab.DotProduct ( a ), ab );

    return v;
}

GXVec3 RayCaster::GetClosestInHullPoint ( Simplex const &simplex ) noexcept
{
    return simplex._supportPoints[ 0U ];
}

GXVec3 RayCaster::GetClosestInHullTetrahedron ( Simplex const &simplex ) noexcept
{
    // The main idea is to find closest face and process it via GetClosestInHullTriangle.

    GXVec3 const& a = simplex._supportPoints[ 0U ];
    GXVec3 const& b = simplex._supportPoints[ 1U ];
    GXVec3 const& c = simplex._supportPoints[ 2U ];
    GXVec3 const& d = simplex._supportPoints[ 3U ];

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

        Simplex s {};
        s._supportPoints[ 0U ] = a;
        s._supportPoints[ 1U ] = b;
        s._supportPoints[ 2U ] = d;

        return GetClosestInHullTriangle ( s );
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
        return GetClosestInHullTriangle ( simplex );
    }

    GXVec3 adc {};
    adc.CrossProduct ( ad, ac );

    if ( adc.DotProduct ( ao ) >= 0.0F )
    {
        // The origin inside the simplex. Any triangle can be used.
        return GetClosestInHullTriangle ( simplex );
    }

    // Note: Previous tests already done with BCD triangle. So CD line is already inspected.
    // Because of that the order could be: [A, C, D] or [A, D, C].
    // Selecting [A, C, D].

    Simplex s {};
    s._supportPoints[ 0U ] = a;
    s._supportPoints[ 1U ] = c;
    s._supportPoints[ 2U ] = d;

    return GetClosestInHullTriangle ( s );
}

GXVec3 RayCaster::GetClosestInHullTriangle ( Simplex const &simplex ) noexcept
{
    // The idea is to check areas of triangle in the following order:
    //  1) behind A vertex
    //  2) behind B vertex
    //  3) behind AB segment
    //  4) behind C vertex
    //  5) behind AC segment
    //  6) behind BC segment
    //  7) ABC half planes
    // Such approach will reduce searching complexity step by step.

    GXVec3 const& a = simplex._supportPoints[ 0U ];
    GXVec3 const& b = simplex._supportPoints[ 1U ];

    GXVec3 ab {};
    // NOLINTNEXTLINE - false positively operand order.
    ab.Subtract ( b, a );

    GXVec3 ao ( a );
    ao.Reverse ();

    GXVec3 const& c = simplex._supportPoints[ 2U ];

    GXVec3 ac {};
    ac.Subtract ( c, a );

    float const abaoDot = ab.DotProduct ( ao );
    float const acaoDot = ac.DotProduct ( ao );

    // 1: Checking behind A vertex...

    if ( abaoDot <= 0.0F && acaoDot <= 0.0F )
    {
        // The A vertex is the closest point.
        return a;
    }

    GXVec3 bo ( b );
    bo.Reverse ();

    float const abboDot = ab.DotProduct ( bo );
    float const acboDot = ac.DotProduct ( bo );

    // 2: Checking behind B vertex...

    if ( abboDot >= 0.0F && acboDot <= abboDot )
    {
        // The B vertex is the closest point.
        return b;
    }

    // 3: Checking behind AB segment...

    if ( abaoDot >= 0.0F && abboDot <= 0.0F )
    {
        // Optimization: abTest = abaoDot * acboDot - abboDot * acaoDot;
        if ( GXVec2 ( abaoDot, -abboDot ).DotProduct ( GXVec2 ( acboDot, acaoDot ) ) <= 0.0F )
        {
            // The projection of AO onto AB is the closest point.

            ab.Normalize ();
            GXVec3 v {};
            v.Sum ( a, ab.DotProduct ( ao ), ab );

            return v;
        }
    }

    GXVec3 co ( c );
    co.Reverse ();

    float const abcoDot = ab.DotProduct ( co );
    float const accoDot = ac.DotProduct ( co );

    // 4: Checking behind C vertex...

    if ( accoDot >= 0.0F && abcoDot <= accoDot )
    {
        // The C vertex is the closest point.
        return c;
    }

    // 5: Checking behind AC segment...

    if ( acaoDot >= 0.0F && accoDot <= 0.0F )
    {
        // Optimization: acTest = abcoDot * abaoDot - acaoDot * accoDot;
        if ( GXVec2 ( abcoDot, -abaoDot ).DotProduct ( GXVec2 ( acaoDot, accoDot ) ) <= 0.0F )
        {
            // The projection of AO onto AC is the closest point.

            ac.Normalize ();
            GXVec3 v {};
            v.Sum ( a, ac.DotProduct ( ao ), ac );

            return v;
        }
    }

    // 6: Checking behind BC segment...

    if ( acboDot - abboDot >= 0.0F && abcoDot - accoDot >= 0.0F )
    {
        // Optimization: bcTest = abboDot * accoDot - abcoDot * acboDot;
        if ( GXVec2 ( abboDot, -abcoDot ).DotProduct ( GXVec2 ( accoDot, acboDot ) ) <= 0.0F )
        {

            // The projection of BO onto BC is the closest point.
            GXVec3 bc {};
            bc.Subtract ( c, b );
            bc.Normalize ();

            GXVec3 v {};
            v.Sum ( b, bc.DotProduct ( bo ), bc );

            return v;
        }
    }

    // 7: The closest point is onto ABC triangle.
    GXVec3 abcNormal {};
    abcNormal.CrossProduct ( ab, ac );
    abcNormal.Normalize ();

    GXVec3 v {};
    // Note OA vector is just A vector.
    v.Multiply ( abcNormal, abcNormal.DotProduct ( a ) );
    return v;
}

} // namespace android_vulkan
