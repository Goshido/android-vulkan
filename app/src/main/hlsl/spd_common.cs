#ifndef SPD_COMMON_CS
#define SPD_COMMON_CS


#include "spd.inc"


#define THREAD_X    256
#define THREAD_Y    1
#define THREAD_Z    1


[[vk::constant_id ( CONST_WORKGROUP_COUNT )]]
const uint32_t                                  g_WorkgroupCount = 256U;

[[vk::constant_id ( CONST_MIP_5_W )]]
const uint32_t                                  g_Mip5W = 32U;

[[vk::constant_id ( CONST_MIP_5_H )]]
const uint32_t                                  g_Mip5H = 14U;

[[vk::constant_id ( CONST_MIP_6_W )]]
const uint32_t                                  g_Mip6W = 16U;

[[vk::constant_id ( CONST_MIP_6_H )]]
const uint32_t                                  g_Mip6H = 7U;

[[vk::constant_id ( CONST_MIP_7_W )]]
const uint32_t                                  g_Mip7W = 8U;

[[vk::constant_id ( CONST_MIP_7_H )]]
const uint32_t                                  g_Mip7H = 3U;

[[vk::binding ( BIND_HDR_IMAGE, SET_RESOURCE )]]
Texture2D<float32_t4>                           g_HDRImage:             register ( t0 );

[[vk::binding ( BIND_SYNC_MIP_5, SET_RESOURCE )]]
globallycoherent RWTexture2D<float32_t4>        g_SyncMip5:             register ( u1 );

[[vk::binding ( BIND_MIPS, SET_RESOURCE )]]
RWTexture2D<float32_t4>                         g_Mips[ MIP_COUNT ]:    register ( u2 );

struct Atomic
{
    uint32_t                                    _counter;
};

[[vk::binding ( BIND_GLOBAL_ATOMIC, SET_RESOURCE )]]
globallycoherent RWStructuredBuffer<Atomic>     g_GlobalAtomic:         register ( u3 );

groupshared float16_t                           s_Luma[ 16U ][ 16U ];
groupshared uint32_t                            s_Counter;

//----------------------------------------------------------------------------------------------------------------------

float16_t Reduce4 ( in float16_t v0, in float16_t v1, in float16_t v2, in float16_t v3 )
{
    const float16_t2 alpha = float16_t2 ( v0, v1 ) + float16_t2 ( v2, v3 );
    return ( alpha.x + alpha.y ) * 0.25H;
}

float16_t ReduceStrict4 ( in float16_t v0, in float16_t v1, in float16_t v2, in float16_t v3, in float16_t w )
{
    const float16_t2 alpha = float16_t2 ( v0, v1 ) + float16_t2 ( v2, v3 );
    return ( alpha.x + alpha.y ) * w;
}

float16_t ReduceStrictLoad4 ( in uint32_t2 base )
{
    const uint32_t2 m = base + 1U;
    uint16_t counter = 0U;

    const float16_t v0 = (float16_t)g_SyncMip5[ base ].x;

    float16_t v1 = 0.0H;

    if ( m.y < g_Mip5H )
    {
        v1 = (float16_t)g_SyncMip5[ base + uint32_t2 ( 0U, 1U ) ].x;
        ++counter;
    }

    float16_t v2 = 0.0H;

    if ( m.x < g_Mip5W )
    {
        v2 = (float16_t)g_SyncMip5[ base + uint32_t2 ( 1U, 0U ) ].x;
        ++counter;
    }

    float16_t v3 = 0.0H;

    if ( all ( m < uint32_t2 ( g_Mip5W, g_Mip5H ) ) )
    {
        v3 = (float16_t)g_SyncMip5[ m ].x;
        ++counter;
    }

    // Scale coeffient which depeds how many members existed.
    static const float16_t weights[ 4U ] = { 1.0H, 0.5H, 3.33333e-1H, 0.25H };
    return ReduceStrict4 ( v0, v1, v2, v3, weights[ counter ] );
}

float16_t ReduceLoadSourceImage ( in uint32_t2 base )
{
    // Converting to RGB to luma using BT.601:
    const float16_t3 bt601 = float16_t3 ( 0.299H, 0.587H, 0.114H );

    const float16_t v0 = dot ( bt601, (float16_t3)g_HDRImage[ base ].xyz );
    const float16_t v1 = dot ( bt601, (float16_t3)g_HDRImage[ base + uint32_t2 ( 0U, 1U ) ].xyz );
    const float16_t v2 = dot ( bt601, (float16_t3)g_HDRImage[ base + uint32_t2 ( 1U, 0U ) ].xyz );
    const float16_t v3 = dot ( bt601, (float16_t3)g_HDRImage[ base + uint32_t2 ( 1U, 1U ) ].xyz );
    return Reduce4 ( v0, v1, v2, v3 );
}

float16_t ReduceIntermediate ( in uint32_t2 i0, in uint32_t2 i1, in uint32_t2 i2, in uint32_t2 i3 )
{
    const float16_t v0 = s_Luma[ i0.x ][ i0.y ];
    const float16_t v1 = s_Luma[ i1.x ][ i1.y ];
    const float16_t v2 = s_Luma[ i2.x ][ i2.y ];
    const float16_t v3 = s_Luma[ i3.x ][ i3.y ];
    return Reduce4 ( v0, v1, v2, v3 );
}

void Store ( in uint32_t2 pix, in float16_t value, in uint32_t mip )
{
    g_Mips[ mip ][ pix ] = (float32_t)value;
}

void StoreStrict ( in uint32_t2 pix, in float16_t value, in uint32_t mip, in uint32_t2 resolution )
{
    if ( all ( pix < resolution ) )
    {
        g_Mips[ mip ][ pix ] = (float32_t)value;
    }
}

void StoreSync ( in uint32_t2 pix, in float16_t value )
{
    g_SyncMip5[ pix ] = (float32_t)value;
}

void StoreIntermediate ( in uint32_t x, in uint32_t y, in float16_t value )
{
    s_Luma[ x ][ y ] = value;
}

uint32_t BitfieldExtract ( in uint32_t src, in uint32_t off, in uint32_t bits )
{
    const uint32_t mask = ( 1U << bits ) - 1U;
    return ( src >> off ) & mask;
}

uint32_t BitfieldInsertMask ( in uint32_t src, in uint32_t ins, in uint32_t bits )
{
    const uint32_t mask = ( 1U << bits ) - 1U;
    return ( ins & mask ) | ( src & ( ~mask ) );
}

// A helper function performing a remap 64x1 to 8x8 remapping which is necessary for 2D wave reductions.
// The 64-wide lane indices to 8x8 remapping is performed as follows:
//
//      00 01 08 09 10 11 18 19
//      02 03 0a 0b 12 13 1a 1b
//      04 05 0c 0d 14 15 1c 1d
//      06 07 0e 0f 16 17 1e 1f
//      20 21 28 29 30 31 38 39
//      22 23 2a 2b 32 33 3a 3b
//      24 25 2c 2d 34 35 3c 3d
//      26 27 2e 2f 36 37 3e 3f
uint32_t2 RemapForWaveReduction ( in uint32_t a )
{
    return uint32_t2 ( BitfieldInsertMask ( BitfieldExtract ( a, 2U, 3U ), a, 1U ),
        BitfieldInsertMask ( BitfieldExtract ( a, 3U, 3U ), BitfieldExtract ( a, 1U, 2U ), 2U )
    );
}

uint32_t2 GetThreadTarget ( in uint32_t threadID )
{
    const uint32_t2 subXY = RemapForWaveReduction ( threadID % 64U );
    const uint32_t2 alpha = uint32_t2 ( ( threadID >> 6U ) % 2U, threadID >> 7U );
    return subXY + alpha * 8U;
}

// Increase the global atomic counter for the given slice and check if it's the last remaining thread group:
// terminate if not, continue if yes.
// Only last active workgroup should proceed
bool ExitWorkgroup ( in uint32_t threadID )
{
    // global atomic counter
    if ( threadID == 0U )
        InterlockedAdd ( g_GlobalAtomic[ 0U ]._counter, 1U, s_Counter );

    GroupMemoryBarrierWithGroupSync ();
    return s_Counter != g_WorkgroupCount;
}

void DownsampleMips01 ( in uint32_t x, in uint32_t y, in uint32_t2 workGroupID, in uint32_t threadID )
{
    float16_t v[ 4U ];

    const uint32_t2 xy = uint32_t2 ( x, y );
    const uint32_t2 pixBase = workGroupID.xy * 32U + xy;
    const uint32_t2 texBase = pixBase + pixBase;

    v[ 0U ] = ReduceLoadSourceImage ( texBase );
    Store ( pixBase, v[ 0U ], 0U );

    v[ 1U ] = ReduceLoadSourceImage ( texBase + uint32_t2 ( 32U, 0U ) );
    Store ( pixBase + uint32_t2 ( 16U, 0U ), v[ 1U ], 0U );

    v[ 2U ] = ReduceLoadSourceImage ( texBase + uint32_t2 ( 0U, 32U ) );
    Store ( pixBase + uint32_t2 ( 0U, 16U ), v[ 2U ], 0U );

    v[ 3U ] = ReduceLoadSourceImage ( texBase + uint32_t2 ( 32U, 32U ) );
    Store ( pixBase + uint32_t2 ( 16U, 16U ), v[ 3U ], 0U );

    const uint32_t2 alpha = xy + xy;
    const uint32_t2 betta = workGroupID * 16U + xy;

    [unroll]
    for ( uint32_t i = 0U; i < 4U; ++i )
    {
        StoreIntermediate ( x, y, v[ i ] );
        GroupMemoryBarrierWithGroupSync ();

        if ( threadID < 64U )
        {
            v[ i ] = ReduceIntermediate ( alpha,
                alpha + uint32_t2 ( 1U, 0U ),
                alpha + uint32_t2 ( 0U, 1U ),
                alpha + uint32_t2 ( 1U, 1U )
            );

            Store ( betta + uint32_t2 ( i % 2U, i / 2U ) * 8U, v[ i ], 1U );
        }

        GroupMemoryBarrierWithGroupSync ();
    }

    if ( threadID >= 64U )
        return;

    const uint32_t2 gamma = xy + 8U;
    StoreIntermediate ( x, y, v[ 0U ] );
    StoreIntermediate ( gamma.x, y, v[ 1U ] );
    StoreIntermediate ( x, gamma.y, v[ 2U ] );
    StoreIntermediate ( gamma.x, gamma.y, v[ 3U ] );
}

void DownsampleMip2 ( in uint32_t x, in uint32_t y, in uint32_t2 workGroupID, in uint32_t threadID )
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

    Store ( workGroupID * 8U + xy, v, 2U );

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

void DownsampleMip3 ( in uint32_t x, in uint32_t y, in uint32_t2 workGroupID, in uint32_t threadID )
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

    Store ( workGroupID.xy * 4U + xy, v, 3U );

    // store to LDS
    // x 0 0 0 x 0 0 0 x 0 0 0 x 0 0 0
    // 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
    // 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
    // 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
    // 0 x 0 0 0 x 0 0 0 x 0 0 0 x 0 0
    // ...
    // 0 0 x 0 0 0 x 0 0 0 x 0 0 0 x 0
    // ...
    // 0 0 0 x 0 0 0 x 0 0 0 x 0 0 0 x
    // ...
    StoreIntermediate ( base.x + y, base.y, v );
}

void DownsampleMip4 ( in uint32_t x, in uint32_t y, in uint32_t2 workGroupID, in uint32_t threadID )
{
    if ( threadID >= 4U )
        return;

    const uint32_t2 xy = uint32_t2 ( x, y );
    const uint32_t yy = y + y;
    uint32_t2 alpha = xy * 8U;
    alpha.x += yy;

    // x 0 0 0 x 0 0 0
    // ...
    // 0 x 0 0 0 x 0 0
    const float16_t v = ReduceIntermediate ( alpha,
        alpha + uint32_t2 ( 4U, 0U ),
        alpha + uint32_t2 ( 1U, 4U ),
        alpha + uint32_t2 ( 5U, 4U )
    );

    Store ( workGroupID.xy + workGroupID.xy + xy, v, 4U );

    // store to LDS
    // x x x x 0 ...
    // 0 ...
    StoreIntermediate ( x + yy, 0U, v );
}

void DownsampleMip5 ( in uint32_t2 workGroupID, in uint32_t threadID )
{
    if ( threadID >= 1U )
        return;

    // x x x x 0 ...
    // 0 ...
    const float16_t v = ReduceIntermediate ( (uint32_t2)0U,
        uint32_t2 ( 1U, 0U ),
        uint32_t2 ( 2U, 0U ),
        uint32_t2 ( 3U, 0U )
    );

    StoreSync ( workGroupID.xy, v );
}

void DownsampleMips67 ( in uint32_t x, in uint32_t y )
{
    const uint32_t2 xy = uint32_t2 ( x, y );

    // Optimization: Replacing multiplication by addition:
    // pixBase = xy * 2U
    // texBase = xy * 4U
    const uint32_t2 pixBase = xy + xy;
    const uint32_t2 texBase = pixBase + pixBase;
    const uint32_t2 resolution = uint32_t2 ( g_Mip6W, g_Mip6H );

    const float16_t v0 = ReduceStrictLoad4 ( texBase );
    StoreStrict ( pixBase, v0, 5U, resolution );

    const float16_t v1 = ReduceStrictLoad4 ( texBase + uint32_t2 ( 2U, 0U ) );
    StoreStrict ( pixBase + uint32_t2 ( 1U, 0U ), v1, 5U, resolution );

    const float16_t v2 = ReduceStrictLoad4 ( texBase + uint32_t2 ( 0U, 2U ) );
    StoreStrict ( pixBase + uint32_t2 ( 0U, 1U ), v2, 5U, resolution );

    const float16_t v3 = ReduceStrictLoad4 ( texBase + uint32_t2 ( 2U, 2U ) );
    StoreStrict ( pixBase + uint32_t2 ( 1U, 1U ), v3, 5U, resolution );

    // no barrier needed, working on values only from the same thread
    const float16_t v = Reduce4 ( v0, v1, v2, v3 );
    StoreStrict ( xy, v, 6U, uint32_t2 ( g_Mip7W, g_Mip7H ) );
    StoreIntermediate ( x, y, v );
}

void DownsampleMip8Last ( in uint32_t x, in uint32_t y, in uint32_t2 workGroupID, in uint32_t threadID )
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

    StoreStrict ( workGroupID * 8U + xy, v, 7U, uint32_t2 ( 1U, 1U ) );
}


#endif // SPD_COMMON_CS
