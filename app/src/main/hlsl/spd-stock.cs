#define FFX_SPD_BIND_SRV_INPUT_DOWNSAMPLE_SRC               0

#define FFX_SPD_BIND_UAV_INTERNAL_GLOBAL_ATOMIC             0
#define FFX_SPD_BIND_UAV_INPUT_DOWNSAMPLE_SRC_MID_MIPMAP    1
#define FFX_SPD_BIND_UAV_INPUT_DOWNSAMPLE_SRC_MIPS          2

#define FFX_SPD_BIND_CB_SPD                                 0

// Assumes a maximum on 12 mips. If larger base resolution support is added,
// extra mip resources need to also be added
#define SPD_MAX_MIP_LEVELS 12

#define FFX_SPD_THREAD_GROUP_WIDTH 256
#define FFX_SPD_THREAD_GROUP_HEIGHT 1
#define FFX_SPD_THREAD_GROUP_DEPTH 1

#define FFX_DECLARE_SRV_REGISTER(regIndex)  t##regIndex
#define FFX_SPD_DECLARE_SRV(regIndex)  register(FFX_DECLARE_SRV_REGISTER(regIndex))

#define FFX_DECLARE_UAV_REGISTER(regIndex)  u##regIndex
#define FFX_SPD_DECLARE_UAV(regIndex)  register(FFX_DECLARE_UAV_REGISTER(regIndex))

#define FFX_DECLARE_CB_REGISTER(regIndex)   b##regIndex
#define FFX_SPD_DECLARE_CB(regIndex)   register(FFX_DECLARE_CB_REGISTER(regIndex))

/// A define for abstracting shared memory between shading languages.
///
/// @ingroup HLSLCore
#define FFX_GROUPSHARED groupshared

typedef int32_t     FfxInt32;

/// A typedef for a 2-dimensional signed 32bit integer.
///
/// @ingroup HLSL62Types
typedef int32_t2    FfxInt32x2;

/// A typedef for a unsigned 32bit integer.
///
/// @ingroup HLSL62Types
typedef uint32_t    FfxUInt32;

/// A typedef for a 2-dimensional 32bit unsigned integer.
///
/// @ingroup HLSL62Types
typedef uint32_t2   FfxUInt32x2;

/// A typedef for a 3-dimensional 32bit unsigned integer.
///
/// @ingroup HLSL62Types
typedef uint32_t3   FfxUInt32x3;

/// A typedef for a 2-dimensional floating point value.
///
/// @ingroup HLSL62Types
typedef float32_t2  FfxFloat32x2;

/// A typedef for a 4-dimensional floating point value.
///
/// @ingroup HLSL62Types
typedef float32_t4  FfxFloat32x4;

typedef float16_t  FfxFloat16;
typedef float16_t2 FfxFloat16x2;
typedef float16_t4 FfxFloat16x4;

#define FFX_PARAMETER_IN        in
#define FFX_PARAMETER_INOUT     inout

/// A define for abstracting compute memory barriers between shading languages.
///
/// @ingroup HLSLCore
#define FFX_GROUP_MEMORY_BARRIER() GroupMemoryBarrierWithGroupSync()

cbuffer cbSPD : FFX_SPD_DECLARE_CB(FFX_SPD_BIND_CB_SPD)
{
    FfxUInt32       mips;
    FfxUInt32       numWorkGroups;
    FfxUInt32x2     workGroupOffset;
};

Texture2DArray<FfxFloat32x4>                                r_input_downsample_src          : FFX_SPD_DECLARE_SRV(FFX_SPD_BIND_SRV_INPUT_DOWNSAMPLE_SRC);
struct SpdGlobalAtomicBuffer { FfxUInt32 counter[6]; };
globallycoherent RWStructuredBuffer<SpdGlobalAtomicBuffer>  rw_internal_global_atomic       : FFX_SPD_DECLARE_UAV(FFX_SPD_BIND_UAV_INTERNAL_GLOBAL_ATOMIC);
globallycoherent RWTexture2DArray<FfxFloat32x4>             rw_input_downsample_src_mid_mip : FFX_SPD_DECLARE_UAV(FFX_SPD_BIND_UAV_INPUT_DOWNSAMPLE_SRC_MID_MIPMAP);
RWTexture2DArray<FfxFloat32x4>                              rw_input_downsample_src_mips[SPD_MAX_MIP_LEVELS+1] : FFX_SPD_DECLARE_UAV(FFX_SPD_BIND_UAV_INPUT_DOWNSAMPLE_SRC_MIPS);

FFX_GROUPSHARED FfxFloat16x2 spdIntermediateRG[16][16];
FFX_GROUPSHARED FfxFloat16x2 spdIntermediateBA[16][16];
FFX_GROUPSHARED FfxUInt32 spdCounter;

FfxUInt32x2  WorkGroupOffset()
{
    return workGroupOffset;
}

FfxUInt32 NumWorkGroups()
{
    return numWorkGroups;
}

FfxUInt32 Mips()
{
    return mips;
}

FfxUInt32 bitfieldExtract(FfxUInt32 src, FfxUInt32 off, FfxUInt32 bits)
{
    FfxUInt32 mask = (1u << bits) - 1;
    return (src >> off) & mask;
}

FfxUInt32 bitfieldInsertMask(FfxUInt32 src, FfxUInt32 ins, FfxUInt32 bits)
{
    FfxUInt32 mask = (1u << bits) - 1;
    return (ins & mask) | (src & (~mask));
}

/// A helper function performing a remap 64x1 to 8x8 remapping which is necessary for 2D wave reductions.
///
/// The 64-wide lane indices to 8x8 remapping is performed as follows:
/// 
///     00 01 08 09 10 11 18 19
///     02 03 0a 0b 12 13 1a 1b
///     04 05 0c 0d 14 15 1c 1d
///     06 07 0e 0f 16 17 1e 1f
///     20 21 28 29 30 31 38 39
///     22 23 2a 2b 32 33 3a 3b
///     24 25 2c 2d 34 35 3c 3d
///     26 27 2e 2f 36 37 3e 3f
///
/// @param [in] a       The input 1D coordinate to remap.
/// 
/// @returns
/// The remapped 2D coordinates.
/// 
/// @ingroup GPUCore
FfxUInt32x2 ffxRemapForWaveReduction(FfxUInt32 a)
{
    return FfxUInt32x2(bitfieldInsertMask(bitfieldExtract(a, 2u, 3u), a, 1u), bitfieldInsertMask(bitfieldExtract(a, 3u, 3u), bitfieldExtract(a, 1u, 2u), 2u));
}

FfxFloat16x4 LoadSrcImageH(FfxFloat32x2 uv, FfxUInt32 slice)
{
    return FfxFloat16x4(rw_input_downsample_src_mips[0][FfxUInt32x3(uv, slice)]);
}

FfxFloat16x4 SpdLoadSourceImageH(FfxInt32x2 tex, FfxUInt32 slice)
{
    return LoadSrcImageH(tex, slice);
}

FfxFloat16x4 SpdReduce4H(FfxFloat16x4 v0, FfxFloat16x4 v1, FfxFloat16x4 v2, FfxFloat16x4 v3)
{
    return (v0 + v1 + v2 + v3) * FfxFloat16(0.25);
}

FfxFloat16x4 SpdReduceLoadSourceImage4H(FfxUInt32x2 i0, FfxUInt32x2 i1, FfxUInt32x2 i2, FfxUInt32x2 i3, FfxUInt32 slice)
{
    FfxFloat16x4 v0 = SpdLoadSourceImageH(FfxInt32x2(i0), slice);
    FfxFloat16x4 v1 = SpdLoadSourceImageH(FfxInt32x2(i1), slice);
    FfxFloat16x4 v2 = SpdLoadSourceImageH(FfxInt32x2(i2), slice);
    FfxFloat16x4 v3 = SpdLoadSourceImageH(FfxInt32x2(i3), slice);
    return SpdReduce4H(v0, v1, v2, v3);
}

FfxFloat16x4 SpdReduceLoadSourceImageH(FfxUInt32x2 base, FfxUInt32 slice)
{
    return SpdReduceLoadSourceImage4H(FfxUInt32x2(base + FfxUInt32x2(0, 0)), FfxUInt32x2(base + FfxUInt32x2(0, 1)), FfxUInt32x2(base + FfxUInt32x2(1, 0)), FfxUInt32x2(base + FfxUInt32x2(1, 1)), slice);
}

void StoreMidMipH(FfxFloat16x4 value, FfxInt32x2 uv, FfxUInt32 slice)
{
    rw_input_downsample_src_mid_mip[FfxUInt32x3(uv, slice)] = FfxFloat32x4(value);
}

void StoreSrcMipH(FfxFloat16x4 value, FfxInt32x2 uv, FfxUInt32 slice, FfxUInt32 mip)
{
    rw_input_downsample_src_mips[mip][FfxUInt32x3(uv, slice)] = FfxFloat32x4(value);
}

void SpdStoreH(FfxInt32x2 pix, FfxFloat16x4 value, FfxUInt32 mip, FfxUInt32 slice)
{
    if (mip == 5)
        StoreMidMipH(value, pix, slice);
    else
        StoreSrcMipH(value, pix, slice, mip + 1);
}

void SpdStoreIntermediateH(FfxUInt32 x, FfxUInt32 y, FfxFloat16x4 value)
{
    spdIntermediateRG[x][y] = value.xy;
    spdIntermediateBA[x][y] = value.zw;
}

void ffxSpdWorkgroupShuffleBarrier()
{
    FFX_GROUP_MEMORY_BARRIER();
}

FfxFloat16x4 SpdLoadIntermediateH(FfxUInt32 x, FfxUInt32 y)
{
    return FfxFloat16x4(
        spdIntermediateRG[x][y].x,
        spdIntermediateRG[x][y].y,
        spdIntermediateBA[x][y].x,
        spdIntermediateBA[x][y].y);
}

FfxFloat16x4 SpdReduceIntermediateH(FfxUInt32x2 i0, FfxUInt32x2 i1, FfxUInt32x2 i2, FfxUInt32x2 i3)
{
    FfxFloat16x4 v0 = SpdLoadIntermediateH(i0.x, i0.y);
    FfxFloat16x4 v1 = SpdLoadIntermediateH(i1.x, i1.y);
    FfxFloat16x4 v2 = SpdLoadIntermediateH(i2.x, i2.y);
    FfxFloat16x4 v3 = SpdLoadIntermediateH(i3.x, i3.y);
    return SpdReduce4H(v0, v1, v2, v3);
}

void SpdDownsampleMips_0_1_LDSH(FfxUInt32 x, FfxUInt32 y, FfxUInt32x2 workGroupID, FfxUInt32 localInvocationIndex, FfxUInt32 mips, FfxUInt32 slice)
{
    FfxFloat16x4 v[4];

    FfxInt32x2 tex = FfxInt32x2(workGroupID.xy * 64) + FfxInt32x2(x * 2, y * 2);
    FfxInt32x2 pix = FfxInt32x2(workGroupID.xy * 32) + FfxInt32x2(x, y);
    v[0]     = SpdReduceLoadSourceImageH(tex, slice);
    SpdStoreH(pix, v[0], 0, slice);

    tex  = FfxInt32x2(workGroupID.xy * 64) + FfxInt32x2(x * 2 + 32, y * 2);
    pix  = FfxInt32x2(workGroupID.xy * 32) + FfxInt32x2(x + 16, y);
    v[1] = SpdReduceLoadSourceImageH(tex, slice);
    SpdStoreH(pix, v[1], 0, slice);

    tex  = FfxInt32x2(workGroupID.xy * 64) + FfxInt32x2(x * 2, y * 2 + 32);
    pix  = FfxInt32x2(workGroupID.xy * 32) + FfxInt32x2(x, y + 16);
    v[2] = SpdReduceLoadSourceImageH(tex, slice);
    SpdStoreH(pix, v[2], 0, slice);

    tex  = FfxInt32x2(workGroupID.xy * 64) + FfxInt32x2(x * 2 + 32, y * 2 + 32);
    pix  = FfxInt32x2(workGroupID.xy * 32) + FfxInt32x2(x + 16, y + 16);
    v[3] = SpdReduceLoadSourceImageH(tex, slice);
    SpdStoreH(pix, v[3], 0, slice);

    if (mips <= 1)
        return;

    for (FfxInt32 i = 0; i < 4; i++)
    {
        SpdStoreIntermediateH(x, y, v[i]);
        ffxSpdWorkgroupShuffleBarrier();
        if (localInvocationIndex < 64)
        {
            v[i] = SpdReduceIntermediateH(FfxUInt32x2(x * 2 + 0, y * 2 + 0), FfxUInt32x2(x * 2 + 1, y * 2 + 0), FfxUInt32x2(x * 2 + 0, y * 2 + 1), FfxUInt32x2(x * 2 + 1, y * 2 + 1));
            SpdStoreH(FfxInt32x2(workGroupID.xy * 16) + FfxInt32x2(x + (i % 2) * 8, y + (i / 2) * 8), v[i], 1, slice);
        }
        ffxSpdWorkgroupShuffleBarrier();
    }

    if (localInvocationIndex < 64)
    {
        SpdStoreIntermediateH(x + 0, y + 0, v[0]);
        SpdStoreIntermediateH(x + 8, y + 0, v[1]);
        SpdStoreIntermediateH(x + 0, y + 8, v[2]);
        SpdStoreIntermediateH(x + 8, y + 8, v[3]);
    }
}

void SpdDownsampleMips_0_1H(FfxUInt32 x, FfxUInt32 y, FfxUInt32x2 workGroupID, FfxUInt32 localInvocationIndex, FfxUInt32 mips, FfxUInt32 slice)
{
    SpdDownsampleMips_0_1_LDSH(x, y, workGroupID, localInvocationIndex, mips, slice);
}

void SpdDownsampleMip_2H(FfxUInt32 x, FfxUInt32 y, FfxUInt32x2 workGroupID, FfxUInt32 localInvocationIndex, FfxUInt32 mip, FfxUInt32 slice)
{
    if (localInvocationIndex < 64)
    {
        FfxFloat16x4 v = SpdReduceIntermediateH(FfxUInt32x2(x * 2 + 0, y * 2 + 0), FfxUInt32x2(x * 2 + 1, y * 2 + 0), FfxUInt32x2(x * 2 + 0, y * 2 + 1), FfxUInt32x2(x * 2 + 1, y * 2 + 1));
        SpdStoreH(FfxInt32x2(workGroupID.xy * 8) + FfxInt32x2(x, y), v, mip, slice);
        // store to LDS, try to reduce bank conflicts
        // x 0 x 0 x 0 x 0 x 0 x 0 x 0 x 0
        // 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
        // 0 x 0 x 0 x 0 x 0 x 0 x 0 x 0 x
        // 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
        // x 0 x 0 x 0 x 0 x 0 x 0 x 0 x 0
        // ...
        // x 0 x 0 x 0 x 0 x 0 x 0 x 0 x 0
        SpdStoreIntermediateH(x * 2 + y % 2, y * 2, v);
    }
}

void SpdDownsampleMip_3H(FfxUInt32 x, FfxUInt32 y, FfxUInt32x2 workGroupID, FfxUInt32 localInvocationIndex, FfxUInt32 mip, FfxUInt32 slice)
{
    if (localInvocationIndex < 16)
    {
        // x 0 x 0
        // 0 0 0 0
        // 0 x 0 x
        // 0 0 0 0
        FfxFloat16x4 v =
            SpdReduceIntermediateH(FfxUInt32x2(x * 4 + 0 + 0, y * 4 + 0), FfxUInt32x2(x * 4 + 2 + 0, y * 4 + 0), FfxUInt32x2(x * 4 + 0 + 1, y * 4 + 2), FfxUInt32x2(x * 4 + 2 + 1, y * 4 + 2));
        SpdStoreH(FfxInt32x2(workGroupID.xy * 4) + FfxInt32x2(x, y), v, mip, slice);
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
        SpdStoreIntermediateH(x * 4 + y, y * 4, v);
    }
}

void SpdDownsampleMip_4H(FfxUInt32 x, FfxUInt32 y, FfxUInt32x2 workGroupID, FfxUInt32 localInvocationIndex, FfxUInt32 mip, FfxUInt32 slice)
{
    if (localInvocationIndex < 4)
    {
        // x 0 0 0 x 0 0 0
        // ...
        // 0 x 0 0 0 x 0 0
        FfxFloat16x4 v = SpdReduceIntermediateH(FfxUInt32x2(x * 8 + 0 + 0 + y * 2, y * 8 + 0),
                                       FfxUInt32x2(x * 8 + 4 + 0 + y * 2, y * 8 + 0),
                                       FfxUInt32x2(x * 8 + 0 + 1 + y * 2, y * 8 + 4),
                                       FfxUInt32x2(x * 8 + 4 + 1 + y * 2, y * 8 + 4));
        SpdStoreH(FfxInt32x2(workGroupID.xy * 2) + FfxInt32x2(x, y), v, mip, slice);
        // store to LDS
        // x x x x 0 ...
        // 0 ...
        SpdStoreIntermediateH(x + y * 2, 0, v);
    }
}

void SpdDownsampleMip_5H(FfxUInt32x2 workGroupID, FfxUInt32 localInvocationIndex, FfxUInt32 mip, FfxUInt32 slice)
{
    if (localInvocationIndex < 1)
    {
        // x x x x 0 ...
        // 0 ...
        FfxFloat16x4 v = SpdReduceIntermediateH(FfxUInt32x2(0, 0), FfxUInt32x2(1, 0), FfxUInt32x2(2, 0), FfxUInt32x2(3, 0));
        SpdStoreH(FfxInt32x2(workGroupID.xy), v, mip, slice);
    }
}

void IncreaseAtomicCounter(FFX_PARAMETER_IN FfxUInt32 slice, FFX_PARAMETER_INOUT FfxUInt32 counter)
{
    InterlockedAdd(rw_internal_global_atomic[0].counter[slice], 1, counter);
}

void SpdIncreaseAtomicCounter(FfxUInt32 slice)
{
    IncreaseAtomicCounter(slice, spdCounter);
}

FfxUInt32 SpdGetAtomicCounter()
{
    return spdCounter;
}

// Only last active workgroup should proceed
bool SpdExitWorkgroup(FfxUInt32 numWorkGroups, FfxUInt32 localInvocationIndex, FfxUInt32 slice)
{
    // global atomic counter
    if (localInvocationIndex == 0)
    {
        SpdIncreaseAtomicCounter(slice);
    }

    ffxSpdWorkgroupShuffleBarrier();
    return (SpdGetAtomicCounter() != (numWorkGroups - 1));
}

void ResetAtomicCounter(FFX_PARAMETER_IN FfxUInt32 slice)
{
    rw_internal_global_atomic[0].counter[slice] = 0;
}

void SpdResetAtomicCounter(FfxUInt32 slice)
{
    ResetAtomicCounter(slice);
}

FfxFloat16x4 LoadMidMipH(FfxInt32x2 uv, FfxUInt32 slice)
{
    return FfxFloat16x4(rw_input_downsample_src_mid_mip[FfxUInt32x3(uv, slice)]);
}

FfxFloat16x4 SpdLoadH(FfxInt32x2 p, FfxUInt32 slice)
{
    return LoadMidMipH(p, slice);
}

FfxFloat16x4 SpdReduceLoad4H(FfxUInt32x2 i0, FfxUInt32x2 i1, FfxUInt32x2 i2, FfxUInt32x2 i3, FfxUInt32 slice)
{
    FfxFloat16x4 v0 = SpdLoadH(FfxInt32x2(i0), slice);
    FfxFloat16x4 v1 = SpdLoadH(FfxInt32x2(i1), slice);
    FfxFloat16x4 v2 = SpdLoadH(FfxInt32x2(i2), slice);
    FfxFloat16x4 v3 = SpdLoadH(FfxInt32x2(i3), slice);
    return SpdReduce4H(v0, v1, v2, v3);
}

FfxFloat16x4 SpdReduceLoad4H(FfxUInt32x2 base, FfxUInt32 slice)
{
    return SpdReduceLoad4H(FfxUInt32x2(base + FfxUInt32x2(0, 0)), FfxUInt32x2(base + FfxUInt32x2(0, 1)), FfxUInt32x2(base + FfxUInt32x2(1, 0)), FfxUInt32x2(base + FfxUInt32x2(1, 1)), slice);
}

void SpdDownsampleMips_6_7H(FfxUInt32 x, FfxUInt32 y, FfxUInt32 mips, FfxUInt32 slice)
{
    FfxInt32x2 tex = FfxInt32x2(x * 4 + 0, y * 4 + 0);
    FfxInt32x2 pix = FfxInt32x2(x * 2 + 0, y * 2 + 0);
    FfxFloat16x4  v0  = SpdReduceLoad4H(tex, slice);
    SpdStoreH(pix, v0, 6, slice);

    tex    = FfxInt32x2(x * 4 + 2, y * 4 + 0);
    pix    = FfxInt32x2(x * 2 + 1, y * 2 + 0);
    FfxFloat16x4 v1 = SpdReduceLoad4H(tex, slice);
    SpdStoreH(pix, v1, 6, slice);

    tex    = FfxInt32x2(x * 4 + 0, y * 4 + 2);
    pix    = FfxInt32x2(x * 2 + 0, y * 2 + 1);
    FfxFloat16x4 v2 = SpdReduceLoad4H(tex, slice);
    SpdStoreH(pix, v2, 6, slice);

    tex    = FfxInt32x2(x * 4 + 2, y * 4 + 2);
    pix    = FfxInt32x2(x * 2 + 1, y * 2 + 1);
    FfxFloat16x4 v3 = SpdReduceLoad4H(tex, slice);
    SpdStoreH(pix, v3, 6, slice);

    if (mips < 8)
        return;
    // no barrier needed, working on values only from the same thread

    FfxFloat16x4 v = SpdReduce4H(v0, v1, v2, v3);
    SpdStoreH(FfxInt32x2(x, y), v, 7, slice);
    SpdStoreIntermediateH(x, y, v);
}

void SpdDownsampleNextFourH(FfxUInt32 x, FfxUInt32 y, FfxUInt32x2 workGroupID, FfxUInt32 localInvocationIndex, FfxUInt32 baseMip, FfxUInt32 mips, FfxUInt32 slice)
{
    if (mips <= baseMip)
        return;
    ffxSpdWorkgroupShuffleBarrier();
    SpdDownsampleMip_2H(x, y, workGroupID, localInvocationIndex, baseMip, slice);

    if (mips <= baseMip + 1)
        return;
    ffxSpdWorkgroupShuffleBarrier();
    SpdDownsampleMip_3H(x, y, workGroupID, localInvocationIndex, baseMip + 1, slice);

    if (mips <= baseMip + 2)
        return;
    ffxSpdWorkgroupShuffleBarrier();
    SpdDownsampleMip_4H(x, y, workGroupID, localInvocationIndex, baseMip + 2, slice);

    if (mips <= baseMip + 3)
        return;
    ffxSpdWorkgroupShuffleBarrier();
    SpdDownsampleMip_5H(workGroupID, localInvocationIndex, baseMip + 3, slice);
}

/// Downsamples a 64x64 tile based on the work group id and work group offset.
/// If after downsampling it's the last active thread group, computes the remaining MIP levels.
/// Uses half types.
///
/// @param [in] workGroupID             index of the work group / thread group
/// @param [in] localInvocationIndex    index of the thread within the thread group in 1D
/// @param [in] mips                    the number of total MIP levels to compute for the input texture
/// @param [in] numWorkGroups           the total number of dispatched work groups / thread groups for this slice
/// @param [in] slice                   the slice of the input texture
///
/// @ingroup FfxGPUSpd
void SpdDownsampleH(FfxUInt32x2 workGroupID, FfxUInt32 localInvocationIndex, FfxUInt32 mips, FfxUInt32 numWorkGroups, FfxUInt32 slice)
{
    FfxUInt32x2        sub_xy = ffxRemapForWaveReduction(localInvocationIndex % 64);
    FfxUInt32 x      = sub_xy.x + 8 * ((localInvocationIndex >> 6) % 2);
    FfxUInt32 y      = sub_xy.y + 8 * ((localInvocationIndex >> 7));

    // compute MIP level 0 and 1
    SpdDownsampleMips_0_1H(x, y, workGroupID, localInvocationIndex, mips, slice);

    // compute MIP level 2, 3, 4, 5
    SpdDownsampleNextFourH(x, y, workGroupID, localInvocationIndex, 2, mips, slice);

    if (mips < 7)
        return;

    // increase the global atomic counter for the given slice and check if it's the last remaining thread group:
    // terminate if not, continue if yes.
    if (SpdExitWorkgroup(numWorkGroups, localInvocationIndex, slice))
        return;

    // reset the global atomic counter back to 0 for the next spd dispatch
    SpdResetAtomicCounter(slice);

    // After mip 5 there is only a single workgroup left that downsamples the remaining up to 64x64 texels.
    // compute MIP level 6 and 7
    SpdDownsampleMips_6_7H(x, y, mips, slice);

    // compute MIP level 8, 9, 10, 11
    SpdDownsampleNextFourH(x, y, FfxUInt32x2(0, 0), localInvocationIndex, 8, mips, slice);
}

/// Downsamples a 64x64 tile based on the work group id and work group offset.
/// If after downsampling it's the last active thread group, computes the remaining MIP levels.
/// Uses half types.
///
/// @param [in] workGroupID             index of the work group / thread group
/// @param [in] localInvocationIndex    index of the thread within the thread group in 1D
/// @param [in] mips                    the number of total MIP levels to compute for the input texture
/// @param [in] numWorkGroups           the total number of dispatched work groups / thread groups for this slice
/// @param [in] slice                   the slice of the input texture
/// @param [in] workGroupOffset         the work group offset. it's (0,0) in case the entire input texture is downsampled.
///
/// @ingroup FfxGPUSpd
void SpdDownsampleH(FfxUInt32x2 workGroupID, FfxUInt32 localInvocationIndex, FfxUInt32 mips, FfxUInt32 numWorkGroups, FfxUInt32 slice, FfxUInt32x2 workGroupOffset)
{
    SpdDownsampleH(workGroupID + workGroupOffset, localInvocationIndex, mips, numWorkGroups, slice);
}

void DOWNSAMPLE(FfxUInt32 LocalThreadId, FfxUInt32x3 WorkGroupId)
{
    SpdDownsampleH(WorkGroupId.xy, LocalThreadId, Mips(), NumWorkGroups(), WorkGroupId.z, WorkGroupOffset());
}

[numthreads(FFX_SPD_THREAD_GROUP_WIDTH, FFX_SPD_THREAD_GROUP_HEIGHT, FFX_SPD_THREAD_GROUP_DEPTH)]
void CS(uint LocalThreadIndex : SV_GroupIndex, uint3 WorkGroupId : SV_GroupID)
{
    DOWNSAMPLE(LocalThreadIndex, WorkGroupId);
}
