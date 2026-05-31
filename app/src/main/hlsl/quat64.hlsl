#ifndef QUAT_64_HLSL
#define QUAT_64_HLSL


#include "quat.hlsl"


struct Quat64x2
{
    uint32_t    _q0High;
    uint32_t    _q0Low;
    uint32_t    _q1High;
    uint32_t    _q1Low;
};

//----------------------------------------------------------------------------------------------------------------------

QuatH Decompress ( in Quat64x2 tbn64, in uint32_t idx )
{
    uint32_t const highCases[ 2U ] = { tbn64._q0High, tbn64._q1High };
    uint32_t const lowCases[ 2U ] = { tbn64._q0Low, tbn64._q1Low };

    uint32_t2 const lowHigh = uint32_t2 ( lowCases[ idx ], highCases[ idx ] );
    uint16_t2 const shifted = (uint16_t2)( lowHigh >> 16U );
    uint16_t2 const masked = (uint16_t2)( lowHigh & 0x0000FFFFU );

    // 2.0 / ( 2 ^ 16 - 1 ) = 3.0518043793392843518730449378195e-5
    return (QuatH)mad ( (float32_t4)uint16_t4 ( shifted.x, masked.x, shifted.y, masked.y ), 3.0518e-5F, -1.0F );
}


#endif // QUAT_64_HLSL
