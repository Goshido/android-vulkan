#ifndef TBN_32_HLSL
#define TBN_32_HLSL


#include "quat.hlsl"


using TBN32 = uint32_t;
using TBN32F = float32_t4;

//----------------------------------------------------------------------------------------------------------------------

TBN32 Compress ( in QuatH q, in TBN32 oldCompressedTBN )
{
    if ( q.x < 0.0H )
        q.yzw = -q.yzw;

    uint32_t3 unormData = uint32_t3 ( mad ( q.yzw, 511.0H, 512.0H ) );
    unormData.xy <<= uint32_t2 ( 20U, 10U );
    return ( oldCompressedTBN & 0xC0000000U ) | unormData.x | unormData.y | unormData.z;
}

QuatH ToQuat ( in TBN32 tbn )
{
    uint32_t3 comp = (uint32_t3)tbn & uint32_t3 ( 0x3FF00000U, 0x000FFC00U, 0x000003FFU );
    comp.xy >>= uint32_t2 ( 20U, 10U );

    // 2 / ( ( 2 ^ 10 ) - 1 ) = 1.955034213098729227761485826e-3
    return Recover ( mad ( (float16_t3)comp, 1.955e-3H, -1.0H ) );
}


#endif // TBN_32_HLSL
