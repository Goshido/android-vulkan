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

    StoreStrict ( xy, v, 7U, uint32_t2 ( g_Mip8W, g_Mip8H ) );
    StoreIntermediate ( base.x + y % 2U, base.y, v );
}

// See <repo>/docs/spd-algorithm.md#mip-3
void DownsampleMip9Last ( in uint32_t x, in uint32_t y, in uint32_t threadID )
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

    // It's last mip level. So resolution is 1x1.
    StoreStrict ( xy, v, 8U, uint32_t2 ( 1U, 1U ) );
}


#endif // SPD_HANDLE_MIP_9_CS
