// version 1.14

#include <precompiled_headers.hpp>
#include <GXCommon/GXMath.hpp>


namespace {

[[nodiscard]] GXBool AABBIsOverlapped ( float32x4_t const &leftA,
    float32x2_t const &leftB,
    float32x4_t const &rightA,
    float32x2_t const &rightB
) noexcept
{
    uint32x4_t const cmp4 = vcleq_f32 ( leftA, rightA );
    uint32x2_t const cmp2 = vcle_f32 ( leftB, rightB );

    uint32x4_t const cmp4Bits = vshrq_n_u32 ( cmp4, 31 );
    uint32x2_t const cmp2Bits = vshr_n_u32 ( cmp2, 31 );

    constexpr int const shift4[ 4U ] = { 0, 1, 2, 3 };
    uint32x4_t const cmp4Lines = vshlq_u32 ( cmp4Bits, vld1q_s32 ( shift4 ) );

    constexpr int const shift2[ 2U ] = { 4, 5 };
    uint32x2_t const cmp2Lines = vshl_u32 ( cmp2Bits, vld1_s32 ( shift2 ) );

    uint32_t const gather4 = vaddvq_u32 ( cmp4Lines );
    uint32_t const gather2 = vaddv_u32 ( cmp2Lines );

    return ( gather4 | gather2 ) == 0b0011'1111U;
}

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXVec2::Reverse () noexcept
{
    vst1_f32 ( _data, vneg_f32 ( vld1_f32 ( _data ) ) );
}

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
    vst1_f32 ( _data, vfma_n_f32 ( vld1_f32 ( a._data ), vld1_f32 ( b._data ), bScale ) );
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
    float32x4_t const tmp = vfmaq_n_f32 ( vld1q_f32 ( a._data ), vld1q_f32 ( b._data ), bScale );

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
    constexpr uint32_t max32U = std::numeric_limits<uint32_t>::max ();
    constexpr uint32_t const maskData[ 4U ] = { max32U, max32U, max32U, 0U };
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
    float32x4_t const result = vfmsq_f32 ( alpha, aShift, bData );

    vst1_f32 ( _data, vget_low_f32 ( result ) );
    vst1_lane_f32 ( _data + 2U, vget_high_f32 ( result ), 0 );
}

[[maybe_unused]] GXVoid GXVec3::LinearInterpolation ( GXVec3 const &start,
    GXVec3 const &finish,
    GXFloat interpolationFactor
) noexcept
{
    // Note vld1q_f32 expects float32_t[ 4U ] array as input. So the 4-th component is garbage but it's not used in
    // computation anyway.
    float32x4_t const s = vld1q_f32 ( start._data );
    float32x4_t const tmp = vfmaq_n_f32 ( s, vsubq_f32 ( vld1q_f32 ( finish._data ), s ), interpolationFactor );

    vst1_f32 ( _data, vget_low_f32 ( tmp ) );
    vst1_lane_s32 ( _data + 2U, vget_high_f32 ( tmp ), 0 );
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXVec4::Sum ( GXVec4 const &a, GXVec4 const &b ) noexcept
{
    vst1q_f32 ( _data, vaddq_f32 ( vld1q_f32 ( a._data ), vld1q_f32 ( b._data ) ) );
}

[[maybe_unused]] GXVoid GXVec4::Sum ( GXVec4 const &a, GXFloat bScale, GXVec4 const &b ) noexcept
{
    vst1q_f32 ( _data, vfmaq_n_f32 ( vld1q_f32 ( a._data ), vld1q_f32 ( b._data ), bScale ) );
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

    constexpr uint32_t max32U = std::numeric_limits<uint32_t>::max ();
    constexpr uint32_t const mask[ 4U ] = { max32U, max32U, 0U, 0U };
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
    vst1q_f32 ( _data, vfmaq_n_f32 ( vld1q_f32 ( a._data ), vld1q_f32 ( b._data ), bScale ) );
    vst1_f32 ( _data + 4U, vfma_n_f32 ( vld1_f32 ( a._data + 4U ), vld1_f32 ( b._data + 4U ), bScale ) );
}

[[maybe_unused]] GXVoid GXVec6::Multiply ( GXVec6 const &a, GXFloat factor ) noexcept
{
    vst1q_f32 ( _data, vmulq_n_f32 ( vld1q_f32 ( a._data ), factor ) );
    vst1_f32 ( _data + 4U, vmul_n_f32 ( vld1_f32 ( a._data + 4U ), factor ) );
}

//----------------------------------------------------------------------------------------------------------------------

// NOLINTNEXTLINE - constructor does not initialize these fields: _data
[[maybe_unused]] GXColorRGB::GXColorRGB ( GXColorUNORM color ) noexcept
{
    float32_t const tmp[ 4U ]
    {
        static_cast<float32_t> ( color._data[ 0U ] ),
        static_cast<float32_t> ( color._data[ 1U ] ),
        static_cast<float32_t> ( color._data[ 2U ] ),
        static_cast<float32_t> ( color._data[ 3U ] )
    };

    vst1q_f32 ( _data, vmulq_n_f32 ( vld1q_f32 ( tmp ), GX_MATH_UNORM_FACTOR ) );
}

[[maybe_unused]] GXVoid GXColorRGB::From ( GXUByte red, GXUByte green, GXUByte blue, GXFloat alpha ) noexcept
{
    float32_t const tmp[ 4U ] =
    {
        static_cast<float32_t> ( red ),
        static_cast<float32_t> ( green ),
        static_cast<float32_t> ( blue ),
        0.0F
    };

    vst1q_f32 ( _data, vmulq_n_f32 ( vld1q_f32 ( tmp ), GX_MATH_UNORM_FACTOR ) );
    _data[ 3U ] = alpha;
}

[[maybe_unused]] GXVoid GXColorRGB::From ( GXUInt red, GXUInt green, GXUInt blue, GXFloat alpha ) noexcept
{
    float32_t const tmp[ 4U ] =
    {
        static_cast<float32_t> ( red ),
        static_cast<float32_t> ( green ),
        static_cast<float32_t> ( blue ),
        0.0F
    };

    vst1q_f32 ( _data, vmulq_n_f32 ( vld1q_f32 ( tmp ), GX_MATH_UNORM_FACTOR ) );
    _data[ 3U ] = alpha;
}

[[maybe_unused]] GXColorUNORM GXColorRGB::ToColorUNORM () const noexcept
{
    constexpr auto convertFactor = static_cast<float> ( std::numeric_limits<uint8_t>::max () );

    float32_t tmp[ 4U ];
    vst1q_f32 ( tmp, vmulq_n_f32 ( vld1q_f32 ( _data ), convertFactor ) );

    return
    {
        static_cast<GXUByte> ( tmp[ 0U ] ),
        static_cast<GXUByte> ( tmp[ 1U ] ),
        static_cast<GXUByte> ( tmp[ 2U ] ),
        static_cast<GXUByte> ( tmp[ 3U ] )
    };
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

[[maybe_unused]] GXVoid GXQuat::SphericalLinearInterpolation ( GXQuat const &start,
    GXQuat const &finish,
    GXFloat interpolationFactor
) noexcept
{
    float32x4_t const s = vld1q_f32 ( start._data );
    float32x4_t const f = vld1q_f32 ( finish._data );
    float cosom = vaddvq_f32 ( vmulq_f32 ( s, f ) );

    float32x4_t const casesQuat[] = { f, vnegq_f32 ( f ) };
    float const casesCosom[] = { cosom, -cosom };

    auto const idx = static_cast<size_t> ( cosom < 0.0F );
    float32x4_t const temp ( casesQuat[ idx ] );
    cosom = casesCosom[ idx ];

    float32_t scale[] = { 1.0F - interpolationFactor, interpolationFactor };

    if ( 1.0F - cosom > GX_MATH_FLOAT_EPSILON ) [[likely]]
    {
        float const angle = std::acos ( cosom );

        float32_t alphaValues[ 2U ];
        vst1_f32 ( alphaValues, vmul_f32 ( vdup_n_f32 ( angle ), vld1_f32 ( scale ) ) );

        float32_t const beta[] = { std::sin ( alphaValues[ 0U ] ), std::sin ( alphaValues[ 1U ] ) };
        vst1_f32 ( scale, vmul_f32 ( vdup_n_f32 ( 1.0F / std::sin ( angle ) ), vld1_f32 ( beta ) ) );
    }

    vst1q_f32 ( _data, vfmaq_n_f32 ( vmulq_n_f32 ( s, scale[ 0U ] ), temp, scale[ 1U ] ) );
}

[[maybe_unused]] GXVoid GXQuat::TransformFast ( GXVec3 &out, GXVec3 const &v ) const noexcept
{
    float32x4_t const rabc = vld1q_f32 ( _data );
    float32x4_t const rabc2 = vaddq_f32 ( rabc, rabc );

    // Note 'T' is just notation for variable separation. Nothing more.
    float32x2_t const ra = vget_low_f32 ( rabc );
    float32x2_t const ar = vext_f32 ( ra, ra, 1 );
    float32x2_t const bc = vget_high_f32 ( rabc );
    float32x2_t const rr = vzip1_f32 ( ra, ra );
    float32x2_t const ca = vext_f32 ( bc, ar, 1 );
    float32x2_t const cb = vext_f32 ( bc, bc, 1 );
    float32x2_t const bc2 = vget_high_f32 ( rabc2 );

    float32_t rXabc2Tr[ 4U ];
    vst1q_f32 ( rXabc2Tr, vmulq_f32 ( vextq_f32 ( rabc2, rabc, 1 ), vcombine_f32 ( rr, rr ) ) );

    float32_t aacaXbc2Tca[ 4U ];
    vst1q_f32 ( aacaXbc2Tca, vmulq_f32 ( vcombine_f32 ( vzip1_f32 ( ar, ar ), ca ), vcombine_f32 ( bc2, ca ) ) );

    float32_t bXbTc2[ 2U ];
    vst1_f32 ( bXbTc2, vmul_f32 ( vzip1_f32 ( bc, bc ), vzip2_f32 ( cb, bc2 ) ) );

    float32_t const tmp0[ 4U ] =
    {
        rXabc2Tr[ 2U ],
        aacaXbc2Tca[ 1U ],
        aacaXbc2Tca[ 0U ],
        rXabc2Tr[ 0U ]
    };

    float32_t const tmp1[ 4U ] =
    {
        aacaXbc2Tca[ 0U ],
        -rXabc2Tr[ 1U ],
        -rXabc2Tr[ 2U ],
        bXbTc2[ 1U ],
    };

    float32x4_t const factor0 = vaddq_f32 ( vld1q_f32 ( tmp0 ), vld1q_f32 ( tmp1 ) );

    float32_t const tmp2[ 2U ] =
    {
        rXabc2Tr[ 1U ],
        bXbTc2[ 1U ],
    };

    float32_t const tmp3[ 2U ] =
    {
        aacaXbc2Tca[ 1U ],
        -rXabc2Tr[ 0U ],
    };

    float32x2_t const factor1 = vadd_f32 ( vld1_f32 ( tmp2 ), vld1_f32 ( tmp3 ) );
    constexpr float32_t whatever = 0.0F;

    float32_t const row0[ 2U ] =
    {
        whatever,
        rXabc2Tr[ 3U ] + aacaXbc2Tca[ 3U ] - bXbTc2[ 0U ] - aacaXbc2Tca[ 2U ]
    };

    float32_t const *vec = v._data;

    float32_t const row1[ 4U ] =
    {
        rXabc2Tr[ 3U ] - aacaXbc2Tca[ 3U ] + bXbTc2[ 0U ] - aacaXbc2Tca[ 2U ],
        whatever,
        whatever,
        whatever
    };

    float32_t *result = out._data;
    float32x4_t const rawRow0 = vcombine_f32 ( vld1_f32 ( row0 ), vget_low_f32 ( factor0 ) );
    float32x4_t const alpha0 = vmulq_n_f32 ( vextq_f32 ( rawRow0, rawRow0, 1 ), vec[ 0U ] );

    float32_t const row2[ 2U ] =
    {
        rXabc2Tr[ 3U ] - aacaXbc2Tca[ 3U ] - bXbTc2[ 0U ] + aacaXbc2Tca[ 2U ],
        whatever
    };

    float32x2_t const sum = vget_high_f32 ( factor0 );
    float32x4_t const alpha1 = vmulq_n_f32 ( vzip1q_f32 ( vcombine_f32 ( sum, sum ), vld1q_f32 ( row1 ) ), vec[ 1U ] );

    float32x4_t const alpha2 = vmulq_n_f32 ( vcombine_f32 ( factor1, vld1_f32 ( row2 ) ), vec[ 2U ] );

    float32x4_t const beta = vaddq_f32 ( alpha0, alpha1 );
    float32x4_t const yotta = vaddq_f32 ( beta, alpha2 );

    vst1_f32 ( result, vget_low_f32 ( yotta ) );
    vst1_lane_s32 ( result + 2U, vget_high_f32 ( yotta ), 0 );
}

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXMat3::Multiply ( GXMat3 const &a, GXMat3 const &b ) noexcept
{
    constexpr uint32_t max32U = std::numeric_limits<uint32_t>::max ();
    constexpr uint32_t const maskData[ 4U ] = { max32U, max32U, max32U, 0U };
    uint32x4_t const mask = vld1q_u32 ( maskData );

    float32x4_t const tmpA0 = vld1q_f32 ( a._data );
    float32x4_t const a0 = vreinterpretq_u32_f32 ( vandq_u32 ( tmpA0, mask ) );
    float32x4_t const tmpB0 = vld1q_f32 ( b._data );
    float32x4_t const b0 = vreinterpretq_u32_f32 ( vandq_u32 ( tmpB0, mask ) );
    float32x4_t c0 = vmulq_laneq_f32 ( b0, a0, 0 );

    float32x4_t const tmpA1 = vld1q_f32 ( a._data + 3U );
    float32x4_t const a1 = vreinterpretq_u32_f32 ( vandq_u32 ( tmpA1, mask ) );
    float32x4_t c1 = vmulq_laneq_f32 ( b0, a1, 0 );

    float32x4_t const tmpB1 = vld1q_f32 ( b._data + 3U );
    float32x4_t const b1 = vreinterpretq_u32_f32 ( vandq_u32 ( tmpB1, mask ) );
    c0 = vfmaq_laneq_f32 ( c0, b1, a0, 1 );
    c1 = vfmaq_laneq_f32 ( c1, b1, a1, 1 );

    float32x4_t const tmpA2 = vld1q_f32 ( a._data + 6U );
    float32x4_t const a2 = vreinterpretq_u32_f32 ( vandq_u32 ( tmpA2, mask ) );
    float32x4_t c2 = vmulq_laneq_f32 ( b0, a2, 0 );
    c2 = vfmaq_laneq_f32 ( c2, b1, a2, 1 );

    float32x4_t const tmpB2 = vld1q_f32 ( b._data + 6U );
    float32x4_t const b2 = vreinterpretq_u32_f32 ( vandq_u32 ( tmpB2, mask ) );
    c0 = vfmaq_laneq_f32 ( c0, b2, a0, 2 );
    c1 = vfmaq_laneq_f32 ( c1, b2, a1, 2 );
    vst1q_f32 ( _data, c0 );

    c2 = vfmaq_laneq_f32 ( c2, b2, a2, 2 );
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
    constexpr uint32_t max32U = std::numeric_limits<uint32_t>::max ();
    constexpr uint32_t const maskData[ 4U ] = { max32U, max32U, max32U, 0U };
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

[[maybe_unused]] GXVoid GXMat4::Inverse ( GXMat4 const &sourceMatrix ) noexcept
{
    // The implementation is based on ideas from
    // https://lxjk.github.io/2017/09/03/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained.html

    // Sub-matrices.
    float32_t const aExtract[ 4U ] =
    {
        sourceMatrix._data[ 0U ],
        sourceMatrix._data[ 1U ],
        sourceMatrix._data[ 4U ],
        sourceMatrix._data[ 5U ]
    };

    float32x4_t const a = vld1q_f32 ( aExtract );

    float32_t const bExtract[ 4U ] =
    {
        sourceMatrix._data[ 2U ],
        sourceMatrix._data[ 3U ],
        sourceMatrix._data[ 6U ],
        sourceMatrix._data[ 7U ]
    };

    float32x4_t const b = vld1q_f32 ( bExtract );

    float32_t const cExtract[ 4U ] =
    {
        sourceMatrix._data[ 8U ],
        sourceMatrix._data[ 9U ],
        sourceMatrix._data[ 12U ],
        sourceMatrix._data[ 13U ]
    };

    float32x4_t const c = vld1q_f32 ( cExtract );

    float32_t const dExtract[ 4U ] =
    {
        sourceMatrix._data[ 10U ],
        sourceMatrix._data[ 11U ],
        sourceMatrix._data[ 14U ],
        sourceMatrix._data[ 15U ]
    };

    float32x4_t const d = vld1q_f32 ( dExtract );

    // Determinants: |A|, |B|, |C| and |D|.
    float32_t const d0[ 4U ] =
    {
        sourceMatrix._data[ 0U ],
        sourceMatrix._data[ 2U ],
        sourceMatrix._data[ 8U ],
        sourceMatrix._data[ 10U ]
    };

    float32_t const d1[ 4U ] =
    {
        sourceMatrix._data[ 5U ],
        sourceMatrix._data[ 7U ],
        sourceMatrix._data[ 13U ],
        sourceMatrix._data[ 15U ]
    };

    float32_t const d2[ 4U ] =
    {
        sourceMatrix._data[ 4U ],
        sourceMatrix._data[ 6U ],
        sourceMatrix._data[ 12U ],
        sourceMatrix._data[ 14U ]
    };

    float32_t const d3[ 4U ] = { sourceMatrix._data[ 1U ], sourceMatrix._data[ 3U ], sourceMatrix._data[ 9U ], sourceMatrix._data[ 11U ] };

    float32x4_t const detComposite = vfmsq_f32 (
        vmulq_f32 ( vld1q_f32 ( d0 ), vld1q_f32 ( d1 ) ), vld1q_f32 ( d2 ), vld1q_f32 ( d3 )
    );

    // Unrolling adjugate by matrix multiplication function for D and C...
    float32x4_t const d3012 = vextq_f32 ( d, d, 3 );
    float32x4_t const c2301 = vextq_f32 ( c, c, 2 );
    float32x4_t const d3300 = vzip1q_f32 ( d3012, d3012 );
    float32x4_t const d1122 = vzip2q_f32 ( d3012, d3012 );

    float32x4_t const dAdjC = vfmsq_f32 ( vmulq_f32 ( d3300, c ), d1122, c2301 );
    float32x4_t const detA = vdupq_laneq_f32 ( detComposite, 0 );

    // Unrolling adjugate by matrix multiplication function for A and B...
    float32x4_t const a3012 = vextq_f32 ( a, a, 3 );
    float32x4_t const b2301 = vextq_f32 ( b, b, 2 );
    float32x4_t const a3300 = vzip1q_f32 ( a3012, a3012 );
    float32x4_t const a1122 = vzip2q_f32 ( a3012, a3012 );

    float32x4_t const aAdjB = vfmsq_f32 ( vmulq_f32 ( a3300, b ), a1122, b2301 );
    float32x4_t const detD = vdupq_laneq_f32 ( detComposite, 3 );

    // X# = |D| A - B ( D# C )
    float32x4_t const detDAFactor = vmulq_f32 ( detD, a );

    // Unrolling matrix by matrix multiplication function for B and ( D# C )...
    float32x4_t const dAdjC1032 = vrev64q_f32 ( dAdjC );
    float32x4_t const b1032 = vrev64q_f32 ( b );
    float32x4_t const dAdjC0321 = vextq_f32 ( dAdjC1032, dAdjC1032, 1 );
    uint64x2_t const dAdjCx0321 = vreinterpretq_u64_f32 ( dAdjC0321 );
    float32x4_t const dAdjC0303 = vreinterpretq_f32_u64 ( vzip1q_u64 ( dAdjCx0321, dAdjCx0321 ) );
    float32x4_t const dAdjC2121 = vreinterpretq_f32_u64 ( vzip2q_u64 ( dAdjCx0321, dAdjCx0321 ) );
    float32x4_t const bDCFactor = vfmaq_f32 ( vmulq_f32 ( b, dAdjC0303 ), b1032, dAdjC2121 );

    float32x4_t const xAdj = vsubq_f32 ( detDAFactor, bDCFactor );
    float32x4_t const detB = vdupq_laneq_f32 ( detComposite, 1 );

    // W# = |A| D - C ( A# B )
    float32x4_t const detADFactor = vmulq_f32 ( detA, d );

    // Unrolling matrix by matrix multiplication function for C and ( A# B )...
    float32x4_t const aAdjB1032 = vrev64q_f32 ( aAdjB );
    float32x4_t const c1032 = vrev64q_f32 ( c );
    float32x4_t const aAdjB0321 = vextq_f32 ( aAdjB1032, aAdjB1032, 1 );
    uint64x2_t const aAdjBx0321 = vreinterpretq_u64_f32 ( aAdjB0321 );
    float32x4_t const aAdjB0303 = vreinterpretq_f32_u64 ( vzip1q_u64 ( aAdjBx0321, aAdjBx0321 ) );
    float32x4_t const aAdjB2121 = vreinterpretq_f32_u64 ( vzip2q_u64 ( aAdjBx0321, aAdjBx0321 ) );
    float32x4_t const cABFactor = vfmaq_f32 ( vmulq_f32 ( c, aAdjB0303 ), c1032, aAdjB2121 );

    float32x4_t const wAdj = vsubq_f32 ( detADFactor, cABFactor );
    float32x4_t const detC = vdupq_laneq_f32 ( detComposite, 2 );

    // Y# = |B| C - D ( A# B )#
    float32x4_t const detBCFactor = vmulq_f32 ( detB, c );

    constexpr uint32_t const all = std::numeric_limits<uint32_t>::max ();
    constexpr uint32_t const maskFirstAndLastRaw[ 4U ] = { all, 0U, 0U, all };
    uint32x4_t const maskFirstAndLast = vld1q_u32 ( maskFirstAndLastRaw );

    // Unrolling matrix by adjugate multiplication function for D and ( A# B )...
    float32x4_t const aAdjB3012 = vextq_f32 ( aAdjB, aAdjB, 3 );
    float32x4_t const d1032 = vrev64q_f32 ( d );
    uint64x2_t const aAdjBx3012 = vreinterpretq_u64_f32 ( aAdjB3012 );
    float32x4_t const aAdjB3030 = vreinterpretq_f32_u64 ( vzip1q_u64 ( aAdjBx3012, aAdjBx3012 ) );
    float32x4_t const dABFactor = vfmsq_f32 ( vmulq_f32 ( d, aAdjB3030 ), d1032, aAdjB2121 );

    float32x4_t const yAdj = vsubq_f32 ( detBCFactor, dABFactor );

    float32_t dABCD[ 4U ];
    vst1q_f32 ( dABCD, detComposite );

    // Z# = |C| B - A ( D# C )#
    float32x4_t const detCBFactor = vmulq_f32 ( detC, b );

    // Unrolling matrix by adjugate multiplication function for A and ( D# C )...
    float32x4_t const dAdjC3012 = vextq_f32 ( dAdjC, dAdjC, 3 );
    float32x4_t const a1032 = vrev64q_f32 ( a );
    uint64x2_t const dAdjCx3012 = vreinterpretq_u64_f32 ( dAdjC3012 );
    float32x4_t const dAdjC3030 = vreinterpretq_f32_u64 ( vzip1q_u64 ( dAdjCx3012, dAdjCx3012 ) );
    float32x4_t const dACFactor = vfmsq_f32 ( vmulq_f32 ( a, dAdjC3030 ), a1032, dAdjC2121 );

    float32x4_t const zAdj = vsubq_f32 ( detCBFactor, dACFactor );

    // |M| = |A| |D| + |B| |C| - tr ( ( A# B) ( D# C ) )
    constexpr uint32_t const maskMiddleRaw[ 4U ] = { 0U, all, all, 0U };
    uint32x4_t const maskMiddle = vld1q_u32 ( maskMiddleRaw );

    uint32x4_t const dAdjCxx0321 = vreinterpretq_u32_u64 ( dAdjC0321 );
    uint32x4_t const dAdjC3210 = vextq_u32 ( dAdjCxx0321, dAdjCxx0321, 1 );
    float32x2_t const ab = vld1_f32 ( dABCD );

    uint32x4_t const dAdjC0XX3 = vandq_u32 ( vreinterpretq_u32_f32 ( dAdjC0303 ), maskFirstAndLast );
    float32x2_t const cd = vld1_f32 ( dABCD + 2U );

    uint32x4_t const dAdjCX21X = vandq_u32 ( dAdjC3210, maskMiddle );
    float32x2_t const dc = vrev64_f32 ( cd );

    uint32x4_t const dAdjC0213 = vorrq_u32 ( dAdjC0XX3, dAdjCX21X );

    float32_t const invDetM = 1.0F / ( vaddv_f32 ( vmul_f32 ( ab, dc ) ) -
                                       vaddvq_f32 ( vmulq_f32 ( aAdjB, vreinterpretq_f32_u32 ( dAdjC0213 ) ) ) );

    float32_t const negInvDetM = -invDetM;
    float32_t const invDetMFactorData[ 4U ] = { invDetM, negInvDetM, negInvDetM, invDetM };
    float32x4_t const invDetMFactor = vld1q_f32 ( invDetMFactorData );

    // X = { _m[ 1U ][ 1U ]     _m[ 0U ][ 1U ]      _m[ 1U ][ 0U ]      _m[ 0U ][ 0U ] }
    float32x4_t const x = vmulq_f32 ( xAdj, invDetMFactor );

    // Y = { _m[ 1U ][ 3U ]     _m[ 0U ][ 3U ]      _m[ 1U ][ 2U ]      _m[ 0U ][ 2U ] }
    float32x4_t const y = vmulq_f32 ( yAdj, invDetMFactor );

    float32x2_t const x02 = vreinterpret_f32_u32 ( vmovn_u64 ( vreinterpretq_u32_f32 ( x ) ) );
    float32x4_t const xShift = vextq_f32 ( x, x, 1 );
    vst1_f32 ( _data + 4U, vrev64_f32 ( x02 ) );

    float32x2_t const x13 = vreinterpret_f32_u32 ( vmovn_u64 ( vreinterpretq_u32_f32 ( xShift ) ) );
    vst1_f32 ( _data, vrev64_f32 ( x13 ) );

    // Z = { _m[ 3U ][ 1U ]     _m[ 2U ][ 1U ]      _m[ 3U ][ 0U ]      _m[ 2U ][ 0U ] }
    float32x4_t const z = vmulq_f32 ( zAdj, invDetMFactor );

    float32x2_t const y02 = vreinterpret_f32_u32 ( vmovn_u64 ( vreinterpretq_u32_f32 ( y ) ) );
    float32x4_t const yShift = vextq_f32 ( y, y, 1 );
    vst1_f32 ( _data + 6U, vrev64_f32 ( y02 ) );

    float32x2_t const y13 = vreinterpret_f32_u32 ( vmovn_u64 ( vreinterpretq_u32_f32 ( yShift ) ) );
    vst1_f32 ( _data + 2U, vrev64_f32 ( y13 ) );

    // W = { _m[ 3U ][ 3U ]     _m[ 2U ][ 3U ]      _m[ 3U ][ 2U ]      _m[ 2U ][ 2U ] }
    float32x4_t const w = vmulq_f32 ( wAdj, invDetMFactor );

    float32x2_t const z02 = vreinterpret_f32_u32 ( vmovn_u64 ( vreinterpretq_u32_f32 ( z ) ) );
    float32x4_t const zShift = vextq_f32 ( z, z, 1 );
    vst1_f32 ( _data + 12U, vrev64_f32 ( z02 ) );

    float32x2_t const z13 = vreinterpret_f32_u32 ( vmovn_u64 ( vreinterpretq_u32_f32 ( zShift ) ) );
    vst1_f32 ( _data + 8U, vrev64_f32 ( z13 ) );

    float32x2_t const w02 = vreinterpret_f32_u32 ( vmovn_u64 ( vreinterpretq_u32_f32 ( w ) ) );
    float32x4_t const wShift = vextq_f32 ( w, w, 1 );
    vst1_f32 ( _data + 14U, vrev64_f32 ( w02 ) );

    float32x2_t const w13 = vreinterpret_f32_u32 ( vmovn_u64 ( vreinterpretq_u32_f32 ( wShift ) ) );
    vst1_f32 ( _data + 10U, vrev64_f32 ( w13 ) );
}

[[maybe_unused]] GXVoid GXMat4::Multiply ( GXMat4 const &a, GXMat4 const &b ) noexcept
{
    // see docs/arm-neon/matrix-multiplication.odt

    float32x4_t const a0 = vld1q_f32 ( a._data );
    float32x4_t const b0 = vld1q_f32 ( b._data );
    float32x4_t c0 = vmulq_laneq_f32 ( b0, a0, 0 );

    float32x4_t const a1 = vld1q_f32 ( a._data + 4U );
    float32x4_t c1 = vmulq_laneq_f32 ( b0, a1, 0 );

    float32x4_t const b1 = vld1q_f32 ( b._data + 4U );
    c0 = vfmaq_laneq_f32 ( c0, b1, a0, 1 );
    c1 = vfmaq_laneq_f32 ( c1, b1, a1, 1 );

    float32x4_t const a2 = vld1q_f32 ( a._data + 8U );
    float32x4_t c2 = vmulq_laneq_f32 ( b0, a2, 0 );
    c2 = vfmaq_laneq_f32 ( c2, b1, a2, 1 );

    float32x4_t const b2 = vld1q_f32 ( b._data + 8U );
    c0 = vfmaq_laneq_f32 ( c0, b2, a0, 2 );
    c1 = vfmaq_laneq_f32 ( c1, b2, a1, 2 );
    c2 = vfmaq_laneq_f32 ( c2, b2, a2, 2 );

    float32x4_t const a3 = vld1q_f32 ( a._data + 12U );
    float32x4_t c3 = vmulq_laneq_f32 ( b0, a3, 0 );
    c3 = vfmaq_laneq_f32 ( c3, b1, a3, 1 );
    c3 = vfmaq_laneq_f32 ( c3, b2, a3, 2 );

    float32x4_t const b3 = vld1q_f32 ( b._data + 12U );
    c0 = vfmaq_laneq_f32 ( c0, b3, a0, 3 );
    c1 = vfmaq_laneq_f32 ( c1, b3, a1, 3 );
    vst1q_f32 ( _data, c0 );

    c2 = vfmaq_laneq_f32 ( c2, b3, a2, 3 );
    vst1q_f32 ( _data + 4U, c1 );

    c3 = vfmaq_laneq_f32 ( c3, b3, a3, 3 );
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

//----------------------------------------------------------------------------------------------------------------------

[[maybe_unused]] GXBool GXAABB::IsOverlapped ( GXAABB const &other ) const noexcept
{
    float32_t const *minBounds = _min._data;
    float32_t const *maxBounds = _max._data;

    float32_t const *otherMinBounds = other._min._data;
    float32_t const *otherMaxBounds = other._max._data;

    float32_t const leftAData[ 4U ] = { minBounds[ 0U ], minBounds[ 1U ], minBounds[ 2U ], otherMinBounds[ 0U ] };

    float32_t const rightAData[ 4U ] =
    {
        otherMaxBounds[ 0U ],
        otherMaxBounds[ 1U ],
        otherMaxBounds[ 2U ],
        maxBounds[ 0U ]
    };

    return AABBIsOverlapped ( vld1q_f32 ( leftAData ),
        vld1_f32 ( otherMinBounds + 1U ),
        vld1q_f32 ( rightAData ),
        vld1_f32 ( maxBounds + 1U )
    );
}

[[maybe_unused]] GXBool GXAABB::IsOverlapped ( GXVec3 const &point ) const noexcept
{
    float32_t const *minBounds = _min._data;
    float32_t const *maxBounds = _max._data;

    float32_t const *p = point._data;

    float32_t const leftAData[ 4U ] = { minBounds[ 0U ], minBounds[ 1U ], minBounds[ 2U ], p[ 0U ] };
    float32_t const rightAData[ 4U ] = { p[ 0U ], p[ 1U ], p[ 2U ], maxBounds[ 0U ] };

    return AABBIsOverlapped ( vld1q_f32 ( leftAData ),
        vld1_f32 ( p + 1U ),
        vld1q_f32 ( rightAData ),
        vld1_f32 ( maxBounds + 1U )
    );
}

[[maybe_unused]] GXBool GXAABB::IsOverlapped ( GXFloat x, GXFloat y, GXFloat z ) const noexcept
{
    float32_t const *minBounds = _min._data;
    float32_t const *maxBounds = _max._data;

    float32_t const leftAData[ 4U ] = { minBounds[ 0U ], minBounds[ 1U ], minBounds[ 2U ], x };
    float32_t const leftBData[ 4U ] = { y, z };
    float32_t const rightAData[ 4U ] = { x, y, z, maxBounds[ 0U ] };

    return AABBIsOverlapped ( vld1q_f32 ( leftAData ),
        vld1_f32 ( leftBData ),
        vld1q_f32 ( rightAData ),
        vld1_f32 ( maxBounds + 1U )
    );
}
