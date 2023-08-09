// It's modified AMD SPD algorithm.
// See https://gpuopen.com/fidelityfx-spd/

#define MAX_MIP_LEVELS          12

#define THREAD_GROUP_WIDTH      256
#define THREAD_GROUP_HEIGHT     1
#define THREAD_GROUP_DEPTH      1

cbuffer cbSPD:                                                                                      register ( b0 )
{
    uint32_t                                                    mips;
    uint32_t                                                    numWorkGroups;
    uint32_t2                                                   workGroupOffset;
};

struct SpdGlobalAtomicBuffer
{
    uint32_t                                                    counter[ 6U ];
};

globallycoherent RWStructuredBuffer<SpdGlobalAtomicBuffer>      g_GlobalAtomic:                     register ( u0 );
globallycoherent RWTexture2DArray<float32_t4>                   g_MidMip:                           register ( u1 );
RWTexture2DArray<float32_t4>                                    g_Mips[ MAX_MIP_LEVELS + 1U ]:      register ( u2 );

groupshared float16_t2                                          s_RG[ 16U ][ 16U ];
groupshared float16_t2                                          s_BA[ 16U ][ 16U ];
groupshared uint32_t                                            s_Counter;

//----------------------------------------------------------------------------------------------------------------------

float16_t4 LoadIntermediate ( in uint32_t x, in uint32_t y )
{
    return float16_t4 ( s_RG[ x ][ y ], s_BA[ x ][ y ] );
}

float16_t4 Reduce4 ( in float16_t4 v0, in float16_t4 v1, in float16_t4 v2, in float16_t4 v3 )
{
    return ( v0 + v1 + v2 + v3 ) * 0.25H;
}

float16_t4 ReduceLoad4 ( in uint32_t2 base, in uint32_t slice )
{
    const float16_t4 v0 = (float16_t4)g_MidMip[ uint32_t3 ( base, slice ) ];
    const float16_t4 v1 = (float16_t4)g_MidMip[ uint32_t3 ( base + uint32_t2 ( 0U, 1U ), slice ) ];
    const float16_t4 v2 = (float16_t4)g_MidMip[ uint32_t3 ( base + uint32_t2 ( 1U, 0U ), slice ) ];
    const float16_t4 v3 = (float16_t4)g_MidMip[ uint32_t3 ( base + uint32_t2 ( 1U, 1U ), slice ) ];
    return Reduce4 ( v0, v1, v2, v3 );
}

float16_t4 ReduceLoadSourceImage ( in uint32_t2 base, in uint32_t slice )
{
    const float16_t4 v0 = (float16_t4)g_Mips[ 0U ][ uint32_t3 ( base, slice ) ];
    const float16_t4 v1 = (float16_t4)g_Mips[ 0U ][ uint32_t3 ( base + uint32_t2 ( 0U, 1U ), slice ) ];
    const float16_t4 v2 = (float16_t4)g_Mips[ 0U ][ uint32_t3 ( base + uint32_t2 ( 1U, 0U ), slice ) ];
    const float16_t4 v3 = (float16_t4)g_Mips[ 0U ][ uint32_t3 ( base + uint32_t2 ( 1U, 1U ), slice ) ];
    return Reduce4 ( v0, v1, v2, v3 );
}

float16_t4 ReduceIntermediate ( in uint32_t2 i0, in uint32_t2 i1, in uint32_t2 i2, in uint32_t2 i3 )
{
    const float16_t4 v0 = LoadIntermediate ( i0.x, i0.y );
    const float16_t4 v1 = LoadIntermediate ( i1.x, i1.y );
    const float16_t4 v2 = LoadIntermediate ( i2.x, i2.y );
    const float16_t4 v3 = LoadIntermediate ( i3.x, i3.y );
    return Reduce4 ( v0, v1, v2, v3 );
}

void Store ( in uint32_t2 pix, in float16_t4 value, in uint32_t mip, in uint32_t slice )
{
    if ( mip == 5U )
    {
        g_MidMip[ uint32_t3 ( pix, slice ) ] = (float32_t4)value;
        return;
    }

    g_Mips[ mip + 1U ][ uint32_t3 ( pix, slice ) ] = (float32_t4)value;
}

void StoreIntermediate ( in uint32_t x, in uint32_t y, in float16_t4 value )
{
    s_RG[ x ][ y ] = value.xy;
    s_BA[ x ][ y ] = value.zw;
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

// Only last active workgroup should proceed
bool ExitWorkgroup ( in uint32_t numWorkGroups, in uint32_t localInvocationIndex, in uint32_t slice )
{
    // global atomic counter
    if ( localInvocationIndex == 0U )
        InterlockedAdd ( g_GlobalAtomic[ 0U ].counter[ slice ], 1U, s_Counter );

    GroupMemoryBarrierWithGroupSync ();
    return s_Counter != ( numWorkGroups - 1U );
}

void DownsampleMips01 ( in uint32_t x,
    in uint32_t y,
    in uint32_t2 workGroupID,
    in uint32_t localInvocationIndex,
    in uint32_t slice
)
{
    float16_t4 v[ 4U ];

    const uint32_t2 xy = uint32_t2 ( x, y );
    const uint32_t2 pixBase = workGroupID.xy * 32U + xy;
    const uint32_t2 texBase = pixBase + pixBase;

    v[ 0U ] = ReduceLoadSourceImage ( texBase, slice );
    Store ( pixBase, v[ 0U ], 0U, slice );

    v[ 1U ] = ReduceLoadSourceImage ( texBase + uint32_t2 ( 32U, 0U ), slice );
    Store ( pixBase + uint32_t2 ( 16U, 0U ), v[ 1U ], 0U, slice );

    v[ 2U ] = ReduceLoadSourceImage ( texBase + uint32_t2 ( 0U, 32U ), slice );
    Store ( pixBase + uint32_t2 ( 0U, 16U ), v[ 2U ], 0U, slice );

    v[ 3U ] = ReduceLoadSourceImage ( texBase + uint32_t2 ( 32U, 32U ), slice );
    Store ( pixBase + uint32_t2 ( 16U, 16U ), v[ 3U ], 0U, slice );

    if ( mips <= 1U )
        return;

    const uint32_t2 alpha = xy + xy;
    const uint32_t2 betta = workGroupID * 16U + xy;

    [unroll]
    for ( uint32_t i = 0U; i < 4U; ++i )
    {
        StoreIntermediate ( x, y, v[ i ] );
        GroupMemoryBarrierWithGroupSync ();

        if ( localInvocationIndex < 64U )
        {
            v[ i ] = ReduceIntermediate ( alpha,
                alpha + uint32_t2 ( 1U, 0U ),
                alpha + uint32_t2 ( 0U, 1U ),
                alpha + uint32_t2 ( 1U, 1U )
            );

            Store ( betta + uint32_t2 ( i % 2U, i / 2U ) * 8U, v[ i ], 1U, slice );
        }

        GroupMemoryBarrierWithGroupSync ();
    }

    if ( localInvocationIndex >= 64U )
        return;

    const uint32_t2 gamma = xy + 8U;
    StoreIntermediate ( x, y, v[ 0U ] );
    StoreIntermediate ( gamma.x, y, v[ 1U ] );
    StoreIntermediate ( x, gamma.y, v[ 2U ] );
    StoreIntermediate ( gamma.x, gamma.y, v[ 3U ] );
}

void DownsampleMip2(in uint32_t x,
    in uint32_t y,
    in uint32_t2 workGroupID,
    in uint32_t localInvocationIndex,
    in uint32_t mip,
    in uint32_t slice
)
{
    if ( localInvocationIndex >= 64U )
        return;

    const uint32_t2 xy = uint32_t2 ( x, y );
    const uint32_t2 base = xy + xy;

    const float16_t4 v = ReduceIntermediate ( base,
        base + uint32_t2 ( 1U, 0U ),
        base + uint32_t2 ( 0U, 1U ),
        base + uint32_t2 ( 1U, 1U )
    );

    Store ( workGroupID * 8U + xy, v, mip, slice );

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

void DownsampleMip3 ( in uint32_t x,
    in uint32_t y,
    in uint32_t2 workGroupID,
    in uint32_t localInvocationIndex,
    in uint32_t mip,
    in uint32_t slice
)
{
    if ( localInvocationIndex >= 16U )
        return;

    const uint32_t2 xy = uint32_t2 ( x, y );
    const uint32_t2 base = xy * 4U;

    // x 0 x 0
    // 0 0 0 0
    // 0 x 0 x
    // 0 0 0 0
    const float16_t4 v = ReduceIntermediate ( base,
        base + uint32_t2 ( 2U, 0U ),
        base + uint32_t2 ( 1U, 2U ),
        base + uint32_t2 ( 3U, 2U )
    );

    Store ( workGroupID.xy * 4U + xy, v, mip, slice );

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

void DownsampleMip4 ( in uint32_t x,
    in uint32_t y,
    in uint32_t2 workGroupID,
    in uint32_t localInvocationIndex,
    in uint32_t mip,
    in uint32_t slice
)
{
    if ( localInvocationIndex >= 4U )
        return;

    const uint32_t2 xy = uint32_t2 ( x, y );
    const uint32_t yy = y + y;
    uint32_t2 alpha = xy * 8U;
    alpha.x += yy;

    // x 0 0 0 x 0 0 0
    // ...
    // 0 x 0 0 0 x 0 0
    const float16_t4 v = ReduceIntermediate ( alpha,
        alpha + uint32_t2 ( 4U, 0U ),
        alpha + uint32_t2 ( 1U, 4U ),
        alpha + uint32_t2 ( 5U, 4U )
    );

    Store ( workGroupID.xy + workGroupID.xy + xy, v, mip, slice );

    // store to LDS
    // x x x x 0 ...
    // 0 ...
    StoreIntermediate ( x + yy, 0U, v );
}

void DownsampleMip5 ( in uint32_t2 workGroupID,
    in uint32_t localInvocationIndex,
    in uint32_t mip,
    in uint32_t slice
)
{
    if ( localInvocationIndex >= 1U )
        return;

    // x x x x 0 ...
    // 0 ...
    const float16_t4 v = ReduceIntermediate ( (uint32_t2)0U,
        uint32_t2 ( 1U, 0U ),
        uint32_t2 ( 2U, 0U ),
        uint32_t2 ( 3U, 0U )
    );

    Store ( workGroupID.xy, v, mip, slice );
}

void DownsampleMips67 ( in uint32_t x, in uint32_t y, in uint32_t mips, in uint32_t slice )
{
    const uint32_t2 xy = uint32_t2 ( x, y );

    // Optimization: Replacing multiplication by addition:
    // pixBase = xy * 2U
    // texBase = xy * 4U
    const uint32_t2 pixBase = xy + xy;
    const uint32_t2 texBase = pixBase + pixBase;

    const float16_t4 v0 = ReduceLoad4 ( texBase, slice );
    Store ( pixBase, v0, 6U, slice );

    const float16_t4 v1 = ReduceLoad4 ( texBase + uint32_t2 ( 2U, 0U ), slice );
    Store ( pixBase + uint32_t2 ( 1U, 0U ), v1, 6U, slice );

    const float16_t4 v2 = ReduceLoad4 ( texBase + uint32_t2 ( 0U, 2U ), slice );
    Store ( pixBase + uint32_t2 ( 0U, 1U ), v2, 6U, slice );

    const float16_t4 v3 = ReduceLoad4 ( texBase + uint32_t2 ( 2U, 2U ), slice );
    Store ( pixBase + uint32_t2 ( 1U, 1U ), v3, 6U, slice );

    if ( mips < 8U )
        return;

    // no barrier needed, working on values only from the same thread
    const float16_t4 v = Reduce4 ( v0, v1, v2, v3 );
    Store ( xy, v, 7U, slice );
    StoreIntermediate ( x, y, v );
}

void DownsampleNextFour ( in uint32_t x,
    in uint32_t y,
    in uint32_t2 workGroupID,
    in uint32_t localInvocationIndex,
    in uint32_t baseMip,
    in uint32_t mips,
    in uint32_t slice
)
{
    if ( mips <= baseMip )
        return;

    GroupMemoryBarrierWithGroupSync ();
    DownsampleMip2 ( x, y, workGroupID, localInvocationIndex, baseMip, slice );

    if ( mips <= baseMip + 1U )
        return;

    GroupMemoryBarrierWithGroupSync ();
    DownsampleMip3 ( x, y, workGroupID, localInvocationIndex, baseMip + 1U, slice );

    if ( mips <= baseMip + 2U )
        return;

    GroupMemoryBarrierWithGroupSync ();
    DownsampleMip4 ( x, y, workGroupID, localInvocationIndex, baseMip + 2U, slice );

    if ( mips <= baseMip + 3U )
        return;

    GroupMemoryBarrierWithGroupSync ();
    DownsampleMip5 ( workGroupID, localInvocationIndex, baseMip + 3U, slice );
}

[numthreads ( THREAD_GROUP_WIDTH, THREAD_GROUP_HEIGHT, THREAD_GROUP_DEPTH )]
void CS ( in uint32_t localInvocationIndex: SV_GroupIndex, in uint32_t3 workGroupID: SV_GroupID )
{
    // Downsamples a 64x64 tile based on the work group id and work group offset.
    // If after downsampling it's the last active thread group, computes the remaining MIP levels.
    // Uses half types.
    //
    // workGroupID.xy          index of the work group / thread group
    // localInvocationIndex    index of the thread within the thread group in 1D
    // mips                    the number of total MIP levels to compute for the input texture
    // numWorkGroups           the total number of dispatched work groups / thread groups for this slice
    // workGroupID.z           the slice of the input texture

    const uint32_t2 wg = workGroupID.xy + workGroupOffset;

    const uint32_t2 subXY = RemapForWaveReduction ( localInvocationIndex % 64U );
    const uint32_t2 alpha = uint32_t2 ( ( localInvocationIndex >> 6U ) % 2U, localInvocationIndex >> 7U );
    const uint32_t2 base = subXY + alpha * 8U;

    // compute MIP level 0 and 1
    DownsampleMips01 ( base.x, base.y, wg, localInvocationIndex, workGroupID.z );

    // compute MIP level 2, 3, 4, 5
    DownsampleNextFour ( base.x, base.y, wg, localInvocationIndex, 2U, mips, workGroupID.z );

    if ( mips < 7U )
        return;

    // increase the global atomic counter for the given slice and check if it's the last remaining thread group:
    // terminate if not, continue if yes.
    if ( ExitWorkgroup ( numWorkGroups, localInvocationIndex, workGroupID.z ) )
        return;

    // reset the global atomic counter back to 0 for the next spd dispatch
    g_GlobalAtomic[ 0U ].counter[ workGroupID.z ] = 0U;

    // After mip 5 there is only a single workgroup left that downsamples the remaining up to 64x64 texels.
    // compute MIP level 6 and 7
    DownsampleMips67 ( base.x, base.y, mips, workGroupID.z );

    // compute MIP level 8, 9, 10, 11
    DownsampleNextFour ( base.x, base.y, (uint32_t2)0U, localInvocationIndex, 8U, mips, workGroupID.z );
}
