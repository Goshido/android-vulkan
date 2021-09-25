#include <core.h>
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

} // namespace android_vulkan

// Note maybe_unused attribute is needed because IDE could not understand that this function is actually visible for
// NativeActivity implementation.
[[maybe_unused]] void android_main ( android_app* app )
{
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
