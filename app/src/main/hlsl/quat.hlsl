#ifndef QUAT_HLSL
#define QUAT_HLSL



using QuatF = float32_t4;
using QuatH = float16_t4;

//----------------------------------------------------------------------------------------------------------------------

QuatH Recover ( in float16_t3 abc )
{
    // Real part of quaternion must be restored. By convention it's used unit quaternions to represent TBN.
    // So the real component will be restored using this property.
    // Note dot product could be a little bit bigger than 1.0H due to float16_t inaccuracy. Fixing it with abs.
    return QuatH ( sqrt ( abs ( 1.0H - dot ( abc, abc ) ) ), abc );
}

QuatH Rotate ( in QuatH a, in QuatH b )
{
    return QuatH (
        dot ( b, float16_t4 ( a.x, -a.yzw ) ),
        dot ( b, float16_t4 ( a.yxw, -a.z ) ),
        dot ( b, float16_t4 ( a.z, -a.w, a.xy ) ),
        dot ( b, float16_t4 ( a.wz, -a.y, a.x ) )
    );
}

QuatH ToQuat ( in float16_t3x3 m )
{
    float16_t3 const d = float16_t3 ( m[ 0U ][ 0U ], m[ 1U ][ 1U ], m[ 2U ][ 2U ] );

    float16_t4 solutions = float16_t4 ( d.x, d.x, -d.x, -d.x );
    solutions += float16_t4 ( d.y, -d.y, d.y, -d.y );
    solutions += float16_t4 ( d.z, -d.z, -d.z, d.z ) + 1.0H;

    float16_t const max0 = max ( solutions.x, solutions.y );
    float16_t const max1 = max ( solutions.z, solutions.w );
    float16_t const w = max ( max0, max1 );

    float16_t const phi = 0.5H * sqrt ( w );
    float16_t const omega = 0.25H / phi;

    if ( w == solutions.x )
    {
        float16_t3 const a = float16_t3 ( m[ 1U ][ 2U ], m[ 2U ][ 0U ], m[ 0U ][ 1U ] );
        float16_t3 const b = a - float16_t3 ( m[ 2U ][ 1U ], m[ 0U ][ 2U ], m[ 1U ][ 0U ] );
        return QuatH ( phi, b * omega );
    }

    if ( w == solutions.y )
    {
        float16_t3 const a = float16_t3 ( m[ 1U ][ 2U ], m[ 0U ][ 1U ], m[ 0U ][ 2U ] );
        float16_t3 const b = ( a + float16_t3 ( -m[ 2U ][ 1U ], m[ 1U ][ 0U ], m[ 2U ][ 0U ] ) ) * omega;
        return QuatH ( b.x, phi, b.yz );
    }

    if ( w == solutions.z )
    {
        float16_t3 const a = float16_t3 ( m[ 2U ][ 0U ], m[ 0U ][ 1U ], m[ 1U ][ 2U ] );
        float16_t3 const b = ( a + float16_t3 ( -m[ 0U ][ 2U ], m[ 1U ][ 0U ], m[ 2U ][ 1U ] ) ) * omega;
        return QuatH ( b.xy, phi, b.z );
    }

    float16_t3 const a = float16_t3 ( m[ 0U ][ 1U ], m[ 0U ][ 2U ], m[ 1U ][ 2U ] );
    float16_t3 const b = a + float16_t3 ( -m[ 1U ][ 0U ], m[ 2U ][ 0U ], m[ 2U ][ 1U ] );
    return QuatH ( b * omega, phi );
}


#endif // QUAT_HLSL
