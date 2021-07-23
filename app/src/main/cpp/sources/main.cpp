#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <android_native_app_glue.h>

GX_RESTORE_WARNING_STATE

#include <core.h>
#include <logger.h>
#include <mandelbrot/mandelbrot_analytic_color.h>
#include <mandelbrot/mandelbrot_lut_color.h>
#include <pbr/pbr_game.h>
#include <pbr/mario/world1x1.h>
#include <pbr/physics/sandbox.h>
#include <rainbow/rainbow.h>
#include <rotating_mesh/game_analytic.h>
#include <rotating_mesh/game_lut.h>

#include <shape_box.h>
#include <shape_sphere.h>


namespace android_vulkan {

enum class eGame : uint16_t
{
    MandelbrotAnalyticColor,
    MandelbrotLutColor,
    PBR,
    PhysicsSandbox,
    Rainbow,
    RotatingMeshAnalytic,
    RotatingMeshLUT,
    World1x1
};

[[maybe_unused]] static void Test () noexcept
{
    RigidBodyRef bodyA = std::make_shared<RigidBody> ();
    bodyA->SetRotation ( GXQuat ( 0.392252475F, 0.0000269473949F, -0.00000322910046F, 0.91985768F ) );
    bodyA->SetLocation ( 1.37368F, 0.147191659F, 0.100026898F );

    ShapeRef shapeA = std::make_shared<ShapeSphere> ( 0.4F );
    bodyA->SetShape ( shapeA );

    RigidBodyRef bodyB = std::make_shared<RigidBody> ();
    bodyB->SetRotation ( GXQuat ( 0.282432348F, -0.926703989F, -0.233253434F, 0.0839324519F ) );
    bodyB->SetLocation ( 0.445023F, 0.325544268F, 0.116885126F );

    ShapeRef shapeB = std::make_shared<ShapeBox> ( 0.899999976F, 0.5F, 0.400000006F );
    bodyB->SetShape ( shapeB );

    ContactManager contactManager {};
    ContactDetector contactDetector {};

    contactDetector.Check ( contactManager, bodyA, bodyB );
    GXVec3 stop {};
}

} // namespace android_vulkan

// Note maybe_unused attribute is needed because IDE could not understand that this function is actually visible for
// NativeActivity implementation.
[[maybe_unused]] void android_main ( android_app* app )
{
    android_vulkan::Test ();

    std::map<android_vulkan::eGame, std::shared_ptr<android_vulkan::Game>> const games =
    {
        { android_vulkan::eGame::MandelbrotAnalyticColor, std::make_shared<mandelbrot::MandelbrotAnalyticColor> () },
        { android_vulkan::eGame::MandelbrotLutColor, std::make_shared<mandelbrot::MandelbrotLUTColor> () },
        { android_vulkan::eGame::PBR, std::make_shared<pbr::PBRGame> () },
        { android_vulkan::eGame::PhysicsSandbox, std::make_shared<pbr::physics::Sandbox> () },
        { android_vulkan::eGame::Rainbow, std::make_shared<rainbow::Rainbow> () },
        { android_vulkan::eGame::RotatingMeshAnalytic, std::make_shared<rotating_mesh::GameAnalytic> () },
        { android_vulkan::eGame::RotatingMeshLUT, std::make_shared<rotating_mesh::GameLUT> () },
        { android_vulkan::eGame::World1x1, std::make_shared<pbr::mario::World1x1> () }
    };

    android_vulkan::Core core ( *app, *( games.find ( android_vulkan::eGame::World1x1 )->second ) );

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
