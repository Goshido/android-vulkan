#include <core.h>

GX_DISABLE_COMMON_WARNINGS

#include <map>

GX_RESTORE_WARNING_STATE

#include "vulkan_utils.h"


namespace android_vulkan {

AAssetManager* g_AssetManager = nullptr;
constexpr static const double FPS_PERIOD = 3.0;

Core::Core ( android_app &app, Game &game ):
    _game ( game )
{
    // grab asset manager
    g_AssetManager = app.activity->assetManager;

    app.onAppCmd = &Core::OnOSCommand;
    app.userData = this;
    ActivateFullScreen ( app );
}

bool Core::IsSuspend () const
{
    return !_renderer.IsReady ();
}

void Core::OnFrame ()
{
    if ( !_game.IsReady () )
        return;

    const timestamp now = std::chrono::system_clock::now ();
    const std::chrono::duration<double> delta = now - _frameTimestamp;

    if ( _renderer.CheckSwapchainStatus () )
        _game.OnFrame ( _renderer, delta.count () );

    _frameTimestamp = now;
    UpdateFPS ( now );
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
            if ( core._renderer.OnInit ( *app->window, false ) )
                core._game.OnInit ( core._renderer );

            core._fpsTimestamp = std::chrono::system_clock::now ();
            core._frameTimestamp = core._fpsTimestamp;
        break;

        case APP_CMD_TERM_WINDOW:
            core._game.OnDestroy ( core._renderer );
            core._renderer.OnDestroy ();
            AV_CHECK_VULKAN_LEAKS ()
        break;

        case APP_CMD_DESTROY:
            app->onAppCmd = nullptr;
            app->userData = nullptr;
        break;

        default:
            // NOTHING
        break;
    }
}

} // namespace android_vulkan
