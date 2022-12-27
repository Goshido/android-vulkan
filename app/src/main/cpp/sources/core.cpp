#include <core.h>
#include <bitwise.h>
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

#include <cassert>
#include <android/asset_manager_jni.h>
#include <android/native_window_jni.h>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

constexpr static double FPS_PERIOD = 3.0;
constexpr static auto TIMEOUT = std::chrono::milliseconds ( 10U );

AAssetManager* g_AssetManager = nullptr;
static Core* g_Core = nullptr;

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

Core::Core ( JNIEnv* env, jobject activity, jobject assetManager, std::string &&cacheDirectory ) noexcept:
    _cacheDirectory ( std::move ( cacheDirectory ) )
{
    env->GetJavaVM ( &_vm );

    _assetManager = env->NewGlobalRef ( assetManager );
    g_AssetManager = AAssetManager_fromJava ( env, assetManager );

    _activity = env->NewGlobalRef ( activity );
    _finishMethod = env->GetMethodID ( env->FindClass ( "com/goshidoInc/androidVulkan/Activity" ), "finish", "()V" );

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
            if ( !_renderer.OnCreateDevice () || !_game->OnInitDevice ( _renderer ) )
                return;

            while ( ExecuteMessageQueue () )
            {
                // C++ calling method by pointer syntax.
                ( this->*_rendererBodyHandler ) ();
            }
        }
    );

    _gamepad.BindKey ( nullptr, &Core::OnHomeUp, eGamepadKey::Home, eButtonState::Up );
}

void Core::OnAboutDestroy ( JNIEnv* env ) noexcept
{
    {
        std::unique_lock<std::mutex> const lock ( _mutex );
        _writeQueue.push_back ( eCommand::Quit );
    }

    if ( _thread.joinable () )
        _thread.join ();

    env->DeleteGlobalRef ( _assetManager );
    _assetManager = nullptr;
    g_AssetManager = nullptr;

    env->DeleteGlobalRef ( _activity );
    _activity = nullptr;

    _finishMethod = nullptr;
    _vm = nullptr;
}

void Core::OnKeyDown ( int32_t key ) const noexcept
{
    _gamepad.OnKeyDown ( key );
}

void Core::OnKeyUp ( int32_t key ) const noexcept
{
    _gamepad.OnKeyUp ( key );
}

void Core::OnLeftStick ( float x, float y ) const noexcept
{
    _gamepad.OnLeftStick ( x, y );
}

void Core::OnRightStick ( float x, float y ) const noexcept
{
    _gamepad.OnRightStick ( x, y );
}

void Core::OnLeftTrigger ( float value ) const noexcept
{
    _gamepad.OnLeftTrigger ( value );
}

void Core::OnRightTrigger ( float value ) const noexcept
{
    _gamepad.OnRightTrigger ( value );
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

    while ( AV_BITWISE ( !_readQueue.empty () ) | AV_BITWISE ( !_writeQueue.empty () ) )
        std::this_thread::sleep_for ( TIMEOUT );

    ANativeWindow_release ( _nativeWindow );
    _nativeWindow = nullptr;
}

std::string const& Core::GetCacheDirectory () noexcept
{
    assert ( g_Core );
    static std::string const nullDirectory = "fuck!";
    return g_Core ? g_Core->_cacheDirectory : nullDirectory;
}

void Core::Quit () noexcept
{
    if ( !g_Core )
        return;

    std::unique_lock<std::mutex> const lock ( g_Core->_mutex );
    g_Core->_writeQueue.push_back ( eCommand::QuitRequest );
}

bool Core::ExecuteMessageQueue () noexcept
{
    {
        std::unique_lock<std::mutex> const lock ( _mutex );
        _readQueue.swap ( _writeQueue );
    }

    bool result = true;

    for ( auto const command : _readQueue )
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
    _commandHandlers[ static_cast<size_t> ( eCommand::Quit ) ] = &Core::OnQuit;
    _commandHandlers[ static_cast<size_t> ( eCommand::QuitRequest ) ] = &Core::OnQuitRequest;
    _commandHandlers[ static_cast<size_t> ( eCommand::SwapchainCreated ) ] = &Core::OnSwapchainCreated;
    _commandHandlers[ static_cast<size_t> ( eCommand::SwapchainDestroyed ) ] = &Core::OnSwapchainDestroyed;
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
    _game->OnDestroyDevice ( _renderer );
    _renderer.OnDestroyDevice ();
    return false;
}

bool Core::OnQuitRequest () noexcept
{
    JNIEnv* env = nullptr;

    JavaVMAttachArgs args
    {
        .version = JNI_VERSION_1_6,
        .name = "Native Core",
        .group = nullptr
    };

    _vm->AttachCurrentThread ( &env, &args );
    env->CallVoidMethod ( _activity, _finishMethod );
    _vm->DetachCurrentThread ();

    return true;
}

bool Core::OnSwapchainCreated () noexcept
{
    if ( !_renderer.OnCreateSwapchain ( *_nativeWindow, false ) || !_game->OnSwapchainCreated ( _renderer ) )
        return false;

    _fpsTimestamp = std::chrono::system_clock::now ();
    _frameTimestamp = _fpsTimestamp;
    _rendererBodyHandler = &Core::OnFrame;

    return true;
}

bool Core::OnSwapchainDestroyed () noexcept
{
    _rendererBodyHandler = &Core::OnIdle;

    if ( !_renderer.FinishAllJobs () )
        return false;

    _game->OnSwapchainDestroyed ( _renderer );
    _renderer.OnDestroySwapchain ();

    return true;
}

void Core::UpdateFPS ( Timestamp now )
{
    ++_frameCount;

    const std::chrono::duration<double> seconds = now - _fpsTimestamp;
    const double delta = seconds.count ();

    if ( delta < FPS_PERIOD )
        return;

    LogInfo ( "FPS: %g", _frameCount / delta );
    _fpsTimestamp = now;
    _frameCount = 0U;
}

void Core::OnHomeUp ( void* /*context*/ ) noexcept
{
    Core::Quit ();
}

extern "C" {

JNIEXPORT void Java_com_goshidoInc_androidVulkan_Activity_doCreate ( JNIEnv* env,
    jobject obj,
    jobject assetManager,
    jstring cacheDirectory
)
{
    char const* utf8 = env->GetStringUTFChars ( cacheDirectory, nullptr );
    g_Core = new Core ( env, obj, assetManager, utf8 );
    env->ReleaseStringUTFChars ( cacheDirectory, utf8 );
    LogInfo ( "Core has been created." );
}

JNIEXPORT void Java_com_goshidoInc_androidVulkan_Activity_doDestroy ( JNIEnv* env, jobject /*obj*/ )
{
    if ( !g_Core )
        return;

    g_Core->OnAboutDestroy ( env );

    delete g_Core;
    g_Core = nullptr;
    LogInfo ( "Core has been destroyed." );
}

JNIEXPORT void Java_com_goshidoInc_androidVulkan_Activity_doKeyDown ( JNIEnv* /*env*/, jobject /*obj*/, jint keyCode )
{
    if ( !g_Core )
        return;

    g_Core->OnKeyDown ( keyCode );
}

JNIEXPORT void Java_com_goshidoInc_androidVulkan_Activity_doKeyUp ( JNIEnv* /*env*/, jobject /*obj*/, jint keyCode )
{
    if ( !g_Core )
        return;

    g_Core->OnKeyUp ( keyCode );
}

JNIEXPORT void Java_com_goshidoInc_androidVulkan_Activity_doLeftStick ( JNIEnv* /*env*/,
    jobject /*obj*/,
    jfloat x,
    jfloat y
)
{
    if ( !g_Core )
        return;

    g_Core->OnLeftStick ( x, y );
}

JNIEXPORT void Java_com_goshidoInc_androidVulkan_Activity_doRightStick ( JNIEnv* /*env*/,
    jobject /*obj*/,
    jfloat x,
    jfloat y
)
{
    if ( !g_Core )
        return;

    g_Core->OnRightStick ( x, y );
}

JNIEXPORT void Java_com_goshidoInc_androidVulkan_Activity_doLeftTrigger ( JNIEnv* /*env*/,
    jobject /*obj*/,
    jfloat value
)
{
    if ( !g_Core )
        return;

    g_Core->OnLeftTrigger ( value );
}

JNIEXPORT void Java_com_goshidoInc_androidVulkan_Activity_doRightTrigger ( JNIEnv* /*env*/,
    jobject /*obj*/,
    jfloat value
)
{
    if ( !g_Core )
        return;

    g_Core->OnRightTrigger ( value );
}

JNIEXPORT void Java_com_goshidoInc_androidVulkan_Activity_doSurfaceCreated ( JNIEnv* env,
    jobject /*obj*/,
    jobject surface
)
{
    if ( !g_Core )
        return;

    g_Core->OnSurfaceCreated ( env, surface );
}

JNIEXPORT void Java_com_goshidoInc_androidVulkan_Activity_doSurfaceDestroyed ( JNIEnv* /*env*/, jobject /*obj*/ )
{
    if ( !g_Core )
        return;

    g_Core->OnSurfaceDestroyed ();
}

} // extern "C"

} // namespace android_vulkan
