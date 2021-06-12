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
#include <rigid_body.h>
#include <shape_box.h>


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

// Note maybe_unused attribute is needed because IDE could not understand that this function is actually visible for
// NativeActivity implementation.
[[maybe_unused]] void android_main ( android_app* app )
{
    android_vulkan::RigidBody rigidBody {};
    rigidBody.SetMass ( 77.7F );

    android_vulkan::ShapeRef boxShape = std::make_shared<android_vulkan::ShapeBox> ( 0.2F, 0.2F, 0.2F );
    rigidBody.SetShape ( boxShape );

    rigidBody.SetLocation ( 0.0F, 0.0F, 0.0F );

    GXVec3 const point ( 85.3253F, 0.0664F, 40.6206F );
    GXVec3 const impulse ( -96.4106F, 83.0577F, 32.931F );

    rigidBody.AddImpulse ( impulse, point );

    std::map<android_vulkan::eGame, std::shared_ptr<android_vulkan::Game>> const games =
    {
        { android_vulkan::eGame::MandelbrotAnalyticColor, std::make_shared<mandelbrot::MandelbrotAnalyticColor> () },
        { android_vulkan::eGame::MandelbrotLutColor, std::make_shared<mandelbrot::MandelbrotLUTColor> () },
        { android_vulkan::eGame::PBR, std::make_shared<pbr::PBRGame> () },
        { android_vulkan::eGame::Rainbow, std::make_shared<rainbow::Rainbow> () },
        { android_vulkan::eGame::RotatingMeshAnalytic, std::make_shared<rotating_mesh::GameAnalytic> () },
        { android_vulkan::eGame::RotatingMeshLUT, std::make_shared<rotating_mesh::GameLUT> () }
    };

    android_vulkan::Core core ( *app, *( games.find ( android_vulkan::eGame::Rainbow )->second ) );

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
