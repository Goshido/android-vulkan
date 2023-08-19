#ifndef SPD_HANDLE_MIP_10_CS
#define SPD_HANDLE_MIP_10_CS


#include "spd_handle_mip_9.cs"


[[vk::constant_id ( CONST_MIP_9_W )]]
const uint32_t      g_Mip9W = 2U;

[[vk::constant_id ( CONST_MIP_9_H )]]
const uint32_t      g_Mip9H = 1U;

//----------------------------------------------------------------------------------------------------------------------

// See <repo>/docs/spd-algorithm.md#mip-3
void DownsampleMip9 ( in uint32_t x, in uint32_t y, in uint32_t2 workGroupID, in uint32_t threadID )
{
    if ( threadID >= 16U )
        return;

    const uint32_t2 xy = uint32_t2 ( x, y );
    const uint32_t2 base = xy * 4U;

    const float16_t v = ReduceIntermediate ( base,
        base + uint32_t2 ( 2U, 0U ),
        base + uint32_t2 ( 1U, 2U ),
        base + uint32_t2 ( 3U, 2U )
    );

    StoreStrict ( workGroupID.xy * 4U + xy, v, 8U, uint32_t2 ( g_Mip9W, g_Mip9H ) );
    StoreIntermediate ( base.x + y, base.y, v );
}

// See <repo>/docs/spd-algorithm.md#mip-4
void DownsampleMip10Last ( in uint32_t x, in uint32_t y, in uint32_t2 workGroupID, in uint32_t threadID )
{
    if ( threadID >= 4U )
        return;

    const uint32_t2 xy = uint32_t2 ( x, y );
    const uint32_t yy = y + y;
    uint32_t2 alpha = xy * 8U;
    alpha.x += yy;

    const float16_t v = ReduceIntermediate ( alpha,
        alpha + uint32_t2 ( 4U, 0U ),
        alpha + uint32_t2 ( 1U, 4U ),
        alpha + uint32_t2 ( 5U, 4U )
    );

    // It's last mip level. So resolution is 1x1.
    StoreStrict ( workGroupID.xy + workGroupID.xy + xy, v, 9U, uint32_t2 ( 1U, 1U ) );
}


#endif // SPD_HANDLE_MIP_10_CS
