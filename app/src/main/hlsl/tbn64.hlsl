#ifndef TBN_64_HLSL
#define TBN_64_HLSL


#include "quat.hlsl"


struct TBN64
{
    uint64_t    _a: 21;
    uint64_t    _b: 21;
    uint64_t    _c: 22;
};

//----------------------------------------------------------------------------------------------------------------------

QuatH ToQuat ( in TBN64 tbn )
{
    uint32_t3 const comp = uint32_t3 ( (uint32_t)tbn._a, (uint32_t)tbn._b, (uint32_t)tbn._c );

    // 2 / ( 2 ^ 21 - 1 ) = 9.5367477115381772700201368427929e-7F
    // 2 / ( 2 ^ 22 - 1 ) = 4.7683727188998982667680422706705e-7F
    return Recover (
        (float16_t3)mad ( (float32_t3)comp, float32_t3 ( 9.53675e-7F, 9.53675e-7F, 4.76837e-7F ), -1.0F )
    );
}

float32_t3x3 ToMatrix ( in TBN64 tbn )
{
    // 2 / ( 2 ^ 21 - 1 ) = 9.5367477115381772700201368427929e-7F
    // 2 / ( 2 ^ 22 - 1 ) = 4.7683727188998982667680422706705e-7F

    uint32_t3 const comp = uint32_t3 ( (uint32_t)tbn._a, (uint32_t)tbn._b, (uint32_t)tbn._c );
    float32_t3 const imaginary = mad ( (float32_t3)comp, float32_t3 ( 9.53675e-7F, 9.53675e-7F, 4.76837e-7F ), -1.0F );

    // By convention xyz contains imaginary part of quaternion.
    // Real part of quaternion must be restored. By convention it's used unit quaternions to represent rotation.
    // So the real component will be restored using this property.
    // Note dot product could be a little bit bigger than 1.0F due to float32_t inaccuracy. Fixing it with abs.
    float32_t4 const rabc = float32_t4 ( sqrt ( abs ( 1.0F - dot ( imaginary, imaginary ) ) ), imaginary );

    float32_t3 const abc2 = rabc.yzw + rabc.yzw;

    // Note 'T' is just notation for variable separation. Nothing more.
    float32_t4 const rXrTabc2 = rabc.x * float32_t4 ( rabc.x, abc2 );
    float32_t4 const caaaXcaTbc2 = float32_t4 ( rabc.wyyy ) * float32_t4 ( rabc.wy, abc2.yz );
    float32_t2 const bXbTc2 = rabc.z * float32_t2 ( rabc.z, abc2.z );

    float32_t4 const left0 = float32_t4 ( rXrTabc2.w, caaaXcaTbc2.w, caaaXcaTbc2.z, rXrTabc2.y );
    float32_t4 const right0 = float32_t4 ( caaaXcaTbc2.z, -rXrTabc2.z, -rXrTabc2.w, bXbTc2.y );

    float32_t2 const tmp1 = float32_t2 ( rXrTabc2.z, bXbTc2.y ) + float32_t2 ( caaaXcaTbc2.w, -rXrTabc2.y );
    float32_t4 const tmp0 = left0 + right0;

    // Note quaternion unpacks to matrix with column-major like behavior.
    return float32_t3x3
    (
        // First row.
        rXrTabc2.x + caaaXcaTbc2.y - bXbTc2.x - caaaXcaTbc2.x,
        tmp0.x,
        tmp0.y,

        // Second row.
        tmp0.z,
        rXrTabc2.x - caaaXcaTbc2.y + bXbTc2.x - caaaXcaTbc2.x,
        tmp0.w,

        // Third row.
        tmp1.x,
        tmp1.y,
        rXrTabc2.x - caaaXcaTbc2.y - bXbTc2.x + caaaXcaTbc2.x
    );
}


#endif // TBN_64_HLSL
