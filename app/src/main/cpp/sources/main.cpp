#include <core.h>
#include <file.h>
#include <shape_box.h>
#include <mandelbrot/mandelbrot_analytic_color.h>
#include <mandelbrot/mandelbrot_lut_color.h>
#include <pbr/pbr_game.h>
#include <pbr/box_stack/box_stack.h>
#include <pbr/collision/collision.h>
#include <pbr/mario/world1x1.h>
#include <rainbow/rainbow.h>
#include <rotating_mesh/game_analytic.h>
#include <rotating_mesh/game_lut.h>

GX_DISABLE_COMMON_WARNINGS

#include <android_native_app_glue.h>
#include <arm_neon.h>
#include <chrono>
#include <random>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

enum class eGame : uint16_t
{
    Collision,
    BoxStack,
    MandelbrotAnalyticColor,
    MandelbrotLutColor,
    PBR,
    Rainbow,
    RotatingMeshAnalytic,
    RotatingMeshLUT,
    World1x1
};

[[maybe_unused]] static void Inverse ( GXMat4 &dst, GXMat4 const &src ) noexcept
{
    // The implementation is based on ideas from
    // https://lxjk.github.io/2017/09/03/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained.html

    // Sub-matrices.
    float32_t const aExtract[ 4U ] = { src._data[ 0U ], src._data[ 1U ], src._data[ 4U ], src._data[ 5U ] };
    float32x4_t const a = vld1q_f32 ( aExtract );

    float32_t const bExtract[ 4U ] = { src._data[ 2U ], src._data[ 3U ], src._data[ 6U ], src._data[ 7U ] };
    float32x4_t const b = vld1q_f32 ( bExtract );

    float32_t const cExtract[ 4U ] = { src._data[ 8U ], src._data[ 9U ], src._data[ 12U ], src._data[ 13U ] };
    float32x4_t const c = vld1q_f32 ( cExtract );

    float32_t const dExtract[ 4U ] = { src._data[ 10U ], src._data[ 11U ], src._data[ 14U ], src._data[ 15U ] };
    float32x4_t const d = vld1q_f32 ( dExtract );

    // Determinants: |A|, |B|, |C| and |D|.
    float32_t const d0[ 4U ] = { src._data[ 0U ], src._data[ 2U ], src._data[ 8U ], src._data[ 10U ] };
    float32_t const d1[ 4U ] = { src._data[ 5U ], src._data[ 7U ], src._data[ 13U ], src._data[ 15U ] };
    float32_t const d2[ 4U ] = { src._data[ 4U ], src._data[ 6U ], src._data[ 12U ], src._data[ 14U ] };
    float32_t const d3[ 4U ] = { src._data[ 1U ], src._data[ 3U ], src._data[ 9U ], src._data[ 11U ] };

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

    // X = [dst11   dst01   dst10   dst00]
    float32x4_t const x = vmulq_f32 ( xAdj, invDetMFactor );

    // Y = [dst13   dst03   dst12   dst02]
    float32x4_t const y = vmulq_f32 ( yAdj, invDetMFactor );

    float32x2_t const x02 = vreinterpret_f32_u32 ( vmovn_u64 ( vreinterpretq_u32_f32 ( x ) ) );
    float32x4_t const xShift = vextq_f32 ( x, x, 1 );
    vst1_f32 ( dst._data + 4U, vrev64_f32 ( x02 ) );

    float32x2_t const x13 = vreinterpret_f32_u32 ( vmovn_u64 ( vreinterpretq_u32_f32 ( xShift ) ) );
    vst1_f32 ( dst._data, vrev64_f32 ( x13 ) );

    // Z = [dst31   dst21   dst30   dst20]
    float32x4_t const z = vmulq_f32 ( zAdj, invDetMFactor );

    float32x2_t const y02 = vreinterpret_f32_u32 ( vmovn_u64 ( vreinterpretq_u32_f32 ( y ) ) );
    float32x4_t const yShift = vextq_f32 ( y, y, 1 );
    vst1_f32 ( dst._data + 6U, vrev64_f32 ( y02 ) );

    float32x2_t const y13 = vreinterpret_f32_u32 ( vmovn_u64 ( vreinterpretq_u32_f32 ( yShift ) ) );
    vst1_f32 ( dst._data + 2U, vrev64_f32 ( y13 ) );

    // W = [dst33   dst23   dst32   dst22]
    float32x4_t const w = vmulq_f32 ( wAdj, invDetMFactor );

    float32x2_t const z02 = vreinterpret_f32_u32 ( vmovn_u64 ( vreinterpretq_u32_f32 ( z ) ) );
    float32x4_t const zShift = vextq_f32 ( z, z, 1 );
    vst1_f32 ( dst._data + 12U, vrev64_f32 ( z02 ) );

    float32x2_t const z13 = vreinterpret_f32_u32 ( vmovn_u64 ( vreinterpretq_u32_f32 ( zShift ) ) );
    vst1_f32 ( dst._data + 8U, vrev64_f32 ( z13 ) );

    float32x2_t const w02 = vreinterpret_f32_u32 ( vmovn_u64 ( vreinterpretq_u32_f32 ( w ) ) );
    float32x4_t const wShift = vextq_f32 ( w, w, 1 );
    vst1_f32 ( dst._data + 14U, vrev64_f32 ( w02 ) );

    float32x2_t const w13 = vreinterpret_f32_u32 ( vmovn_u64 ( vreinterpretq_u32_f32 ( wShift ) ) );
    vst1_f32 ( dst._data + 10U, vrev64_f32 ( w13 ) );
}

__attribute__((visibility("default"))) void Test ( android_app &app ) noexcept
{
    extern AAssetManager* g_AssetManager;
    g_AssetManager = app.activity->assetManager;
    std::vector<char> content {};

    {
        File dataset ( "math/1" );

        if ( !dataset.LoadContent () )
            return;

        auto const& data = dataset.GetContent ();
        content.resize ( data.size () + 1U );
        content[ data.size () ] = '\0';

        std::memcpy ( content.data (), data.data (), data.size () );
    }

    std::string_view const sv ( content.data (), content.size () );
    std::string_view i = sv;
    size_t counter = 0U;

    for ( ; ; )
    {
        auto r = i.find ( '\n' );

        if ( r == std::string_view::npos )
            break;

        ++counter;
        i = i.substr ( r + 1U );
    }

    size_t const amount = counter / 4U;
    i = sv;
    std::vector<GXMat4> dataset {};
    dataset.reserve ( amount );

    for ( size_t line = 0U; line < amount; ++line )
    {
        GXMat4 get {};
        std::sscanf ( i.data (), "%f %f %f %f", get._data, get._data + 1U, get._data + 2U, get._data + 3U );
        i = i.substr ( i.find ( '\n' ) + 1U );

        std::sscanf ( i.data (), "%f %f %f %f", get._data + 4U, get._data + 5U, get._data + 6U, get._data + 7U );
        i = i.substr ( i.find ( '\n' ) + 1U );

        std::sscanf ( i.data (), "%f %f %f %f", get._data + 8U, get._data + 9U, get._data + 10U, get._data + 11U );
        i = i.substr ( i.find ( '\n' ) + 1U );

        std::sscanf ( i.data (), "%f %f %f %f", get._data + 12U, get._data + 13U, get._data + 14U, get._data + 15U );
        i = i.substr ( i.find ( '\n' ) + 1U );

        dataset.push_back ( get );
    }

    size_t mm;

    {
        File multiply ( "math/2" );

        if ( !multiply.LoadContent () )
            return;

        auto& m = multiply.GetContent ();
        m[ m.size () - 1U ] = 0;

        std::sscanf ( reinterpret_cast<char const*> ( m.data () ), "%zu", &mm );
    }

    std::vector<GXMat4> resultDataset ( amount * mm );
    size_t const rawData = sizeof ( GXMat4 ) * amount;

    for ( size_t step = 0U; step < mm; ++step )
        std::memcpy ( resultDataset.data () + step * amount, dataset.data (), rawData );

    float sur = 1.0F;
    GXMat4 inverseClassical {};
    GXMat4 inverseNew {};

    auto const timePoint1 = std::chrono::high_resolution_clock::now ();

    for ( auto const& matrix : resultDataset )
    {
        inverseClassical.Inverse ( matrix );
        sur *= std::signbit ( inverseClassical._data[ 15U ] ) ? -1.0F : 1.0F;
    }

    auto const timePoint2 = std::chrono::high_resolution_clock::now ();

    for ( auto const& matrix : resultDataset )
    {
        Inverse ( inverseNew, matrix );
        sur *= std::signbit ( inverseNew._data[ 15U ] ) ? -1.0F : 1.0F;
    }

    auto const timePoint3 = std::chrono::high_resolution_clock::now ();

    LogDebug ( "Result %f", sur );

    auto classicDuration = timePoint2 - timePoint1;
    auto neonDuration = timePoint3 - timePoint2;

    constexpr char const format[] =
R"__(Results:
>>>
    Matrices:      %14zu items
    Matrices data: %14g Gb
    Classical:     %14zu nanoseconds
    Neon:          %14zu nanoseconds
<<<)__";

    LogDebug ( format,
        resultDataset.size (),
        ( ( ( static_cast<double> ( resultDataset.size () * sizeof ( GXMat4 ) ) / 1024.0 ) / 1024.0 ) / 1024.0 ),
        std::chrono::duration_cast<std::chrono::nanoseconds> ( classicDuration ).count (),
        std::chrono::duration_cast<std::chrono::nanoseconds> ( neonDuration ).count ()
    );

    GXVec3 const stop {};
}

} // namespace android_vulkan

// Note maybe_unused attribute is needed because IDE could not understand that this function is actually visible for
// NativeActivity implementation.
[[maybe_unused]] void android_main ( android_app* app )
{
    android_vulkan::Test ( *app );

    std::map<android_vulkan::eGame, std::shared_ptr<android_vulkan::Game>> const games =
    {
        { android_vulkan::eGame::Collision, std::make_shared<pbr::collision::Collision> () },
        { android_vulkan::eGame::BoxStack, std::make_shared<pbr::box_stack::BoxStack> () },
        { android_vulkan::eGame::MandelbrotAnalyticColor, std::make_shared<mandelbrot::MandelbrotAnalyticColor> () },
        { android_vulkan::eGame::MandelbrotLutColor, std::make_shared<mandelbrot::MandelbrotLUTColor> () },
        { android_vulkan::eGame::PBR, std::make_shared<pbr::PBRGame> () },
        { android_vulkan::eGame::Rainbow, std::make_shared<rainbow::Rainbow> () },
        { android_vulkan::eGame::RotatingMeshAnalytic, std::make_shared<rotating_mesh::GameAnalytic> () },
        { android_vulkan::eGame::RotatingMeshLUT, std::make_shared<rotating_mesh::GameLUT> () },
        { android_vulkan::eGame::World1x1, std::make_shared<pbr::mario::World1x1> () }
    };

    android_vulkan::Core core ( *app, *( games.find ( android_vulkan::eGame::BoxStack )->second ) );

    for ( ; ; )
    {
        do
        {
            int events;
            android_poll_source* source;

            int const pollResult = ALooper_pollAll ( core.IsSuspend () ? -1 : 0,
                nullptr,
                &events,
                reinterpret_cast<void**> ( &source )
            );

            if ( pollResult < 0 || !source )
                break;

            source->process ( app, source );
        }
        while ( !app->destroyRequested );

        if ( app->destroyRequested )
            break;

        core.OnFrame ();
    }
}
