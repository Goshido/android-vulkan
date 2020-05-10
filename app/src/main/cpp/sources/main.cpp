#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <android_native_app_glue.h>

GX_RESTORE_WARNING_STATE

#include <core.h>
#include <logger.h>
#include <rotating_mesh/game.h>


void android_main ( android_app* app )
{

#ifdef ANDROID_VULKAN_DEBUG

    android_vulkan::LogDebug ( "android_main - Application was started." );

#endif // ANDROID_VULKAN_DEBUG

    rotating_mesh::Game game;
    android_vulkan::Core core ( *app, game );

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
