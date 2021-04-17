// version 1.2

#include <GXCommon/GXMath.h>

GX_DISABLE_COMMON_WARNINGS

#include <cstring>
#include <arm_neon.h>

GX_RESTORE_WARNING_STATE


[[maybe_unused]] GXVoid GXVec2::CalculateNormalFast ( GXVec2 const &a, GXVec2 const &b )
{
    float32_t const alpha[ 2U ] = { a._data[ 1U ], b._data[ 0U ] };
    float32_t const beta[ 2U ] = { b._data[ 1U ], a._data[ 0U ] };
    vst1_f32 ( _data, vsub_f32 ( vld1_f32 ( alpha ), vld1_f32 ( beta ) ) );
}

[[maybe_unused]] GXFloat GXVec2::DotProduct ( GXVec2 const &other ) const
{
    return vaddv_f32 ( vmul_f32 ( vld1_f32 ( _data ), vld1_f32 ( other._data ) ) );
}

[[maybe_unused]] GXVoid GXVec2::Sum ( GXVec2 const &a, GXVec2 const &b )
{
    vst1_f32 ( _data, vadd_f32 ( vld1_f32 ( a._data ), vld1_f32 ( b._data ) ) );
}

[[maybe_unused]] GXVoid GXVec2::Sum ( GXVec2 const &a, GXFloat bScale, GXVec2 const &b )
{
    vst1_f32 ( _data, vmla_n_f32 ( vld1_f32 ( a._data ), vld1_f32 ( b._data ), bScale ) );
}

[[maybe_unused]] GXVoid GXVec2::Subtract ( GXVec2 const &a, GXVec2 const &b )
{
    vst1_f32 ( _data, vsub_f32 ( vld1_f32 ( a._data ), vld1_f32 ( b._data ) ) );
}

[[maybe_unused]] GXVoid GXVec2::Multiply ( GXVec2 const &a, GXVec2 const &b )
{
    vst1_f32 ( _data, vmul_f32 ( vld1_f32 ( a._data ), vld1_f32 ( b._data ) ) );
}

[[maybe_unused]] GXVoid GXVec2::Multiply ( GXVec2 const &v, GXFloat scale )
{
    vst1_f32 ( _data, vmul_n_f32 ( vld1_f32 ( v._data ), scale ) );
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXVec3::Reverse ()
{
    // Note vld1q_f32 expects float32_t[ 4U ] array as input. So the 4-th component is garbage but it's not used in
    // computation anyway.
    float32x4_t const tmp = vnegq_f32 ( vld1q_f32 ( _data ) );

    vst1_f32 ( _data, vget_low_f32 ( tmp ) );
    vst1_lane_s32 ( _data + 2U, vget_high_f32 ( tmp ), 0 );
}

[[maybe_unused]] GXVoid GXVec3::Sum ( GXVec3 const &a, GXVec3 const &b )
{
    // Note vld1q_f32 expects float32_t[ 4U ] array as input. So the 4-th component is garbage but it's not used in
    // computation anyway.
    float32x4_t const tmp = vaddq_f32 ( vld1q_f32 ( a._data ), vld1q_f32 ( b._data ) );

    vst1_f32 ( _data, vget_low_f32 ( tmp ) );
    vst1_lane_s32 ( _data + 2U, vget_high_f32 ( tmp ), 0 );
}

[[maybe_unused]] GXVoid GXVec3::Sum ( GXVec3 const &a, GXFloat bScale, GXVec3 const &b )
{
    // Note vld1q_f32 expects float32_t[ 4U ] array as input. So the 4-th component is garbage but it's not used in
    // computation anyway.
    float32x4_t const tmp = vmlaq_n_f32 ( vld1q_f32 ( a._data ), vld1q_f32 ( b._data ), bScale );

    vst1_f32 ( _data, vget_low_f32 ( tmp ) );
    vst1_lane_s32 ( _data + 2U, vget_high_f32 ( tmp ), 0 );
}

[[maybe_unused]] GXVoid GXVec3::Subtract ( GXVec3 const &a, GXVec3 const &b )
{
    // Note vld1q_f32 expects float32_t[ 4U ] array as input. So the 4-th component is garbage but it's not used in
    // computation anyway.
    float32x4_t const tmp = vsubq_f32 ( vld1q_f32 ( a._data ), vld1q_f32 ( b._data ) );

    vst1_f32 ( _data, vget_low_f32 ( tmp ) );
    vst1_lane_s32 ( _data + 2U, vget_high_f32 ( tmp ), 0 );
}

[[maybe_unused]] GXVoid GXVec3::Multiply ( GXVec3 const &a, GXFloat scale )
{
    // Note vld1q_f32 expects float32_t[ 4U ] array as input. So the 4-th component is garbage but it's not used in
    // computation anyway.
    float32x4_t const tmp = vmulq_n_f32 ( vld1q_f32 ( a._data ), scale );

    vst1_f32 ( _data, vget_low_f32 ( tmp ) );
    vst1_lane_s32 ( _data + 2U, vget_high_f32 ( tmp ), 0 );
}

[[maybe_unused]] GXVoid GXVec3::Multiply ( GXVec3 const &a, GXVec3 const &b )
{
    // Note vld1q_f32 expects float32_t[ 4U ] array as input. So the 4-th component is garbage but it's not used in
    // computation anyway.
    float32x4_t const tmp = vmulq_f32 ( vld1q_f32 ( a._data ), vld1q_f32 ( b._data ) );

    vst1_f32 ( _data, vget_low_f32 ( tmp ) );
    vst1_lane_s32 ( _data + 2U, vget_high_f32 ( tmp ), 0 );
}

[[maybe_unused]] GXFloat GXVec3::DotProduct ( GXVec3 const &other ) const
{
    constexpr uint32_t const maskData[ 4U ] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, 0U };
    uint32x4_t const mask = vld1q_u32 ( maskData );

    uint32x4_t const tmpA = vld1q_u32 ( reinterpret_cast<uint32_t const*> ( _data ) );
    float32x4_t const a = vreinterpretq_u32_f32 ( vandq_u32 ( tmpA, mask ) );

    uint32x4_t const tmpB = vld1q_u32 ( reinterpret_cast<uint32_t const*> ( other._data ) );
    float32x4_t const b = vreinterpretq_u32_f32 ( vandq_u32 ( tmpB, mask ) );

    return vaddvq_f32 ( vmulq_f32 ( a, b ) );
}

[[maybe_unused]] GXVoid GXVec3::CrossProduct ( GXVec3 const &a, GXVec3 const &b )
{
    // The implementation is based on
    // https://developer.arm.com/documentation/den0018/a/NEON-Code-Examples-with-Mixed-Operations/Cross-product/Single-cross-product

    // aData = [Ay, Ax, Az, Ay]
    float32x4_t const aData = vcombine_f32 ( vld1_f32 ( a._data + 1U ), vld1_f32 ( a._data ) );

    // bData = [By, Bx, Bz, By]
    float32x4_t const bData = vcombine_f32 ( vld1_f32 ( b._data + 1U ), vld1_f32 ( b._data ) );

    // aShift = [Ay, Ay, Ax, Az]
    float32x4_t const aShift = vextq_f32 ( aData, aData, 1 );

    // bShift = [By, By, Bx, Bz]
    float32x4_t const bShift = vextq_f32 ( bData, bData, 1 );

    // alpha = [AyBy AxBy AzBx AyBz]
    float32x4_t const alpha = vmulq_f32 ( aData, bShift );

    // res = [AyBy - AyBy, AxBy - AyBx, AzBx - AxBz, AyBz - AzBy]
    // "AyBy - AyBy" is a side effect of the computation.
    float32x4_t const result = vmlsq_f32 ( alpha, aShift, bData );

    vst1_f32 ( _data, vget_low_f32 ( result ) );
    vst1_lane_f32 ( _data + 2U, vget_high_f32 ( result ), 0 );
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXMat4::Multiply ( GXMat4 const &a, GXMat4 const &b )
{
    // see docs/arm-neon/matrix-multiplication.odt

    float32x4_t c0 = vmovq_n_f32 ( 0.0F );
    float32x4_t c1 = vmovq_n_f32 ( 0.0F );
    float32x4_t c2 = vmovq_n_f32 ( 0.0F );
    float32x4_t c3 = vmovq_n_f32 ( 0.0F );

    float32x4_t const a0 = vld1q_f32 ( a._data );
    float32x4_t const a1 = vld1q_f32 ( a._data + 4U );
    float32x4_t const a2 = vld1q_f32 ( a._data + 8U );
    float32x4_t const a3 = vld1q_f32 ( a._data + 12U );

    float32x4_t const b0 = vld1q_f32 ( b._data );
    float32x4_t const b1 = vld1q_f32 ( b._data + 4U );
    float32x4_t const b2 = vld1q_f32 ( b._data + 8U );
    float32x4_t const b3 = vld1q_f32 ( b._data + 12U );

    c0 = vfmaq_laneq_f32 ( c0, b0, a0, 0 );
    c0 = vfmaq_laneq_f32 ( c0, b1, a0, 1 );
    c0 = vfmaq_laneq_f32 ( c0, b2, a0, 2 );
    c0 = vfmaq_laneq_f32 ( c0, b3, a0, 3 );

    c1 = vfmaq_laneq_f32 ( c1, b0, a1, 0 );
    c1 = vfmaq_laneq_f32 ( c1, b1, a1, 1 );
    c1 = vfmaq_laneq_f32 ( c1, b2, a1, 2 );
    c1 = vfmaq_laneq_f32 ( c1, b3, a1, 3 );

    c2 = vfmaq_laneq_f32 ( c2, b0, a2, 0 );
    c2 = vfmaq_laneq_f32 ( c2, b1, a2, 1 );
    c2 = vfmaq_laneq_f32 ( c2, b2, a2, 2 );
    c2 = vfmaq_laneq_f32 ( c2, b3, a2, 3 );

    c3 = vfmaq_laneq_f32 ( c3, b0, a3, 0 );
    c3 = vfmaq_laneq_f32 ( c3, b1, a3, 1 );
    c3 = vfmaq_laneq_f32 ( c3, b2, a3, 2 );
    c3 = vfmaq_laneq_f32 ( c3, b3, a3, 3 );

    vst1q_f32 ( _data, c0 );
    vst1q_f32 ( _data + 4U, c1 );
    vst1q_f32 ( _data + 8U, c2 );
    vst1q_f32 ( _data + 12U, c3 );
}
