// version 1.5

#include <GXCommon/GXMath.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>
#include <cstring>
#include <arm_neon.h>

GX_RESTORE_WARNING_STATE


[[maybe_unused]] GXVoid GXVec2::CalculateNormalFast ( GXVec2 const &a, GXVec2 const &b ) noexcept
{
    float32_t const alpha[ 2U ] = { a._data[ 1U ], b._data[ 0U ] };
    float32_t const beta[ 2U ] = { b._data[ 1U ], a._data[ 0U ] };
    vst1_f32 ( _data, vsub_f32 ( vld1_f32 ( alpha ), vld1_f32 ( beta ) ) );
}

[[maybe_unused]] GXFloat GXVec2::DotProduct ( GXVec2 const &other ) const noexcept
{
    return vaddv_f32 ( vmul_f32 ( vld1_f32 ( _data ), vld1_f32 ( other._data ) ) );
}

[[maybe_unused]] GXVoid GXVec2::Sum ( GXVec2 const &a, GXVec2 const &b ) noexcept
{
    vst1_f32 ( _data, vadd_f32 ( vld1_f32 ( a._data ), vld1_f32 ( b._data ) ) );
}

[[maybe_unused]] GXVoid GXVec2::Sum ( GXVec2 const &a, GXFloat bScale, GXVec2 const &b ) noexcept
{
    vst1_f32 ( _data, vmla_n_f32 ( vld1_f32 ( a._data ), vld1_f32 ( b._data ), bScale ) );
}

[[maybe_unused]] GXVoid GXVec2::Subtract ( GXVec2 const &a, GXVec2 const &b ) noexcept
{
    vst1_f32 ( _data, vsub_f32 ( vld1_f32 ( a._data ), vld1_f32 ( b._data ) ) );
}

[[maybe_unused]] GXVoid GXVec2::Multiply ( GXVec2 const &a, GXVec2 const &b ) noexcept
{
    vst1_f32 ( _data, vmul_f32 ( vld1_f32 ( a._data ), vld1_f32 ( b._data ) ) );
}

[[maybe_unused]] GXVoid GXVec2::Multiply ( GXVec2 const &v, GXFloat scale ) noexcept
{
    vst1_f32 ( _data, vmul_n_f32 ( vld1_f32 ( v._data ), scale ) );
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXVec3::Reverse () noexcept
{
    // Note vld1q_f32 expects float32_t[ 4U ] array as input. So the 4-th component is garbage but it's not used in
    // computation anyway.
    float32x4_t const tmp = vnegq_f32 ( vld1q_f32 ( _data ) );

    vst1_f32 ( _data, vget_low_f32 ( tmp ) );
    vst1_lane_s32 ( _data + 2U, vget_high_f32 ( tmp ), 0 );
}

[[maybe_unused]] GXVoid GXVec3::Sum ( GXVec3 const &a, GXVec3 const &b ) noexcept
{
    // Note vld1q_f32 expects float32_t[ 4U ] array as input. So the 4-th component is garbage but it's not used in
    // computation anyway.
    float32x4_t const tmp = vaddq_f32 ( vld1q_f32 ( a._data ), vld1q_f32 ( b._data ) );

    vst1_f32 ( _data, vget_low_f32 ( tmp ) );
    vst1_lane_s32 ( _data + 2U, vget_high_f32 ( tmp ), 0 );
}

[[maybe_unused]] GXVoid GXVec3::Sum ( GXVec3 const &a, GXFloat bScale, GXVec3 const &b ) noexcept
{
    // Note vld1q_f32 expects float32_t[ 4U ] array as input. So the 4-th component is garbage but it's not used in
    // computation anyway.
    float32x4_t const tmp = vmlaq_n_f32 ( vld1q_f32 ( a._data ), vld1q_f32 ( b._data ), bScale );

    vst1_f32 ( _data, vget_low_f32 ( tmp ) );
    vst1_lane_s32 ( _data + 2U, vget_high_f32 ( tmp ), 0 );
}

[[maybe_unused]] GXVoid GXVec3::Subtract ( GXVec3 const &a, GXVec3 const &b ) noexcept
{
    // Note vld1q_f32 expects float32_t[ 4U ] array as input. So the 4-th component is garbage but it's not used in
    // computation anyway.
    float32x4_t const tmp = vsubq_f32 ( vld1q_f32 ( a._data ), vld1q_f32 ( b._data ) );

    vst1_f32 ( _data, vget_low_f32 ( tmp ) );
    vst1_lane_s32 ( _data + 2U, vget_high_f32 ( tmp ), 0 );
}

[[maybe_unused]] GXVoid GXVec3::Multiply ( GXVec3 const &a, GXFloat scale ) noexcept
{
    // Note vld1q_f32 expects float32_t[ 4U ] array as input. So the 4-th component is garbage but it's not used in
    // computation anyway.
    float32x4_t const tmp = vmulq_n_f32 ( vld1q_f32 ( a._data ), scale );

    vst1_f32 ( _data, vget_low_f32 ( tmp ) );
    vst1_lane_s32 ( _data + 2U, vget_high_f32 ( tmp ), 0 );
}

[[maybe_unused]] GXVoid GXVec3::Multiply ( GXVec3 const &a, GXVec3 const &b ) noexcept
{
    // Note vld1q_f32 expects float32_t[ 4U ] array as input. So the 4-th component is garbage but it's not used in
    // computation anyway.
    float32x4_t const tmp = vmulq_f32 ( vld1q_f32 ( a._data ), vld1q_f32 ( b._data ) );

    vst1_f32 ( _data, vget_low_f32 ( tmp ) );
    vst1_lane_s32 ( _data + 2U, vget_high_f32 ( tmp ), 0 );
}

[[maybe_unused]] GXFloat GXVec3::DotProduct ( GXVec3 const &other ) const noexcept
{
    constexpr uint32_t const maskData[ 4U ] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, 0U };
    uint32x4_t const mask = vld1q_u32 ( maskData );

    uint32x4_t const tmpA = vld1q_u32 ( reinterpret_cast<uint32_t const*> ( _data ) );
    float32x4_t const a = vreinterpretq_u32_f32 ( vandq_u32 ( tmpA, mask ) );

    uint32x4_t const tmpB = vld1q_u32 ( reinterpret_cast<uint32_t const*> ( other._data ) );
    float32x4_t const b = vreinterpretq_u32_f32 ( vandq_u32 ( tmpB, mask ) );

    return vaddvq_f32 ( vmulq_f32 ( a, b ) );
}

[[maybe_unused]] GXVoid GXVec3::CrossProduct ( GXVec3 const &a, GXVec3 const &b ) noexcept
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

[[maybe_unused]] GXVoid GXVec4::Sum ( GXVec4 const &a, GXVec4 const &b ) noexcept
{
    vst1q_f32 ( _data, vaddq_f32 ( vld1q_f32 ( a._data ), vld1q_f32 ( b._data ) ) );
}

[[maybe_unused]] GXVoid GXVec4::Sum ( GXVec4 const &a, GXFloat bScale, GXVec4 const &b ) noexcept
{
    vst1q_f32 ( _data, vmlaq_n_f32 ( vld1q_f32 ( a._data ), vld1q_f32 ( b._data ), bScale ) );
}

[[maybe_unused]] GXVoid GXVec4::Subtract ( GXVec4 const &a, GXVec4 const &b ) noexcept
{
    vst1q_f32 ( _data, vsubq_f32 ( vld1q_f32 ( a._data ), vld1q_f32 ( b._data ) ) );
}

[[maybe_unused]] GXFloat GXVec4::DotProduct ( GXVec4 const &other ) const noexcept
{
    return vaddvq_f32 ( vmulq_f32 ( vld1q_f32 ( _data ), vld1q_f32 ( other._data ) ) );
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXFloat GXVec6::DotProduct ( GXVec6 const &other ) const noexcept
{
    float32x4_t const part1 = vmulq_f32 ( vld1q_f32 ( _data ), vld1q_f32 ( other._data ) );

    // Note vld1q_f32 expects float32_t[ 4U ] array as input. So the 3-rd and 4-th components are garbage
    // but they are not used in computation anyway because of masking later.
    float32x4_t const part2Dirty = vmulq_f32 ( vld1q_f32 ( _data + 4U ), vld1q_f32 ( other._data + 4U ) );

    constexpr uint32_t const mask[ 4U ] = { UINT32_MAX, UINT32_MAX, 0U, 0U };
    uint32x4_t const part2 = vandq_u32 ( vld1q_u32 ( mask ), vreinterpretq_f32_u32 ( part2Dirty ) );

    return vaddvq_f32 ( vaddq_f32 ( part1, vreinterpretq_u32_f32 ( part2 ) ) );
}

[[maybe_unused]] GXVoid GXVec6::Sum ( GXVec6 const &a, GXVec6 const &b ) noexcept
{
    vst1q_f32 ( _data, vaddq_f32 ( vld1q_f32 ( a._data ), vld1q_f32 ( b._data ) ) );
    vst1_f32 ( _data + 4U, vadd_f32 ( vld1_f32 ( a._data + 4U ), vld1_f32 ( b._data + 4U ) ) );
}

[[maybe_unused]] GXVoid GXVec6::Sum ( GXVec6 const &a, GXFloat bScale, GXVec6 const &b ) noexcept
{
    vst1q_f32 ( _data, vmlaq_n_f32 ( vld1q_f32 ( a._data ), vld1q_f32 ( b._data ), bScale ) );
    vst1_f32 ( _data + 4U, vmla_n_f32 ( vld1_f32 ( a._data + 4U ), vld1_f32 ( b._data + 4U ), bScale ) );
}

[[maybe_unused]] GXVoid GXVec6::Multiply ( GXVec6 const &a, GXFloat factor ) noexcept
{
    vst1q_f32 ( _data, vmulq_n_f32 ( vld1q_f32 ( a._data ), factor ) );
    vst1_f32 ( _data + 4U, vmul_n_f32 ( vld1_f32 ( a._data + 4U ), factor ) );
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXColorRGB::From ( GXUByte red, GXUByte green, GXUByte blue, GXFloat alpha ) noexcept
{
    float32_t const tmp[ 4U ] =
    {
        static_cast<float32_t> ( red ),
        static_cast<float32_t> ( green ),
        static_cast<float32_t> ( blue ),
        alpha
    };

    vst1q_f32 ( _data, vmulq_n_f32 ( vld1q_f32 ( tmp ), GX_MATH_UNORM_FACTOR ) );
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXQuat::Normalize () noexcept
{
    float32x4_t const alpha = vld1q_f32 ( _data );
    float32_t squaredLength = vaddvq_f32 ( vmulq_f32 ( alpha, alpha ) );

    assert ( squaredLength > GX_MATH_FLOAT_EPSILON );

    vst1q_f32 ( _data, vmulq_n_f32 ( alpha, 1.0F / std::sqrt ( squaredLength ) ) );
}

[[maybe_unused]] GXVoid GXQuat::Inverse ( GXQuat const &q ) noexcept
{
    float32x4_t const alpha = vld1q_f32 ( q._data );
    float32_t squaredLength = vaddvq_f32 ( vmulq_f32 ( alpha, alpha ) );

    assert ( squaredLength > GX_MATH_FLOAT_EPSILON );

    vst1q_f32 ( _data, vmulq_n_f32 ( alpha, -1.0F / squaredLength ) );
    _data[ 0U ] = -_data[ 0U ];
}

[[maybe_unused]] GXVoid GXQuat::FromAxisAngle ( GXFloat x, GXFloat y, GXFloat z, GXFloat angle ) noexcept
{
    float const halfAngle = 0.5F * angle;

    float32_t const alpha[ 4U ] = { 0.0F, x, y, z };
    vst1q_f32 ( _data, vmulq_n_f32 ( vld1q_f32 ( alpha ), std::sin ( halfAngle ) ) );

    _data[ 0U ] = std::cos ( halfAngle );
}

[[maybe_unused]] GXVoid GXQuat::Multiply ( GXQuat const &q, GXFloat scale ) noexcept
{
    vst1q_f32 ( _data, vmulq_n_f32 ( vld1q_f32 ( q._data ), scale ) );
}

[[maybe_unused]] GXVoid GXQuat::Sum ( GXQuat const &a, GXQuat const &b ) noexcept
{
    vst1q_f32 ( _data, vaddq_f32 ( vld1q_f32 ( a._data ), vld1q_f32 ( b._data ) ) );
}

[[maybe_unused]] GXVoid GXQuat::Subtract ( GXQuat const &a, GXQuat const &b ) noexcept
{
    vst1q_f32 ( _data, vsubq_f32 ( vld1q_f32 ( a._data ), vld1q_f32 ( b._data ) ) );
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXMat3::Multiply ( GXMat3 const &a, GXMat3 const &b ) noexcept
{
    constexpr uint32_t const maskData[ 4U ] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, 0U };
    uint32x4_t const mask = vld1q_u32 ( maskData );

    float32x4_t const tmpA0 = vld1q_f32 ( a._data );
    float32x4_t const a0 = vreinterpretq_u32_f32 ( vandq_u32 ( tmpA0, mask ) );

    float32x4_t const tmpA1 = vld1q_f32 ( a._data + 3U );
    float32x4_t const a1 = vreinterpretq_u32_f32 ( vandq_u32 ( tmpA1, mask ) );

    float32x4_t const tmpA2 = vld1q_f32 ( a._data + 6U );
    float32x4_t const a2 = vreinterpretq_u32_f32 ( vandq_u32 ( tmpA2, mask ) );

    float32x4_t const tmpB0 = vld1q_f32 ( b._data );
    float32x4_t const b0 = vreinterpretq_u32_f32 ( vandq_u32 ( tmpB0, mask ) );

    float32x4_t const tmpB1 = vld1q_f32 ( b._data + 3U );
    float32x4_t const b1 = vreinterpretq_u32_f32 ( vandq_u32 ( tmpB1, mask ) );

    float32x4_t const tmpB2 = vld1q_f32 ( b._data + 6U );
    float32x4_t const b2 = vreinterpretq_u32_f32 ( vandq_u32 ( tmpB2, mask ) );

    float32x4_t c0 = vmovq_n_f32 ( 0.0F );
    float32x4_t c1 = vmovq_n_f32 ( 0.0F );
    float32x4_t c2 = vmovq_n_f32 ( 0.0F );

    c0 = vfmaq_laneq_f32 ( c0, b0, a0, 0 );
    c1 = vfmaq_laneq_f32 ( c1, b0, a1, 0 );
    c2 = vfmaq_laneq_f32 ( c2, b0, a2, 0 );

    c0 = vfmaq_laneq_f32 ( c0, b1, a0, 1 );
    c1 = vfmaq_laneq_f32 ( c1, b1, a1, 1 );
    c2 = vfmaq_laneq_f32 ( c2, b1, a2, 1 );

    c0 = vfmaq_laneq_f32 ( c0, b2, a0, 2 );
    c1 = vfmaq_laneq_f32 ( c1, b2, a1, 2 );
    c2 = vfmaq_laneq_f32 ( c2, b2, a2, 2 );

    vst1q_f32 ( _data, c0 );
    vst1q_f32 ( _data + 3U, c1 );

    vst1_f32 ( _data + 6U, vget_low_f32 ( c2 ) );
    vst1_lane_s32 ( _data + 8U, vget_high_f32 ( c2 ), 0 );
}

[[maybe_unused]] GXVoid GXMat3::MultiplyVectorMatrix ( GXVec3 &out, GXVec3 const &v ) const noexcept
{
    float32x4_t const a0 = vmulq_n_f32 ( vld1q_f32 ( _data ), v._data[ 0U ] );
    float32x4_t const a1 = vmulq_n_f32 ( vld1q_f32 ( _data + 3U ), v._data[ 1U ] );
    float32x4_t const a2 = vmulq_n_f32 ( vld1q_f32 ( _data + 6U ), v._data[ 2U ] );

    float32x4_t const b = vaddq_f32 ( a0, a1 );
    float32x4_t const c = vaddq_f32 ( b, a2 );

    vst1_f32 ( out._data, vget_low_f32 ( c ) );
    vst1_lane_s32 ( out._data + 2U, vget_high_f32 ( c ), 0 );
}

[[maybe_unused]] GXVoid GXMat3::MultiplyMatrixVector ( GXVec3 &out, GXVec3 const &v ) const noexcept
{
    constexpr uint32_t const maskData[ 4U ] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, 0U };
    uint32x4_t const mask = vld1q_u32 ( maskData );

    float32x4_t const tmpM0 = vld1q_f32 ( _data );
    float32x4_t const m0 = vreinterpretq_u32_f32 ( vandq_u32 ( tmpM0, mask ) );

    float32x4_t const tmpM1 = vld1q_f32 ( _data + 3U );
    float32x4_t const m1 = vreinterpretq_u32_f32 ( vandq_u32 ( tmpM1, mask ) );

    float32x4_t const tmpM2 = vld1q_f32 ( _data + 6U );
    float32x4_t const m2 = vreinterpretq_u32_f32 ( vandq_u32 ( tmpM2, mask ) );

    float32x4_t const tmpVec = vld1q_f32 ( v._data );
    float32x4_t const vec = vreinterpretq_u32_f32 ( vandq_u32 ( tmpVec, mask ) );

    out._data[ 0U ] = vaddvq_f32 ( vmulq_f32 ( m0, vec ) );
    out._data[ 1U ] = vaddvq_f32 ( vmulq_f32 ( m1, vec ) );
    out._data[ 2U ] = vaddvq_f32 ( vmulq_f32 ( m2, vec ) );
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXMat4::Multiply ( GXMat4 const &a, GXMat4 const &b ) noexcept
{
    // see docs/arm-neon/matrix-multiplication.odt

    float32x4_t const a0 = vld1q_f32 ( a._data );
    float32x4_t const a1 = vld1q_f32 ( a._data + 4U );
    float32x4_t const a2 = vld1q_f32 ( a._data + 8U );
    float32x4_t const a3 = vld1q_f32 ( a._data + 12U );

    float32x4_t const b0 = vld1q_f32 ( b._data );
    float32x4_t const b1 = vld1q_f32 ( b._data + 4U );
    float32x4_t const b2 = vld1q_f32 ( b._data + 8U );
    float32x4_t const b3 = vld1q_f32 ( b._data + 12U );

    float32x4_t c0 = vmovq_n_f32 ( 0.0F );
    float32x4_t c1 = vmovq_n_f32 ( 0.0F );
    float32x4_t c2 = vmovq_n_f32 ( 0.0F );
    float32x4_t c3 = vmovq_n_f32 ( 0.0F );

    c0 = vfmaq_laneq_f32 ( c0, b0, a0, 0 );
    c1 = vfmaq_laneq_f32 ( c1, b0, a1, 0 );
    c2 = vfmaq_laneq_f32 ( c2, b0, a2, 0 );
    c3 = vfmaq_laneq_f32 ( c3, b0, a3, 0 );

    c0 = vfmaq_laneq_f32 ( c0, b1, a0, 1 );
    c1 = vfmaq_laneq_f32 ( c1, b1, a1, 1 );
    c2 = vfmaq_laneq_f32 ( c2, b1, a2, 1 );
    c3 = vfmaq_laneq_f32 ( c3, b1, a3, 1 );

    c0 = vfmaq_laneq_f32 ( c0, b2, a0, 2 );
    c1 = vfmaq_laneq_f32 ( c1, b2, a1, 2 );
    c2 = vfmaq_laneq_f32 ( c2, b2, a2, 2 );
    c3 = vfmaq_laneq_f32 ( c3, b2, a3, 2 );

    c0 = vfmaq_laneq_f32 ( c0, b3, a0, 3 );
    c1 = vfmaq_laneq_f32 ( c1, b3, a1, 3 );
    c2 = vfmaq_laneq_f32 ( c2, b3, a2, 3 );
    c3 = vfmaq_laneq_f32 ( c3, b3, a3, 3 );

    vst1q_f32 ( _data, c0 );
    vst1q_f32 ( _data + 4U, c1 );
    vst1q_f32 ( _data + 8U, c2 );
    vst1q_f32 ( _data + 12U, c3 );
}

[[maybe_unused]] GXVoid GXMat4::MultiplyVectorMatrix ( GXVec4 &out, GXVec4 const &v ) const noexcept
{
    float32x4_t const a0 = vmulq_n_f32 ( vld1q_f32 ( _data ), v._data[ 0U ] );
    float32x4_t const a1 = vmulq_n_f32 ( vld1q_f32 ( _data + 4U ), v._data[ 1U ] );
    float32x4_t const a2 = vmulq_n_f32 ( vld1q_f32 ( _data + 8U ), v._data[ 2U ] );
    float32x4_t const a3 = vmulq_n_f32 ( vld1q_f32 ( _data + 12U ), v._data[ 3U ] );

    float32x4_t const b0 = vaddq_f32 ( a0, a1 );
    float32x4_t const b1 = vaddq_f32 ( a2, a3 );

    vst1q_f32 ( out._data, vaddq_f32 ( b0, b1 ) );
}

[[maybe_unused]] GXVoid GXMat4::MultiplyMatrixVector ( GXVec4 &out, GXVec4 const &v ) const noexcept
{
    float32x4_t const vec = vld1q_f32 ( v._data );

    out._data[ 0U ] = vaddvq_f32 ( vmulq_f32 ( vld1q_f32 ( _data ), vec ) );
    out._data[ 1U ] = vaddvq_f32 ( vmulq_f32 ( vld1q_f32 ( _data + 4U ), vec ) );
    out._data[ 2U ] = vaddvq_f32 ( vmulq_f32 ( vld1q_f32 ( _data + 8U ), vec ) );
    out._data[ 3U ] = vaddvq_f32 ( vmulq_f32 ( vld1q_f32 ( _data + 12U ), vec ) );
}

[[maybe_unused]] GXVoid GXMat4::MultiplyAsNormal ( GXVec3 &out, GXVec3 const &v ) const noexcept
{
    float32x4_t const a0 = vmulq_n_f32 ( vld1q_f32 ( _data ), v._data[ 0U ] );
    float32x4_t const a1 = vmulq_n_f32 ( vld1q_f32 ( _data + 4U ), v._data[ 1U ] );
    float32x4_t const a2 = vmulq_n_f32 ( vld1q_f32 ( _data + 8U ), v._data[ 2U ] );

    float32x4_t const b = vaddq_f32 ( a0, a1 );
    float32x4_t const c = vaddq_f32 ( b, a2 );

    vst1_f32 ( out._data, vget_low_f32 ( c ) );
    vst1_lane_s32 ( out._data + 2U, vget_high_f32 ( c ), 0 );
}

[[maybe_unused]] GXVoid GXMat4::MultiplyAsPoint ( GXVec3 &out, GXVec3 const &v ) const noexcept
{
    float32x4_t const a0 = vmulq_n_f32 ( vld1q_f32 ( _data ), v._data[ 0U ] );
    float32x4_t const a1 = vmulq_n_f32 ( vld1q_f32 ( _data + 4U ), v._data[ 1U ] );
    float32x4_t const a2 = vmulq_n_f32 ( vld1q_f32 ( _data + 8U ), v._data[ 2U ] );

    float32x4_t const b0 = vaddq_f32 ( vld1q_f32 ( _data + 12U ), a0 );
    float32x4_t const b1 = vaddq_f32 ( a1, a2 );

    float32x4_t const c = vaddq_f32 ( b0, b1 );

    vst1_f32 ( out._data, vget_low_f32 ( c ) );
    vst1_lane_s32 ( out._data + 2U, vget_high_f32 ( c ), 0 );
}
