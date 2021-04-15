// version 1.1

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
    float32_t tmp[ 4U ];

    // Note vld1q_f32 expects float32_t[ 4U ] array as input. So the 4-th component is garbage but it's not used in
    // computation anyway.
    vst1q_f32 ( tmp, vnegq_f32 ( vld1q_f32 ( _data ) ) );

    memcpy ( _data, tmp, sizeof ( _data ) );
}

[[maybe_unused]] GXVoid GXVec3::Sum ( GXVec3 const &a, GXVec3 const &b )
{
    float32_t tmp[ 4U ];

    // Note vld1q_f32 expects float32_t[ 4U ] array as input. So the 4-th component is garbage but it's not used in
    // computation anyway.
    vst1q_f32 ( tmp, vaddq_f32 ( vld1q_f32 ( a._data ), vld1q_f32 ( b._data ) ) );

    memcpy ( _data, tmp, sizeof ( _data ) );
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
