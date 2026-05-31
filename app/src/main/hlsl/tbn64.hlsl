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


#endif // TBN_64_HLSL
