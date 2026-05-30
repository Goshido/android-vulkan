#include <precompiled_headers.hpp>
#include <crash_dump.hpp>
#include <editor.hpp>

#include <tools/hlsl.hpp>

namespace {

struct Q64 final
{
    uint64_t    _a: 21U = 0U;
    uint64_t    _b: 21U = 0U;
    uint64_t    _c: 22U = 0U;
};

void FUCK () noexcept
{
    constexpr GXQuat cpuQuat ( -0.03054957F, 0.39916795F, 0.25535643F, 0.88007087F );
    GXUBigInt const c64 = cpuQuat.Compress64 ();
    Q64 const q64 = std::bit_cast<Q64> ( c64 );

    using namespace hlsl;
    uint32_t3 const comp = uint32_t3 ( (uint32_t)q64._a, (uint32_t)q64._b, (uint32_t)q64._c );

    // 2 / ( 2 ^ 21 - 1 ) = 9.5367477115381772700201368427929e-7F
    // 2 / ( 2 ^ 22 - 1 ) = 4.7683727188998982667680422706705e-7F
    float32_t3 const imaginary = mad ( (float32_t3)comp, float32_t3 ( 9.53675e-7F, 9.53675e-7F, 4.76837e-7F ), -1.0F );

    // By convention xyz contains imaginary part of quaternion.
    // Real part of quaternion must be restored. By convention it's used unit quaternions to represent rotation.
    // So the real component will be restored using this property.
    // Note dot product could be a little bit bigger than 1.0F due to float32_t inaccuracy. Fixing it with abs.
    float32_t4 const gpuQuat = float32_t4 ( sqrt ( abs ( 1.0F - dot ( imaginary, imaginary ) ) ), imaginary );
    GXVec2 stop {};
}

} // end of anonymous namespace

[[nodiscard]] int main ( int argc, char** argv )
{
    FUCK ();

    if ( !editor::CrashDump::Install () )
        return EXIT_FAILURE;

    constexpr int skipExePath = 1;

    std::unique_ptr<editor::Editor> editor = std::make_unique<editor::Editor> (
        editor::CommandLine ( argv + skipExePath, static_cast<size_t> ( argc - skipExePath ) )
    );

    return editor->Run () ? EXIT_SUCCESS : EXIT_FAILURE;
}
