#include <core.h>
#include <logger.h>


namespace android_vulkan {

AAssetManager* g_AssetManager = nullptr;
constexpr static double const FPS_PERIOD = 3.0;

Core::Core ( android_app &app, Game &game ) noexcept:
    _game ( game ),
    _gamepad ( Gamepad::GetInstance () ),
    _renderer {},
    _fpsTimestamp {},
    _frameTimestamp {}
{
    // grab asset manager
    g_AssetManager = app.activity->assetManager;

    app.onAppCmd = &Core::OnOSCommand;
    app.onInputEvent = &Core::OnOSInputEvent;
    app.userData = this;

    ActivateFullScreen ( app );
}

bool Core::IsSuspend () const
{
    return !_renderer.IsSwapchainCreated ();
}

void Core::OnFrame ()
{
    if ( !_game.IsReady () )
        return;

    timestamp const now = std::chrono::system_clock::now ();
    std::chrono::duration<double> const delta = now - _frameTimestamp;

    if ( _renderer.CheckSwapchainStatus () && !_game.OnFrame ( _renderer, delta.count () ) )
        LogError ( "Core::OnFrame - Frame rendering failed." );

    _frameTimestamp = now;
    UpdateFPS ( now );
}

void Core::OnQuit ()
{
    _game.OnDestroyDevice ( _renderer.GetDevice () );
    _renderer.OnDestroyDevice ();
}

void Core::OnInitWindow ( ANativeWindow &window )
{
    if ( !_renderer.IsDeviceCreated () )
    {
        if ( !_renderer.OnCreateDevice () )
            return;

        if ( !_game.OnInitDevice ( _renderer ) )
        {
            _renderer.OnDestroyDevice ();
            return;
        }
    }

    if ( !_renderer.OnCreateSwapchain ( window, false ) )
    {
        _game.OnDestroyDevice ( _renderer.GetDevice () );
        _renderer.OnDestroyDevice ();
        return;
    }

    if ( !_game.OnSwapchainCreated ( _renderer ) )
    {
        _renderer.OnDestroySwapchain ();
        _game.OnDestroyDevice ( _renderer.GetDevice () );
        _renderer.OnDestroyDevice ();
    }

    _fpsTimestamp = std::chrono::system_clock::now ();
    _frameTimestamp = _fpsTimestamp;

    _gamepad.Start ();
}

void Core::OnLowMemory ()
{
    // TODO
    LogWarning ( "Core::OnLowMemory - APP_CMD_LOW_MEMORY has been received [app %p].", this );
}

void Core::OnTerminateWindow ()
{
    if ( !_renderer.IsSwapchainCreated () )
        return;

    if ( !_renderer.FinishAllJobs () )
        LogError ( "Core::OnTerminateWindow - Can't finish all GPU jobs" );

    _gamepad.Stop ();
    _game.OnSwapchainDestroyed ( _renderer.GetDevice () );
    _renderer.OnDestroySwapchain ();
}

void Core::UpdateFPS ( timestamp now )
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

void Core::ActivateFullScreen ( android_app &app )
{
    // Based on https://stackoverflow.com/a/50831255

    JNIEnv* env = nullptr;
    app.activity->vm->AttachCurrentThread ( &env, nullptr );

    jclass activityClass = env->FindClass ( "android/app/NativeActivity" );
    jmethodID getWindow = env->GetMethodID ( activityClass, "getWindow", "()Landroid/view/Window;" );

    jclass windowClass = env->FindClass ( "android/view/Window" );
    jmethodID getDecorView = env->GetMethodID ( windowClass, "getDecorView", "()Landroid/view/View;" );

    jclass viewClass = env->FindClass ( "android/view/View" );
    jmethodID setSystemUiVisibility = env->GetMethodID ( viewClass, "setSystemUiVisibility", "(I)V" );

    jobject window = env->CallObjectMethod ( app.activity->clazz, getWindow );

    jobject decorView = env->CallObjectMethod ( window, getDecorView );

    jfieldID flagFullscreenID = env->GetStaticFieldID ( viewClass, "SYSTEM_UI_FLAG_FULLSCREEN", "I" );
    jfieldID flagHideNavigationID = env->GetStaticFieldID ( viewClass, "SYSTEM_UI_FLAG_HIDE_NAVIGATION", "I" );
    jfieldID flagImmersiveStickyID = env->GetStaticFieldID ( viewClass, "SYSTEM_UI_FLAG_IMMERSIVE_STICKY", "I" );

    const int flagFullscreen = env->GetStaticIntField ( viewClass, flagFullscreenID );
    const int flagHideNavigation = env->GetStaticIntField ( viewClass, flagHideNavigationID );
    const int flagImmersiveSticky = env->GetStaticIntField ( viewClass, flagImmersiveStickyID );

    const int flag = static_cast<int> (
        static_cast<uint> ( flagFullscreen ) |
        static_cast<uint> ( flagHideNavigation ) |
        static_cast<uint> ( flagImmersiveSticky )
    );

    env->CallVoidMethod ( decorView, setSystemUiVisibility, flag );
    app.activity->vm->DetachCurrentThread ();
}

void Core::OnOSCommand ( android_app* app, int32_t cmd )
{
    auto& core = *static_cast<Core*> ( app->userData );

    switch ( cmd )
    {
        case APP_CMD_INIT_WINDOW:
            core.OnInitWindow ( *app->window );
        break;

        case APP_CMD_TERM_WINDOW:
            core.OnTerminateWindow ();
        break;

        case APP_CMD_DESTROY:
            app->onAppCmd = nullptr;
            app->userData = nullptr;
        break;

        case APP_CMD_LOW_MEMORY:
            core.OnLowMemory ();
        break;

        default:
            // NOTHING
        break;
    }
}

int32_t Core::OnOSInputEvent ( android_app* app, AInputEvent *event )
{
    auto& core = *static_cast<Core*> ( app->userData );
    return core._gamepad.OnOSInputEvent ( event );
}

} // namespace android_vulkan
