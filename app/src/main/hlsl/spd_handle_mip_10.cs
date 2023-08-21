#ifndef SPD_HANDLE_MIP_10_CS
#define SPD_HANDLE_MIP_10_CS


#include "spd_handle_mip_9.cs"


[[vk::constant_id ( CONST_MIP_9_W )]]
const uint32_t      g_Mip9W = 2U;

[[vk::constant_id ( CONST_MIP_9_H )]]
const uint32_t      g_Mip9H = 1U;

//----------------------------------------------------------------------------------------------------------------------

// See <repo>/docs/spd-algorithm.md#mip-3
void DownsampleMip9 ( in uint32_t x, in uint32_t y, in uint32_t threadID )
{
    if ( threadID >= 16U )
        return;

    const uint32_t2 xy = uint32_t2 ( x, y );
    const uint32_t2 resolution = xy + xy;
    const uint32_t2 base = xy * 4U;

    const float16_t v = ReduceIntermediateStrict ( resolution,
        uint32_t2 ( g_Mip8W, g_Mip8H ),
        base,
        base + uint32_t2 ( 2U, 0U ),
        base + uint32_t2 ( 1U, 2U ),
        base + uint32_t2 ( 3U, 2U )
    );

    StoreIntermediate ( base.x + y, base.y, v );
}

// See <repo>/docs/spd-algorithm.md#mip-4
void DownsampleMip10Last ( in uint32_t x, in uint32_t y, in uint32_t threadID )
{
    if ( threadID > 0U )
        return;

    const float16_t v = ReduceIntermediateStrict ( uint32_t2 ( 0U, 0U ),
        uint32_t2 ( g_Mip9W, g_Mip9H ),
        uint32_t2 ( 0U, 0U ),
        uint32_t2 ( 4U, 0U ),
        uint32_t2 ( 1U, 4U ),
        uint32_t2 ( 5U, 4U )
    );

    g_Brightness[ 0U ] = (float32_t)v;
}


#endif // SPD_HANDLE_MIP_10_CS
