#ifndef SPD_HANDLE_MIP_9_CS
#define SPD_HANDLE_MIP_9_CS


#include "spd_common.cs"


[[vk::constant_id ( CONST_MIP_8_W )]]
const uint32_t      g_Mip8W = 4U;

[[vk::constant_id ( CONST_MIP_8_H )]]
const uint32_t      g_Mip8H = 1U;

//----------------------------------------------------------------------------------------------------------------------

// See <repo>/docs/spd-algorithm.md#mip-2
void DownsampleMip8 ( in uint32_t x, in uint32_t y, in uint32_t threadID )
{
    if ( threadID >= 64U )
        return;

    const uint32_t2 xy = uint32_t2 ( x, y );
    const uint32_t2 base = xy + xy;

    const float16_t v = ReduceIntermediateStrict ( base,
        uint32_t2 ( g_Mip7W, g_Mip7H ),
        base,
        base + uint32_t2 ( 1U, 0U ),
        base + uint32_t2 ( 0U, 1U ),
        base + uint32_t2 ( 1U, 1U )
    );

    StoreIntermediate ( base.x + y % 2U, base.y, v );
}

// See <repo>/docs/spd-algorithm.md#mip-3
void DownsampleMip9Last ( in uint32_t x, in uint32_t y, in uint32_t threadID )
{
    if ( threadID > 0U )
        return;

    const float16_t v = ReduceIntermediateStrict ( uint32_t2 ( 0U, 0U ),
        uint32_t2 ( g_Mip8W, g_Mip8H ),
        uint32_t2 ( 0U, 0U ),
        uint32_t2 ( 2U, 0U ),
        uint32_t2 ( 1U, 2U ),
        uint32_t2 ( 3U, 2U )
    );

    g_Brightness[ 0U ] = (float32_t)v;
}


#endif // SPD_HANDLE_MIP_9_CS
