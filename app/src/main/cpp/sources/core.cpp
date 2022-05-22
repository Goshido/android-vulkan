#include <core.h>
#include <logger.h>
#include <mandelbrot/mandelbrot_analytic_color.h>
#include <mandelbrot/mandelbrot_lut_color.h>
#include <pbr/pbr_game.h>
#include <pbr/box_stack/box_stack.h>
#include <pbr/collision/collision.h>
#include <pbr/mario/world1x1.h>
#include <pbr/ray_casting/ray_casting.h>
#include <pbr/stipple_test/stipple_test.h>
#include <pbr/sweep_testing/sweep_testing.h>
#include <rainbow/rainbow.h>
#include <rotating_mesh/game_analytic.h>
#include <rotating_mesh/game_lut.h>

GX_DISABLE_COMMON_WARNINGS

#include <android/asset_manager_jni.h>
#include <android/native_window_jni.h>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

AAssetManager* g_AssetManager = nullptr;
Core* g_Core = nullptr;
constexpr static double const FPS_PERIOD = 3.0;
constexpr static auto TIMEOUT = std::chrono::milliseconds ( 10U );

enum class eGame : uint16_t
{
    Collision,
    BoxStack,
    MandelbrotAnalyticColor,
    MandelbrotLutColor,
    PBR,
    Rainbow,
    RayCasting,
    RotatingMeshAnalytic,
    RotatingMeshLUT,
    StippleTest,
    SweepTesting,
    World1x1
};

//----------------------------------------------------------------------------------------------------------------------

Core::Core ( JNIEnv* env, jobject assetManager ) noexcept
{
    _assetManagerJVM = assetManager;
    env->NewGlobalRef ( assetManager );
    g_AssetManager = AAssetManager_fromJava ( env, assetManager );

    InitCommandHandlers ();

    static std::map<android_vulkan::eGame, std::shared_ptr<android_vulkan::Game>> const games =
    {
        { android_vulkan::eGame::Collision, std::make_shared<pbr::collision::Collision> () },
        { android_vulkan::eGame::BoxStack, std::make_shared<pbr::box_stack::BoxStack> () },
        { android_vulkan::eGame::MandelbrotAnalyticColor, std::make_shared<mandelbrot::MandelbrotAnalyticColor> () },
        { android_vulkan::eGame::MandelbrotLutColor, std::make_shared<mandelbrot::MandelbrotLUTColor> () },
        { android_vulkan::eGame::PBR, std::make_shared<pbr::PBRGame> () },
        { android_vulkan::eGame::Rainbow, std::make_shared<rainbow::Rainbow> () },
        { android_vulkan::eGame::RayCasting, std::make_shared<pbr::ray_casting::RayCasting> () },
        { android_vulkan::eGame::RotatingMeshAnalytic, std::make_shared<rotating_mesh::GameAnalytic> () },
        { android_vulkan::eGame::RotatingMeshLUT, std::make_shared<rotating_mesh::GameLUT> () },
        { android_vulkan::eGame::StippleTest, std::make_shared<pbr::stipple_test::StippleTest> () },
        { android_vulkan::eGame::SweepTesting, std::make_shared<pbr::sweep_testing::SweepTesting> () },
        { android_vulkan::eGame::World1x1, std::make_shared<pbr::mario::World1x1> () }
    };

    _game = games.find ( android_vulkan::eGame::World1x1 )->second.get ();

    _thread = std::thread (
        [ this ] () noexcept {
            if ( !_renderer.OnCreateDevice () )
                return;

            if ( !_game->OnInitDevice ( _renderer ) )
                return;

            while ( ExecuteMessageQueue () )
            {
                // C++ calling method by pointer syntax.
                ( this->*_renderBodyHandler ) ();
            }
        }
    );
}

void Core::OnAboutDestroy ( JNIEnv* env ) noexcept
{
    {
        std::unique_lock<std::mutex> const lock ( _mutex );
        _writeQueue.push_back ( eCommand::Quit );
    }

    while ( _thread.joinable () )
        std::this_thread::sleep_for ( TIMEOUT );

    ANativeWindow_release ( _nativeWindow );
    _nativeWindow = nullptr;

    env->DeleteGlobalRef ( _assetManagerJVM );
    _assetManagerJVM = nullptr;
    g_AssetManager = nullptr;
}

void Core::OnSurfaceCreated ( JNIEnv* env, jobject surface ) noexcept
{
    _nativeWindow = ANativeWindow_fromSurface ( env, surface );
    std::unique_lock<std::mutex> const lock ( _mutex );
    _writeQueue.push_back ( eCommand::SwapchainCreated );
}

void Core::OnSurfaceDestroyed () noexcept
{
    {
        std::unique_lock<std::mutex> const lock ( _mutex );
        _writeQueue.push_back ( eCommand::SwapchainDestroyed );
    }

    while ( !_readQueue.empty () | !_writeQueue.empty () )
        std::this_thread::sleep_for ( TIMEOUT );

    ANativeWindow_release ( _nativeWindow );
}

bool Core::ExecuteMessageQueue () noexcept
{
    {
        std::unique_lock<std::mutex> const lock ( _mutex );
        _readQueue.swap ( _writeQueue );
    }

    bool result = true;

    for ( auto const command: _readQueue )
    {
        CommandHandler const handler = _commandHandlers[ static_cast<size_t> ( command ) ];

        // C++ calling method by pointer syntax.
        result &= ( this->*handler ) ();
    }

    _readQueue.clear ();
    return result;
}

void Core::InitCommandHandlers () noexcept
{
    _commandHandlers[ static_cast<size_t> ( eCommand::SwapchainCreated ) ] = &Core::OnSwapchainCreated;
    _commandHandlers[ static_cast<size_t> ( eCommand::SwapchainDestroyed ) ] = &Core::OnSwapchainDestroyed;
    _commandHandlers[ static_cast<size_t> ( eCommand::Quit ) ] = &Core::OnQuit;
}

void Core::OnFrame () noexcept
{
    if ( !_game->IsReady () )
        return;

    Timestamp const now = std::chrono::system_clock::now ();
    std::chrono::duration<double> const delta = now - _frameTimestamp;

    if ( _renderer.CheckSwapchainStatus () && !_game->OnFrame ( _renderer, delta.count () ) )
        LogError ( "Core::OnFrame - Frame rendering failed." );

    _frameTimestamp = now;
    UpdateFPS ( now );
}

// NOLINTNEXTLINE - method can be static
void Core::OnIdle () noexcept
{
    std::this_thread::sleep_for ( TIMEOUT );
}

bool Core::OnQuit () noexcept
{
    _game->OnDestroyDevice ( _renderer.GetDevice () );
    _renderer.OnDestroyDevice ();

    return false;
}

bool Core::OnSwapchainCreated () noexcept
{
    if ( !_renderer.OnCreateSwapchain ( *_nativeWindow, false ) )
        return false;

    if ( !_game->OnSwapchainCreated ( _renderer ) )
        return false;

    _fpsTimestamp = std::chrono::system_clock::now ();
    _frameTimestamp = _fpsTimestamp;
    _renderBodyHandler = &Core::OnFrame;

    return true;
}

bool Core::OnSwapchainDestroyed () noexcept
{
    _renderBodyHandler = &Core::OnIdle;

    if ( !_renderer.FinishAllJobs () )
        return false;

    _game->OnSwapchainDestroyed ( _renderer.GetDevice () );
    _renderer.OnDestroySwapchain ();

    return true;
}

void Core::UpdateFPS ( Timestamp now )
{
    static uint32_t frameCount = 0U;
    ++frameCount;

    const std::chrono::duration<double> seconds = now - _fpsTimestamp;
    const double delta = seconds.count ();

    if ( delta < FPS_PERIOD )
        return;

    LogInfo ( "FPS: %g", frameCount / delta );
    _fpsTimestamp = now;
    frameCount = 0U;
}

extern "C" {

JNIEXPORT void Java_com_goshidoInc_androidVulkan_Activity_doCreate ( JNIEnv *env, jobject /*obj*/,
    jobject assetManager )
{
    g_Core = new Core ( env, assetManager );
    LogDebug ( "~~~ OnCreate" );
}

JNIEXPORT void Java_com_goshidoInc_androidVulkan_Activity_doDestroy ( JNIEnv *env, jobject /*obj*/ )
{
    g_Core->OnAboutDestroy ( env );

    delete g_Core;
    g_Core = nullptr;

    LogDebug ( "~~~ OnDestroy" );
}

JNIEXPORT void Java_com_goshidoInc_androidVulkan_Activity_doKeyDown ( JNIEnv */*env*/, jobject /*obj*/, jint keyCode )
{
    // TODO
    LogDebug ( "~~~ OnKeyDown %d", keyCode );
}

JNIEXPORT void Java_com_goshidoInc_androidVulkan_Activity_doKeyUp ( JNIEnv */*env*/, jobject /*obj*/, jint keyCode )
{
    // TODO
    LogDebug ( "~~~ OnKeyUp %d", keyCode );
}

JNIEXPORT void Java_com_goshidoInc_androidVulkan_Activity_doLeftStick ( JNIEnv */*env*/,
    jobject /*obj*/,
    jfloat x,
    jfloat y
)
{
    // TODO
    LogDebug ( "~~~ OnLeftStick %g %g", x, y );
}

JNIEXPORT void Java_com_goshidoInc_androidVulkan_Activity_doRightStick ( JNIEnv */*env*/,
    jobject /*obj*/,
    jfloat x,
    jfloat y
)
{
    // TODO
    LogDebug ( "~~~ OnRightStick %g %g", x, y );
}

JNIEXPORT void Java_com_goshidoInc_androidVulkan_Activity_doLeftTrigger ( JNIEnv */*env*/,
    jobject /*obj*/,
    jfloat value
)
{
    // TODO
    LogDebug ( "~~~ OnLeftTrigger %g", value );
}

JNIEXPORT void Java_com_goshidoInc_androidVulkan_Activity_doRightTrigger ( JNIEnv */*env*/,
    jobject /*obj*/,
    jfloat value
)
{
    // TODO
    LogDebug ( "~~~ OnRightTrigger %g", value );
}

JNIEXPORT void Java_com_goshidoInc_androidVulkan_Activity_doSurfaceCreated ( JNIEnv* env, jobject /*obj*/, jobject surface )
{
    g_Core->OnSurfaceCreated ( env, surface );
    LogDebug ( "~~~ OnSurfaceCreated" );
}

JNIEXPORT void Java_com_goshidoInc_androidVulkan_Activity_doSurfaceDestroyed ( JNIEnv* /*env*/, jobject /*obj*/ )
{
    g_Core->OnSurfaceDestroyed ();
    LogDebug ( "~~~ OnSurfaceDestroyed" );
}

} // extern "C"

} // namespace android_vulkan
