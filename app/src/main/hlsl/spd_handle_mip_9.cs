#ifndef SPD_HANDLE_MIP_9_CS
#define SPD_HANDLE_MIP_9_CS


#include "spd_common.cs"


[[vk::constant_id ( CONST_MIP_8_W )]]
const uint32_t      g_Mip8W = 4U;

[[vk::constant_id ( CONST_MIP_8_H )]]
const uint32_t      g_Mip8H = 1U;

//----------------------------------------------------------------------------------------------------------------------

void DownsampleMip8 ( in uint32_t x, in uint32_t y, in uint32_t2 workGroupID, in uint32_t threadID )
{
    if ( threadID >= 64U )
        return;

    const uint32_t2 xy = uint32_t2 ( x, y );
    const uint32_t2 base = xy + xy;

    const float16_t v = ReduceIntermediate ( base,
        base + uint32_t2 ( 1U, 0U ),
        base + uint32_t2 ( 0U, 1U ),
        base + uint32_t2 ( 1U, 1U )
    );

    StoreStrict ( workGroupID * 8U + xy, v, 7U, uint32_t2 ( g_Mip8W, g_Mip8H ) );

    // store to LDS, try to reduce bank conflicts
    // x 0 x 0 x 0 x 0 x 0 x 0 x 0 x 0
    // 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
    // 0 x 0 x 0 x 0 x 0 x 0 x 0 x 0 x
    // 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
    // x 0 x 0 x 0 x 0 x 0 x 0 x 0 x 0
    // ...
    // x 0 x 0 x 0 x 0 x 0 x 0 x 0 x 0
    StoreIntermediate ( base.x + y % 2U, base.y, v );
}

void DownsampleMip9Last ( in uint32_t x, in uint32_t y, in uint32_t2 workGroupID, in uint32_t threadID )
{
    if ( threadID >= 16U )
        return;

    const uint32_t2 xy = uint32_t2 ( x, y );
    const uint32_t2 base = xy * 4U;

    // x 0 x 0
    // 0 0 0 0
    // 0 x 0 x
    // 0 0 0 0
    const float16_t v = ReduceIntermediate ( base,
        base + uint32_t2 ( 2U, 0U ),
        base + uint32_t2 ( 1U, 2U ),
        base + uint32_t2 ( 3U, 2U )
    );

    StoreStrict ( workGroupID.xy * 4U + xy, v, 8U, uint32_t2 ( 1U, 1U ) );
}


#endif // SPD_HANDLE_MIP_9_CS
