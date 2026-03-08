#include <precompiled_headers.hpp>
#include <crash_dump.hpp>
#include <editor.hpp>


namespace {

struct Q final
{
    float   _data[ 4U ];

    [[nodiscard]] float Length () const noexcept;
    [[nodiscard]] uint32_t Compress () const noexcept;

    [[nodiscard]] static Q Decompress ( uint32_t compressed ) noexcept;
};

float Q::Length () const noexcept
{
    auto const& v4 = *reinterpret_cast<GXVec4 const*> ( _data );
    return v4.Length ();
}

uint32_t Q::Compress () const noexcept
{
    GXVec3 imaginaryABC = *reinterpret_cast<GXVec3 const*> ( _data + 1U );

    // Shader code expects only positive real value for reconstruction.
    // Using quaternion duality property to satisfy that convention.
    if ( _data[ 0U ] < 0.0F )
        imaginaryABC.Reverse ();

    constexpr auto conv = [] ( uint32_t bits ) consteval -> std::pair<float, GXVec2>
    {
        uint32_t const halfFixedPoint = 1U << ( bits - 1U );
        auto const offset = static_cast<float> ( halfFixedPoint );
        return std::make_pair ( static_cast<float> ( halfFixedPoint - 1U ), GXVec2 ( offset, offset ) );
    };

    auto const [scale10, offset10] = conv ( 10U );
    auto const [scale11, offset11] = conv ( 11U );

    auto &imaginaryAB = *reinterpret_cast<GXVec2*> ( imaginaryABC._data );
    imaginaryAB.Sum ( offset11, scale11, imaginaryAB );

    auto const aSnorm = static_cast<uint32_t> ( imaginaryAB._data[ 0U ] );
    auto const bSnorm = static_cast<uint32_t> ( imaginaryAB._data[ 1U ] );
    auto const cSnorm = static_cast<uint32_t> ( imaginaryABC._data[ 2U ] * scale10 + offset10._data[ 0U ] );

    return ( aSnorm << 21U ) | ( bSnorm << 10U ) | cSnorm;
}

Q Q::Decompress ( uint32_t compressed ) noexcept
{
    uint32_t const compX = ( compressed & 0xFFE0'0000U ) >> 21U;
    uint32_t const compY = ( compressed & 0x001F'FC00U ) >> 10U;
    uint32_t const compZ = compressed & 0x0000'03FFU;

    // 2 / ( ( 2 ^ 10 ) - 1 ) = 1.955034213098729227761485826e-3
    constexpr float norm10 = 1.955034e-3F;

    // 2 / ( ( 2 ^ 11 ) - 1 ) = 9.7703957010258915486077186126038e-4
    constexpr auto norm11 = 9.770396e-4F;

    Q result {};
    result._data[ 1U ] = static_cast<float> ( compX ) * norm11 - 1.0F;
    result._data[ 2U ] = static_cast<float> ( compY ) * norm11 - 1.0F;
    result._data[ 3U ] = static_cast<float> ( compZ ) * norm10 - 1.0F;

    GXVec3 const &imaginary = *reinterpret_cast<GXVec3 const*> ( result._data + 1U );
    result._data[ 0U ] = std::sqrt ( std::abs ( 1.0F - imaginary.DotProduct ( imaginary ) ) );

    return result;
}

struct Q64 final
{
    float   _data[ 4U ];

    struct Data final
    {
        uint32_t    _high = 0U;
        uint32_t    _low = 0U;
    };

    [[nodiscard]] float Length () const noexcept;

    // A21 B21 C22
    [[nodiscard]] Data Compress () const noexcept;

    [[nodiscard]] static Q64 Decompress ( Data compressed ) noexcept;
};

float Q64::Length () const noexcept
{
    auto const& v4 = *reinterpret_cast<GXVec4 const*> ( _data );
    return v4.Length ();
}

Q64::Data Q64::Compress () const noexcept
{
    GXVec3 imaginaryABC = *reinterpret_cast<GXVec3 const*> ( _data + 1U );

    // Shader code expects only positive real value for reconstruction.
    // Using quaternion duality property to satisfy that convention.
    if ( _data[ 0U ] < 0.0F )
        imaginaryABC.Reverse ();

    constexpr auto conv = [] ( uint32_t bits ) consteval -> std::pair<float, GXVec2>
    {
        uint32_t const halfFixedPoint = 1U << ( bits - 1U );
        auto const offset = static_cast<float> ( halfFixedPoint );
        return std::make_pair ( static_cast<float> ( halfFixedPoint - 1U ), GXVec2 ( offset, offset ) );
    };

    auto const [scale21, offset21] = conv ( 21U );
    auto const [scale22, offset22] = conv ( 22U );

    auto &imaginaryAB = *reinterpret_cast<GXVec2*> ( imaginaryABC._data );
    imaginaryAB.Sum ( offset21, scale21, imaginaryAB );

    auto const aSnorm = static_cast<uint32_t> ( imaginaryAB._data[ 0U ] );
    auto const bSnorm = static_cast<uint32_t> ( imaginaryAB._data[ 1U ] );
    auto const cSnorm = static_cast<uint32_t> ( imaginaryABC._data[ 2U ] * scale22 + offset22._data[ 0U ] );

    Data result {};
    result._low = ( aSnorm << 11U ) | ( bSnorm >> 10U );
    result._high = ( bSnorm << 22U ) | cSnorm;

    return result;
}

Q64 Q64::Decompress ( Data compressed ) noexcept
{
    uint32_t const compressedLow = compressed._low;
    uint32_t const compressedHigh = compressed._high;

    uint32_t const compX = compressedLow >> 11U;
    uint32_t const compY = ( ( compressedLow & 0x0000'07FFU ) << 10U ) | ( compressedHigh >> 22U );
    uint32_t const compZ = compressedHigh & 0x003F'FFFFU;

    // 2 / ( ( 2 ^ 21 ) - 1 ) = 9.5367477115381772700201368427929e-7
    constexpr float norm21 = 9.53675e-7F;

    // 2 / ( ( 2 ^ 22 ) - 1 ) = 4.7683727188998982667680422706705e-7
    constexpr auto norm22 = 4.76837e-7F;

    Q64 result {};
    result._data[ 1U ] = static_cast<float> ( compX ) * norm21 - 1.0F;
    result._data[ 2U ] = static_cast<float> ( compY ) * norm21 - 1.0F;
    result._data[ 3U ] = static_cast<float> ( compZ ) * norm22 - 1.0F;

    GXVec3 const &imaginary = *reinterpret_cast<GXVec3 const*> ( result._data + 1U );
    result._data[ 0U ] = std::sqrt ( std::abs ( 1.0F - imaginary.DotProduct ( imaginary ) ) );

    return result;
}

void Fuck () noexcept
{
    GXQuat q ( -0.03054957F, 0.39916795F, 0.25535643F, 0.88007087F );
    Q qqqq {};
    std::memcpy ( qqqq._data, q._data, sizeof ( q ) );
    uint32_t const q32 = qqqq.Compress ();
    Q const recoverQ = Q::Decompress ( q32 );
    float const qLength = recoverQ.Length ();
    std::printf ( "%f %f", recoverQ._data[ 0U ], qLength );

    Q64 qqqq64 {};
    std::memcpy ( qqqq64._data, q._data, sizeof ( q ) );
    Q64::Data const q64 = qqqq64.Compress ();
    Q64 const recoverQ64 = Q64::Decompress ( q64 );
    float const qLength64 = recoverQ64.Length ();
    std::printf ( "%f %f", recoverQ64._data[ 0U ], qLength64 );

    GXVec3 x ( 6.43F, 3.0F, -7.77F );
    GXVec3 y ( -2.3F, 0.0F, 3.33F );
    GXVec3 z {};

    GXVec3::MakeOrthonormalBasis ( x, y, z );
    GXMat3 r {};
    r.SetX ( x );
    r.SetY ( y );
    r.SetZ ( z );

    GXQuat q0 {};
    q0.FromFast ( r );

    constexpr GXVec3 xb ( 1.0F, 0.0F, 0.0F );
    constexpr GXVec3 yb ( 0.0F, 1.0F, 0.0F );
    constexpr GXVec3 zb ( 0.0F, 0.0F, 1.0F );

    GXVec3 xR;
    GXVec3 yR;
    GXVec3 zR;

    q0.TransformFast ( xR, xb );
    q0.TransformFast ( yR, yb );
    q0.TransformFast ( zR, zb );

    std::printf ( "sss" );
}

} // end of anonymous namespace

[[nodiscard]] int main ( int argc, char** argv )
{
    Fuck ();

    if ( !editor::CrashDump::Install () )
        return EXIT_FAILURE;

    constexpr int skipExePath = 1;

    std::unique_ptr<editor::Editor> editor = std::make_unique<editor::Editor> (
        editor::CommandLine ( argv + skipExePath, static_cast<size_t> ( argc - skipExePath ) )
    );

    return editor->Run () ? EXIT_SUCCESS : EXIT_FAILURE;
}
