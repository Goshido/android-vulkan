#ifndef TBN_HLSL
#define TBN_HLSL


#include "tbn64.hlsl"


void GetNormalAndTangent ( out float16_t3 normalView, out float16_t3 tangentView, in float16_t4 tbn )
{
    float16_t3 const abc2 = tbn.yzw + tbn.yzw;

    // Note 'T' is just notation for variable separation. Nothing more.
    float16_t4 const rXrTabc2 = tbn.x * float16_t4 ( tbn.x, abc2 );
    float16_t4 const caaaXcaTbc2 = float16_t4 ( tbn.wyyy ) * float16_t4 ( tbn.wy, abc2.yz );
    float16_t2 const bXbTc2 = tbn.z * float16_t2 ( tbn.z, abc2.z );

    float16_t4 const left = float16_t4 ( rXrTabc2.w, caaaXcaTbc2.w, rXrTabc2.z, bXbTc2.y );
    float16_t4 const right = float16_t4 ( caaaXcaTbc2.z, -rXrTabc2.z, caaaXcaTbc2.w, -rXrTabc2.y );
    float16_t4 const tmp = left + right;

    // Note quaternion unpacks to matrix with column-major like behaviour.
    normalView = float16_t3 ( tmp.zw, rXrTabc2.x - caaaXcaTbc2.y - bXbTc2.x + caaaXcaTbc2.x );
    tangentView = float16_t3 ( rXrTabc2.x + caaaXcaTbc2.y - bXbTc2.x - caaaXcaTbc2.x, tmp.xy );
}

float16_t4 RecoverTBN ( in float16_t3 imaginaryTBN )
{
    // By convention xyz contains imaginary part of quaternion. The w component contains mirroring information.
    // Real part of quaternion must be restored. By convention it's used unit quaternions to represent TBN.
    // So the real component will be restored using this property.
    // Note dot product could be a little bit bigger than 1.0H due to float16_t inaccuracy. Fixing it with abs.
    return float16_t4 ( sqrt ( abs ( 1.0H - dot ( imaginaryTBN, imaginaryTBN ) ) ), imaginaryTBN );
}

float16_t4 DecompressTBN32 ( in uint32_t compressedTBN )
{
    uint32_t3 comp = (uint32_t3)compressedTBN & uint32_t3 ( 0x3FF00000U, 0x000FFC00U, 0x000003FFU );
    comp.xy >>= uint32_t2 ( 20U, 10U );

    // 2 / ( ( 2 ^ 10 ) - 1 ) = 1.955034213098729227761485826e-3
    return RecoverTBN ( mad ( (float16_t3)comp, 1.955e-3H, -1.0H ) );
}

float16_t4 DecompressTBN64 ( in TBN64 tbn64, in uint32_t idx )
{
    uint32_t const highCases[ 2U ] = { tbn64._q0High, tbn64._q1High };
    uint32_t const lowCases[ 2U ] = { tbn64._q0Low, tbn64._q1Low };

    uint32_t2 const lowHigh = uint32_t2 ( lowCases[ idx ], highCases[ idx ] );
    uint16_t2 const shifted = (uint16_t2)( lowHigh >> 16U );
    uint16_t2 const masked = (uint16_t2)( lowHigh & 0x0000FFFFU );

    // 2.0 / ( 2 ^ 16 - 1 ) = 3.0518043793392843518730449378195e-5
    return (float16_t4)mad ( (float32_t4)uint16_t4 ( shifted.x, masked.x, shifted.y, masked.y ), 3.0518e-5F, -1.0F );
}

uint32_t CompressTBN32 ( in float16_t4 tbn, in uint32_t oldCompressedTBN )
{
    if ( tbn.x < 0.0H )
        tbn.yzw = -tbn.yzw;

    uint32_t3 unormData = uint32_t3 ( mad ( tbn.yzw, 511.0H, 512.0H ) );
    unormData.xy <<= uint32_t2 ( 20U, 10U );
    return ( oldCompressedTBN & 0xC0000000U ) | unormData.x | unormData.y | unormData.z;
}

float16_t4 RotateTBN ( in float16_t4 tbn, in float16_t4 quaternion )
{
    return float16_t4 (
        dot ( quaternion, float16_t4 ( tbn.x, -tbn.yzw ) ),
        dot ( quaternion, float16_t4 ( tbn.yxw, -tbn.z ) ),
        dot ( quaternion, float16_t4 ( tbn.z, -tbn.w, tbn.xy ) ),
        dot ( quaternion, float16_t4 ( tbn.wz, -tbn.y, tbn.x ) )
    );
}

float16_t4 ToTBN ( in float16_t3x3 m )
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
        return float16_t4 ( phi, b * omega );
    }

    if ( w == solutions.y )
    {
        float16_t3 const a = float16_t3 ( m[ 1U ][ 2U ], m[ 0U ][ 1U ], m[ 0U ][ 2U ] );
        float16_t3 const b = ( a + float16_t3 ( -m[ 2U ][ 1U ], m[ 1U ][ 0U ], m[ 2U ][ 0U ] ) ) * omega;
        return float16_t4 ( b.x, phi, b.yz );
    }

    if ( w == solutions.z )
    {
        float16_t3 const a = float16_t3 ( m[ 2U ][ 0U ], m[ 0U ][ 1U ], m[ 1U ][ 2U ] );
        float16_t3 const b = ( a + float16_t3 ( -m[ 0U ][ 2U ], m[ 1U ][ 0U ], m[ 2U ][ 1U ] ) ) * omega;
        return float16_t4 ( b.xy, phi, b.z );
    }

    float16_t3 const a = float16_t3 ( m[ 0U ][ 1U ], m[ 0U ][ 2U ], m[ 1U ][ 2U ] );
    float16_t3 const b = a + float16_t3 ( -m[ 1U ][ 0U ], m[ 2U ][ 0U ], m[ 2U ][ 1U ] );
    return float16_t4 ( b * omega, phi );
}


#endif // TBN_HLSL
