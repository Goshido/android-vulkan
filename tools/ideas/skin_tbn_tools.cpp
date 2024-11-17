#include <precompiled_headers.hpp>
#include <crash_dump.hpp>
#include <editor.hpp>


namespace {

GXQuat RecoverTBN ( GXVec3 imaginaryTBN ) noexcept
{
    GXQuat res {};
    res._data[ 0U ] = std::sqrt ( std::abs ( 1.0F - imaginaryTBN.DotProduct ( imaginaryTBN ) ) );
    *reinterpret_cast<GXVec3*> ( res._data + 1U ) = imaginaryTBN;
    return res;
}

GXQuat DecompressTBN ( uint32_t compressedTBN ) noexcept
{
    uint32_t const compX = ( compressedTBN & 0x3FF00000U ) >> 20U;
    uint32_t const compY = ( compressedTBN & 0x000FFC00U ) >> 10U;
    uint32_t const compZ = compressedTBN & 0x000003FFU;

    GXVec3 tmp0 ( static_cast<float> ( compX ), static_cast<float> ( compY ), static_cast<float> ( compZ ) );
    tmp0.Sum ( GXVec3 ( -1.0F, -1.0F, -1.0F ), 1.955e-3F, tmp0 );
    return RecoverTBN ( tmp0 );
}

uint32_t CompressTBN ( GXQuat tbn, uint32_t oldCompressedTBN ) noexcept
{
    GXVec3 &img = *reinterpret_cast<GXVec3*> ( tbn._data + 1U );

    if ( tbn._data[ 0U ] < 0.0F )
        img.Reverse ();

    img.Sum ( GXVec3 ( 512.0F, 512.0F, 512.0F ), 511.0F, img );

    uint32_t const unormDataX = static_cast<uint32_t> ( img._data[ 0U ] ) << 20U;
    uint32_t const unormDataY = static_cast<uint32_t> ( img._data[ 1U ] ) << 10U;
    uint32_t const unormDataZ = static_cast<uint32_t> ( img._data[ 2U ] );

    return ( oldCompressedTBN & 0xC0000000U ) | unormDataX | unormDataY | unormDataZ;
}

} // end of anonymous namespace

[[nodiscard]] int main ( int argc, char** argv )
{
    GXVec3 axis ( 5.0F, -2.0F, 1.0F );
    axis.Normalize ();

    GXQuat q {};
    q.FromAxisAngle ( axis, GXDegToRad ( 30.0F ) );

    uint32_t const compressedTBN = q.Compress ( true );
    [[maybe_unused]] GXQuat qDec = DecompressTBN ( compressedTBN );

    [[maybe_unused]] uint32_t const compressedTBNGPU = CompressTBN ( q, compressedTBN );

    if ( !editor::CrashDump::Install () )
        return EXIT_FAILURE;

    constexpr int skipExePath = 1;

    std::unique_ptr<editor::Editor> editor = std::make_unique<editor::Editor> (
        editor::CommandLine ( argv + skipExePath, static_cast<size_t> ( argc - skipExePath ) )
    );

    return editor->Run () ? EXIT_SUCCESS : EXIT_FAILURE;
}
