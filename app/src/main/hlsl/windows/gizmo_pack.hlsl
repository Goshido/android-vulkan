#ifndef GIZMO_PACK_HLSL
#define GIZMO_PACK_HLSL


#include "windows/gizmo_palette.hlsl"


#define GIZMO_DEPTH_PACK_FACTOR     ( (float32_t)0x000FFFFFU )
#define GIZMO_DEPTH_SHIFT           12U

// Counter is 4 bits
#define TILE_COUNTER_BIT_SHIFTS     4U

// 2 ^ TILE_COUNTER_BIT_SHIFTS - 1
#define TILE_COUNTER_BIT_MASK       0x0000000FU

// 2 ^ 8 - 1
#define ALPHA_8_BIT                 255.0F
#define ALPHA_SHIFT                 8U
#define ALPHA_UNPACK_MASK           0x000000FFU

// 1 / ALPHA_8_BIT
#define ALPHA_8_BIT_UNORM           3.92156e-3F
#define ALPHA_8_BIT_UNORM_16F       3.92156e-3H

#define PALETTE_UNPACK_MASK         0x00000F00U

//----------------------------------------------------------------------------------------------------------------------

uint32_t PackDepth ( in float32_t depth )
{
    return (uint32_t)( depth * GIZMO_DEPTH_PACK_FACTOR ) << GIZMO_DEPTH_SHIFT;
}

uint32_t PackSample ( in uint32_t palette, in float32_t alpha, in float32_t depth )
{
    return (uint32_t)( alpha * ALPHA_8_BIT ) |
        ( palette << ALPHA_SHIFT ) |
        ( (uint32_t)( depth * GIZMO_DEPTH_PACK_FACTOR ) << GIZMO_DEPTH_SHIFT
    );
}

float16_t4 UnpackSamplePremultipliedAlpha ( in uint32_t pack )
{
    uint32_t2 beta = (uint32_t2)pack & uint32_t2 ( ALPHA_UNPACK_MASK, PALETTE_UNPACK_MASK );
    beta.y >>= ALPHA_SHIFT;

    float16_t4 result = float16_t4 ( (float16_t3)g_gizmoPalette[ beta.y ].xyz,
        ALPHA_8_BIT_UNORM_16F * (float16_t)beta.x
    );

    result.xyz *= result.w;
    return result;
}

// See <repo>/docs/gizmo-rendering.md#counters
uint32_t MakeSampleCounterIncrement ( in uint32_t counterIdx )
{
    return 1U << ( counterIdx * TILE_COUNTER_BIT_SHIFTS );
}

// See <repo>/docs/gizmo-rendering.md#counters
uint32_t UnpackSampleCounter ( in uint32_t tileLineSampleCount, in uint32_t counterIdx )
{
    return ( tileLineSampleCount >> ( counterIdx * TILE_COUNTER_BIT_SHIFTS ) ) & TILE_COUNTER_BIT_MASK;
}


#endif // GIZMO_PACK_HLSL
