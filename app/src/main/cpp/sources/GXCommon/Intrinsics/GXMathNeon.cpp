// version 1.0

#include <GXCommon/GXMath.h>

GX_DISABLE_COMMON_WARNINGS

#include <arm_neon.h>

GX_RESTORE_WARNING_STATE


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
