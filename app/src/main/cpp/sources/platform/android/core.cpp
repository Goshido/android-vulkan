#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <bitwise.hpp>
#include <logger.hpp>
#include <mandelbrot/mandelbrot_analytic_color.hpp>
#include <mandelbrot/mandelbrot_lut_color.hpp>
#include <platform/android/core.hpp>
#include <pbr/pbr_game.hpp>
#include <pbr/box_stack/box_stack.hpp>
#include <pbr/collision/collision.hpp>
#include <pbr/ray_casting/ray_casting.hpp>
#include <pbr/stipple_test/stipple_test.hpp>
#include <pbr/sweep_testing/sweep_testing.hpp>
#include <pbr/universal_game.hpp>
#include <rainbow/rainbow.hpp>
#include <rotating_mesh/game_analytic.hpp>
#include <rotating_mesh/game_lut.hpp>


namespace android_vulkan::android {

namespace {

constexpr double FPS_PERIOD = 3.0;
constexpr auto TIMEOUT = std::chrono::milliseconds ( 10U );
constexpr bool VSYNC = true;

enum class eGame : uint16_t
{
    CharacterSandbox,
    Collision,
    BoxStack,
    MandelbrotAnalyticColor,
    MandelbrotLutColor,
    PBR,
    Rainbow,
    RayCasting,
    RotatingMeshAnalytic,
    RotatingMeshLUT,
    SkeletalMeshSandbox,
    StippleTest,
    SweepTesting,
    World1x1
};

Core* g_Core = nullptr;

} // end of anonymous namespace

AAssetManager* g_AssetManager = nullptr;

//----------------------------------------------------------------------------------------------------------------------

Core::Core ( JNIEnv* env, jobject activity, jobject assetManager, std::string &&cacheDirectory, float dpi ) noexcept:
    _cacheDirectory ( std::move ( cacheDirectory ) )
{
    // It's needed for various parsers from string data. For example for float parsers.
    std::locale::global ( std::locale::classic () );

    env->GetJavaVM ( &_vm );

    _assetManager = env->NewGlobalRef ( assetManager );
    g_AssetManager = AAssetManager_fromJava ( env, assetManager );

    _activity = env->NewGlobalRef ( activity );
    _finishMethod = env->GetMethodID ( env->FindClass ( "com/goshidoInc/androidVulkan/Activity" ), "finish", "()V" );

    InitCommandHandlers ();

    static std::map<android_vulkan::android::eGame, std::shared_ptr<android_vulkan::Game>> const games =
    {
        { eGame::CharacterSandbox, std::make_shared<pbr::UniversalGame> ( "pbr/assets/character-sandbox.scene" ) },
        { eGame::Collision, std::make_shared<pbr::collision::Collision> () },
        { eGame::BoxStack, std::make_shared<pbr::box_stack::BoxStack> () },
        { eGame::MandelbrotAnalyticColor, std::make_shared<mandelbrot::MandelbrotAnalyticColor> () },
        { eGame::MandelbrotLutColor, std::make_shared<mandelbrot::MandelbrotLUTColor> () },
        { eGame::PBR, std::make_shared<pbr::PBRGame> () },
        { eGame::Rainbow, std::make_shared<rainbow::Rainbow> () },
        { eGame::RayCasting, std::make_shared<pbr::ray_casting::RayCasting> () },
        { eGame::RotatingMeshAnalytic, std::make_shared<rotating_mesh::GameAnalytic> () },
        { eGame::RotatingMeshLUT, std::make_shared<rotating_mesh::GameLUT> () },

        {
            eGame::SkeletalMeshSandbox,
            std::make_shared<pbr::UniversalGame> ( "pbr/assets/skeletal-mesh-sandbox.scene" )
        },

        { eGame::StippleTest, std::make_shared<pbr::stipple_test::StippleTest> () },
        { eGame::SweepTesting, std::make_shared<pbr::sweep_testing::SweepTesting> () },

        { eGame::World1x1, std::make_shared<pbr::UniversalGame> ( "pbr/assets/world-1-1.scene" ) }
    };

    _game = games.find ( eGame::PBR )->second.get ();

    _thread = std::thread (
        [ this, dpi ] () noexcept {
            if ( !_game->OnInitSoundSystem () ) [[unlikely]]
            {
                _game->OnDestroySoundSystem ();
                return;
            }

            if ( !_renderer.OnCreateDevice ( {} ) ) [[unlikely]]
            {
                _renderer.OnDestroyDevice ();
                _game->OnDestroySoundSystem ();
                return;
            }

            _renderer.OnSetDPI ( dpi );

            if ( !_game->OnInitDevice ( _renderer ) ) [[unlikely]]
            {
                _game->OnDestroyDevice ( _renderer );
                _renderer.OnDestroyDevice ();
                _game->OnDestroySoundSystem ();
                return;
            }

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
        std::lock_guard const lock ( _mutex );
        _writeQueue.push_back ( eCommand::Quit );
    }

    if ( _thread.joinable () ) [[likely]]
        _thread.join ();

    env->DeleteGlobalRef ( _assetManager );
    _assetManager = nullptr;
    g_AssetManager = nullptr;

    env->DeleteGlobalRef ( _activity );
    _activity = nullptr;

    _finishMethod = nullptr;
    _vm = nullptr;
}

bool Core::OnKeyDown ( int32_t key ) const noexcept
{
    return _gamepad.OnKeyDown ( key );
}

bool Core::OnKeyUp ( int32_t key ) const noexcept
{
    return _gamepad.OnKeyUp ( key );
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
    std::lock_guard const lock ( _mutex );
    _writeQueue.push_back ( eCommand::SwapchainCreated );
}

void Core::OnSurfaceDestroyed () noexcept
{
    {
        std::lock_guard const lock ( _mutex );
        _writeQueue.push_back ( eCommand::SwapchainDestroyed );
    }

    while ( AV_BITWISE ( !_readQueue.empty () ) | AV_BITWISE ( !_writeQueue.empty () ) )
        std::this_thread::sleep_for ( TIMEOUT );

    ANativeWindow_release ( _nativeWindow );
    _nativeWindow = nullptr;
}

std::string const &Core::GetCacheDirectory () noexcept
{
    AV_ASSERT ( g_Core )
    static std::string const nullDirectory = "fuck!";
    return g_Core ? g_Core->_cacheDirectory : nullDirectory;
}

void Core::Quit () noexcept
{
    if ( !g_Core ) [[unlikely]]
        return;

    std::lock_guard const lock ( g_Core->_mutex );
    g_Core->_writeQueue.push_back ( eCommand::QuitRequest );
}

bool Core::ExecuteMessageQueue () noexcept
{
    {
        std::lock_guard const lock ( _mutex );
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

    if ( _renderer.CheckSwapchainStatus () && !_game->OnFrame ( _renderer, delta.count () ) ) [[unlikely]]
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
    _game->OnDestroySoundSystem ();

    JavaVMAttachArgs args
    {
        .version = JNI_VERSION_1_6,
        .name = "Native Core",
        .group = nullptr
    };

    JNIEnv* env = nullptr;
    _vm->AttachCurrentThread ( &env, &args );
    env->CallVoidMethod ( _activity, _finishMethod );
    _vm->DetachCurrentThread ();

    return true;
}

bool Core::OnSwapchainCreated () noexcept
{
    Renderer::eSwapchainResult const result = _renderer.OnCreateSwapchain ( false,
        reinterpret_cast<WindowHandle> ( _nativeWindow ),
        VSYNC
    );

    if ( result != Renderer::eSwapchainResult::Success ) [[unlikely]]
    {
        _renderer.OnDestroySwapchain ( false );
        return false;
    }

    if ( !_game->OnSwapchainCreated ( _renderer ) ) [[unlikely]]
    {
        _game->OnSwapchainDestroyed ( _renderer );
        _renderer.OnDestroySwapchain ( false );
        return false;
    }

    _fpsTimestamp = std::chrono::system_clock::now ();
    _frameTimestamp = _fpsTimestamp;
    _rendererBodyHandler = &Core::OnFrame;

    return true;
}

bool Core::OnSwapchainDestroyed () noexcept
{
    _rendererBodyHandler = &Core::OnIdle;

    if ( !_renderer.FinishAllJobs () ) [[unlikely]]
        return false;

    _game->OnSwapchainDestroyed ( _renderer );
    _renderer.OnDestroySwapchain ( false );

    return true;
}

void Core::UpdateFPS ( Timestamp now )
{
    ++_frameCount;

    std::chrono::duration<double> const seconds = now - _fpsTimestamp;
    double const delta = seconds.count ();

    if ( delta < FPS_PERIOD ) [[likely]]
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
    jstring cacheDirectory,
    jfloat dpi
)
{
    char const* utf8 = env->GetStringUTFChars ( cacheDirectory, nullptr );
    g_Core = new Core ( env, obj, assetManager, utf8, dpi );
    env->ReleaseStringUTFChars ( cacheDirectory, utf8 );
    LogInfo ( "Core has been created." );
}

JNIEXPORT void Java_com_goshidoInc_androidVulkan_Activity_doDestroy ( JNIEnv* env, jobject /*obj*/ )
{
    if ( !g_Core ) [[unlikely]]
        return;

    g_Core->OnAboutDestroy ( env );

    delete g_Core;
    g_Core = nullptr;
    LogInfo ( "Core has been destroyed." );
}

JNIEXPORT jboolean Java_com_goshidoInc_androidVulkan_Activity_doKeyDown ( JNIEnv* /*env*/, jobject /*obj*/,
    jint keyCode )
{
    if ( !g_Core ) [[unlikely]]
        return JNI_FALSE;

    constexpr jboolean const cases[] = { JNI_FALSE, JNI_TRUE };
    return cases[ static_cast<size_t> ( g_Core->OnKeyDown ( keyCode ) ) ];
}

JNIEXPORT jboolean Java_com_goshidoInc_androidVulkan_Activity_doKeyUp ( JNIEnv* /*env*/, jobject /*obj*/, jint keyCode )
{
    if ( !g_Core ) [[unlikely]]
        return JNI_FALSE;

    constexpr jboolean const cases[] = { JNI_FALSE, JNI_TRUE };
    return cases[ static_cast<size_t> ( g_Core->OnKeyUp ( keyCode ) ) ];
}

JNIEXPORT void Java_com_goshidoInc_androidVulkan_Activity_doLeftStick ( JNIEnv* /*env*/,
    jobject /*obj*/,
    jfloat x,
    jfloat y
)
{
    if ( !g_Core ) [[unlikely]]
        return;

    g_Core->OnLeftStick ( x, y );
}

JNIEXPORT void Java_com_goshidoInc_androidVulkan_Activity_doRightStick ( JNIEnv* /*env*/,
    jobject /*obj*/,
    jfloat x,
    jfloat y
)
{
    if ( !g_Core ) [[unlikely]]
        return;

    g_Core->OnRightStick ( x, y );
}

JNIEXPORT void Java_com_goshidoInc_androidVulkan_Activity_doLeftTrigger ( JNIEnv* /*env*/,
    jobject /*obj*/,
    jfloat value
)
{
    if ( !g_Core ) [[unlikely]]
        return;

    g_Core->OnLeftTrigger ( value );
}

JNIEXPORT void Java_com_goshidoInc_androidVulkan_Activity_doRightTrigger ( JNIEnv* /*env*/,
    jobject /*obj*/,
    jfloat value
)
{
    if ( !g_Core ) [[unlikely]]
        return;

    g_Core->OnRightTrigger ( value );
}

JNIEXPORT void Java_com_goshidoInc_androidVulkan_Activity_doSurfaceCreated ( JNIEnv* env,
    jobject /*obj*/,
    jobject surface
)
{
    if ( !g_Core ) [[unlikely]]
        return;

    g_Core->OnSurfaceCreated ( env, surface );
}

JNIEXPORT void Java_com_goshidoInc_androidVulkan_Activity_doSurfaceDestroyed ( JNIEnv* /*env*/, jobject /*obj*/ )
{
    if ( !g_Core ) [[unlikely]]
        return;

    g_Core->OnSurfaceDestroyed ();
}

} // extern "C"

} // namespace android_vulkan::android
