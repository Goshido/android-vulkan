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

static const std::map<eGame, std::shared_ptr<Game>> g_Games =
{
    { eGame::MandelbrotAnalyticColor, std::make_shared<mandelbrot::MandelbrotAnalyticColor> () },
    { eGame::MandelbrotLutColor, std::make_shared<mandelbrot::MandelbrotLUTColor> () },
    { eGame::PBR, std::make_shared<pbr::PBRGame> () },
    { eGame::Rainbow, std::make_shared<rainbow::Rainbow> () },
    { eGame::RotatingMeshAnalytic, std::make_shared<rotating_mesh::GameAnalytic> () },
    { eGame::RotatingMeshLUT, std::make_shared<rotating_mesh::GameLUT> () }
};

} // namespace android_vulkan

void android_main ( android_app* app )
{

#ifdef ANDROID_VULKAN_DEBUG

    android_vulkan::LogDebug ( "android_main - Application was started." );

#endif // ANDROID_VULKAN_DEBUG

    android_vulkan::Core core ( *app,
        *( android_vulkan::g_Games.find ( android_vulkan::eGame::RotatingMeshLUT )->second )
    );

    for ( ; ; )
    {
        do
        {
            int events;
            android_poll_source* source;

            const int pollResult = ALooper_pollAll ( core.IsSuspend () ? -1 : 0,
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

#ifdef ANDROID_VULKAN_DEBUG

    android_vulkan::LogDebug ( "android_main - Application was finished." );

#endif // ANDROID_VULKAN_DEBUG

}
