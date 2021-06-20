#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <android_native_app_glue.h>

GX_RESTORE_WARNING_STATE

#include <core.h>
#include <logger.h>
#include <mandelbrot/mandelbrot_analytic_color.h>
#include <mandelbrot/mandelbrot_lut_color.h>
#include <pbr/pbr_game.h>
#include <rainbow/rainbow.h>
#include <rotating_mesh/game_analytic.h>
#include <rotating_mesh/game_lut.h>


namespace android_vulkan {

enum class eGame : uint16_t
{
    MandelbrotAnalyticColor,
    MandelbrotLutColor,
    PBR,
    Rainbow,
    RotatingMeshAnalytic,
    RotatingMeshLUT
};

} // namespace android_vulkan

static void Test () noexcept
{
    constexpr static float const FACE_PERPENDICULAR_TOLERANCE = 1.0e-3F;

    constexpr GXVec3 const edge[] =
    {
        GXVec3 ( 384.894F, 6073.819F, 60.32F ),
        GXVec3 ( 382.701F, 6093.949F, -4.151F )
    };

    constexpr GXVec3 const face[] =
    {
        GXVec3 ( 365.055F, 6081.716F, 46.038F ),
        GXVec3 ( 384.961F, 6053.014F, 19.846F ),
        GXVec3 ( 417.912F, 6089.035F, 5.414F ),
        GXVec3 ( 398.006F, 6117.737F, 31.606F )
    };

    struct Ct
    {
        GXVec3      _point;
    };

    Ct c;
    Ct* contact = &c;

    //------------------------------------------------------------------------------------------------------------------

    GXVec3 edgeDir {};
    edgeDir.Subtract ( edge[ 1U ], edge[ 0U ] );

    GXVec3 ab {};
    ab.Subtract ( face[ 0U ], face[ 1U ] );

    GXVec3 ac {};
    ac.Subtract ( face[ 2U ], face[ 1U ] );

    GXVec3 faceNormal;
    faceNormal.CrossProduct ( ab, ac );

    if ( std::abs ( faceNormal.DotProduct ( edgeDir ) ) >= FACE_PERPENDICULAR_TOLERANCE )
    {
        // The edge pierces the face. There is only one contact point.
        // So we need to find ray vs plane intersection point.
        // https://www.cs.princeton.edu/courses/archive/fall00/cs426/lectures/raycast/sld017.htm

        float const d = faceNormal.DotProduct ( face[ 1U ] );
        float const t = ( d - edge[ 0U ].DotProduct ( faceNormal ) ) / edgeDir.DotProduct ( faceNormal );
        contact->_point.Sum ( edge[ 0U ], t, edgeDir );
        GXVec3 stop {};
    }
}

// Note maybe_unused attribute is needed because IDE could not understand that this function is actually visible for
// NativeActivity implementation.
[[maybe_unused]] void android_main ( android_app* app )
{
    Test ();

    std::map<android_vulkan::eGame, std::shared_ptr<android_vulkan::Game>> const games =
    {
        { android_vulkan::eGame::MandelbrotAnalyticColor, std::make_shared<mandelbrot::MandelbrotAnalyticColor> () },
        { android_vulkan::eGame::MandelbrotLutColor, std::make_shared<mandelbrot::MandelbrotLUTColor> () },
        { android_vulkan::eGame::PBR, std::make_shared<pbr::PBRGame> () },
        { android_vulkan::eGame::Rainbow, std::make_shared<rainbow::Rainbow> () },
        { android_vulkan::eGame::RotatingMeshAnalytic, std::make_shared<rotating_mesh::GameAnalytic> () },
        { android_vulkan::eGame::RotatingMeshLUT, std::make_shared<rotating_mesh::GameLUT> () }
    };

    android_vulkan::Core core ( *app, *( games.find ( android_vulkan::eGame::PBR )->second ) );

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
