// See <repo/docs/spd-algorithm.md>

#include "average_brightness.inc"


#define THREAD_X    256
#define THREAD_Y    1
#define THREAD_Z    1


[[vk::constant_id ( CONST_WORKGROUP_COUNT )]]
const uint32_t                                  g_WorkgroupCount = 256U;

[[vk::constant_id ( CONST_MIP_5_W )]]
const uint32_t                                  g_Mip5W = 32U;

[[vk::constant_id ( CONST_MIP_5_H )]]
const uint32_t                                  g_Mip5H = 14U;

[[vk::constant_id ( CONST_NORMALIZE_W )]]
const float32_t                                 g_NormalizeW = 3.125e-2H;

[[vk::constant_id ( CONST_NORMALIZE_H )]]
const float32_t                                 g_NormalizeH = 7.142e-2H;

[[vk::binding ( BIND_HDR_IMAGE, SET_RESOURCE )]]
Texture2D<float32_t4>                           g_HDRImage:             register ( t0 );

[[vk::binding ( BIND_SYNC_MIP_5, SET_RESOURCE )]]
globallycoherent RWTexture2D<float32_t4>        g_SyncMip5:             register ( u0 );

struct Atomic
{
    uint32_t                                    _counter;
};

[[vk::binding ( BIND_GLOBAL_ATOMIC, SET_RESOURCE )]]
globallycoherent RWStructuredBuffer<Atomic>     g_GlobalAtomic:         register ( u1 );

[[vk::binding ( BIND_BRIGHTNESS, SET_RESOURCE )]]
RWStructuredBuffer<float32_t>                   g_Brightness:           register ( u2 );

groupshared float16_t                           s_Luma[ 16U ][ 16U ];
groupshared uint32_t                            s_Counter;

//----------------------------------------------------------------------------------------------------------------------

float16_t Reduce4 ( in float16_t v0, in float16_t v1, in float16_t v2, in float16_t v3 )
{
    const float16_t2 alpha = float16_t2 ( v0, v1 ) + float16_t2 ( v2, v3 );
    return ( alpha.x + alpha.y ) * 0.25H;
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

void ReduceRows ( in uint32_t threadID )
{
    if ( threadID >= g_Mip5H )
        return;

    float16_t acc = 0.0H;

    for ( uint32_t x = 0U; x < g_Mip5W; ++x )
        acc += (float16_t)g_SyncMip5[ uint32_t2 ( x, threadID ) ].x;

    s_Luma[ threadID >> 4U ][ threadID & 0x0000000FU ] = acc * (float16_t)g_NormalizeW;
}

float32_t ReduceToAverage ()
{
    float16_t acc = s_Luma[ 0U ][ 0U ];

    for ( uint32_t i = 1U; i < g_Mip5H; ++i )
        acc += s_Luma[ i >> 4U ][ i & 0x0000000FU ];

    return (float32_t)acc * g_NormalizeH;
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

// See <repo>/docs/spd-algorithm.md#mip-0
uint32_t2 RemapForWaveReduction ( in uint32_t a )
{
    return uint32_t2 ( BitfieldInsertMask ( BitfieldExtract ( a, 2U, 3U ), a, 1U ),
        BitfieldInsertMask ( BitfieldExtract ( a, 3U, 3U ), BitfieldExtract ( a, 1U, 2U ), 2U )
    );
}

uint32_t2 GetThreadTarget ( in uint32_t threadID )
{
    // Optimization: "threadID & 0x0000003FU" equals "threadID % 64U" equals.
    const uint32_t2 subXY = RemapForWaveReduction ( threadID & 0x0000003FU );

    // Optimization: "( threadID >> 6U ) & 0x00000001U" equals "( threadID >> 6U ) % 2U".
    const uint32_t2 alpha = uint32_t2 ( ( threadID >> 6U ) & 0x00000001U, threadID >> 7U );
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
    // See <repo>/docs/spd-algorithm.md#mip-0
    float16_t v[ 4U ];

    const uint32_t2 xy = uint32_t2 ( x, y );
    const uint32_t2 pixBase = workGroupID.xy * 32U + xy;
    const uint32_t2 texBase = pixBase + pixBase;

    v[ 0U ] = ReduceLoadSourceImage ( texBase );
    v[ 1U ] = ReduceLoadSourceImage ( texBase + uint32_t2 ( 32U, 0U ) );
    v[ 2U ] = ReduceLoadSourceImage ( texBase + uint32_t2 ( 0U, 32U ) );
    v[ 3U ] = ReduceLoadSourceImage ( texBase + uint32_t2 ( 32U, 32U ) );

    const uint32_t2 alpha = xy + xy;
    const uint32_t2 betta = workGroupID * 16U + xy;

    // See <repo>/docs/spd-algorithm.md#mip-1

    [unroll]
    for ( uint32_t i = 0U; i < 4U; ++i )
    {
        s_Luma[ x ][ y ] = v[ i ];
        GroupMemoryBarrierWithGroupSync ();

        if ( threadID < 64U )
        {
            v[ i ] = ReduceIntermediate ( alpha,
                alpha + uint32_t2 ( 1U, 0U ),
                alpha + uint32_t2 ( 0U, 1U ),
                alpha + uint32_t2 ( 1U, 1U )
            );
        }

        GroupMemoryBarrierWithGroupSync ();
    }

    if ( threadID >= 64U )
        return;

    const uint32_t2 gamma = xy + 8U;

    s_Luma[ x ][ y ] = v[ 0U ];
    s_Luma[ gamma.x ][ y ] = v[ 1U ];
    s_Luma[ x ][ gamma.y ] = v[ 2U ];
    s_Luma[ gamma.x ][ gamma.y ] = v[ 3U ];
}

// See <repo>/docs/spd-algorithm.md#mip-2
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

    // Optimization: "base.x + ( y & 0x00000001U )" equals "base.x + y % 2U".
    s_Luma[ base.x + ( y & 0x00000001U ) ][ base.y ] = v;
}

// See <repo>/docs/spd-algorithm.md#mip-3
void DownsampleMip3 ( in uint32_t x, in uint32_t y, in uint32_t2 workGroupID, in uint32_t threadID )
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

    s_Luma[ base.x + y ][ base.y ] = v;
}

// See <repo>/docs/spd-algorithm.md#mip-4
void DownsampleMip4 ( in uint32_t x, in uint32_t y, in uint32_t2 workGroupID, in uint32_t threadID )
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

    s_Luma[ x + yy ][ 0U ] = v;
}

// See <repo>/docs/spd-algorithm.md#mip-5
void DownsampleMip5 ( in uint32_t2 workGroupID, in uint32_t threadID )
{
    if ( threadID >= 1U )
        return;

    const float16_t v = ReduceIntermediate ( (uint32_t2)0U,
        uint32_t2 ( 1U, 0U ),
        uint32_t2 ( 2U, 0U ),
        uint32_t2 ( 3U, 0U )
    );

    g_SyncMip5[ workGroupID.xy ] = (float32_t)v;
}

//----------------------------------------------------------------------------------------------------------------------

[numthreads ( THREAD_X, THREAD_Y, THREAD_Z )]
void CS ( in uint32_t threadID: SV_GroupIndex, in uint32_t3 workGroupID: SV_GroupID )
{
    const uint32_t2 base = GetThreadTarget ( threadID );
    DownsampleMips01 ( base.x, base.y, workGroupID.xy, threadID );

    GroupMemoryBarrierWithGroupSync ();
    DownsampleMip2 ( base.x, base.y, workGroupID.xy, threadID );

    GroupMemoryBarrierWithGroupSync ();
    DownsampleMip3 ( base.x, base.y, workGroupID.xy, threadID );

    GroupMemoryBarrierWithGroupSync ();
    DownsampleMip4 ( base.x, base.y, workGroupID.xy, threadID );

    GroupMemoryBarrierWithGroupSync ();
    DownsampleMip5 ( workGroupID.xy, threadID );

    if ( ExitWorkgroup ( threadID ) )
        return;

    ReduceRows ( threadID );
    GroupMemoryBarrierWithGroupSync ();

    if ( threadID > 0U )
        return;

    g_Brightness[ 0U ] = ReduceToAverage ();

    // Reset the global atomic counter back to 0 for the next spd dispatch.
    g_GlobalAtomic[ 0U ]._counter = 0U;
}
