#include <precompiled_headers.hpp>
#include <crash_dump.hpp>
#include <editor.hpp>


namespace {

void FUCK () noexcept
{

#pragma pack ( push, 1 )

    union Q final
    {
        struct GPU final
        {
            uint32_t _q0High;
            uint32_t _q0Low;
            uint32_t _q1High;
            uint32_t _q1Low;
        }
        _gpu;

        struct CPU final
        {
            uint64_t _q0;
            uint64_t _q1;
        }
        _cpu;
    };

#pragma pack ( pop )

    Q interop;

    GXVec3 axis0 ( 5.0F, -3.0F, 1.0F );
    axis0.Normalize ();
    GXQuat q0 {};
    q0.FromAxisAngle ( axis0, GXDegToRad ( 33.0F ) );
    interop._cpu._q0 = q0.Compress64 ();

    GXVec3 axis1 ( 42.0F, 40.0F, -51.0F );
    axis1.Normalize ();
    GXQuat q1 {};
    q1.FromAxisAngle ( axis1, GXDegToRad ( -77.0F ) );
    interop._cpu._q1 = q1.Compress64 ();

    constexpr auto unpack = [] ( Q const &q, uint32_t idx ) noexcept -> GXQuat {
        uint32_t const highCases[] = { q._gpu._q0High, q._gpu._q1High };
        uint32_t const lowCases[] = { q._gpu._q0Low, q._gpu._q1Low };

        uint32_t const high = highCases[ idx ];
        uint32_t const low = lowCases[ idx ];

        auto const rSnorm = static_cast<uint16_t> ( low >> 16U );
        auto const aSnorm = static_cast<uint16_t> ( low & 0x0000FFFFU );
        auto const bSnorm = static_cast<uint16_t> ( high >> 16U );
        auto const cSnorm = static_cast<uint16_t> ( high & 0x0000FFFFU );

        GXVec4 v ( static_cast<float> ( rSnorm ),
            static_cast<float> ( aSnorm ),
            static_cast<float> ( bSnorm ),
            static_cast<float> ( cSnorm )
        );

        constexpr float scale = 2.0F / ( static_cast<float> ( ( 1U << 16U ) - 1U ) );
        constexpr GXVec4 offset ( -1.0F, -1.0F, -1.0F, -1.0F );
        v.Sum ( offset, scale, v );

        return *reinterpret_cast<GXQuat const*> ( &v );
    };

    GXQuat const res0 = unpack ( interop, 0U );
    GXQuat const res1 = unpack ( interop, 1U );

    std::printf ( "%f %f", res0._data[ 1U ], res1._data[ 0U ] );
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
