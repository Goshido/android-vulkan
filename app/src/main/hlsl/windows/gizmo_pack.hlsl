#ifndef GIZMO_PACK_HLSL
#define GIZMO_PACK_HLSL


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

// FUCK - rewrite to use color_packing
// float32_t4 UnpackSamplePremultipliedAlpha ( in uint32_t pack, in StructuredBuffer<uint32_t> palette )
// {
//     uint32_t2 beta = (uint32_t2)pack & uint32_t2 ( ALPHA_UNPACK_MASK, PALETTE_UNPACK_MASK );
//     beta.y >>= ALPHA_SHIFT;

//     // FUCK use v3 version
//     float32_t4 result = UnpackColorF32x4 ( palette[ beta.y ] );
//     result.w = ALPHA_8_BIT_UNORM * (float32_t)beta.x;
//     result.xyz *= result.w;
//     return result;
// }

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
