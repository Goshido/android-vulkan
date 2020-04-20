#include <android_native_app_glue.h>
#include <core.h>
#include <logger.h>
#include <mandelbrot/mandelbrot_analytic_color.h>


void android_main ( android_app* app )
{

#ifdef ANDROID_VULKAN_DEBUG

    android_vulkan::LogDebug ( "android_main - Application was started." );

#endif // ANDROID_VULKAN_DEBUG

    mandelbrot::MandelbrotAnalyticColor mandelbrotAnalyticColorGame;
    android_vulkan::Core core ( *app, mandelbrotAnalyticColorGame );

    for ( ; ; )
    {
        for ( ; ; )
        {
            int events;
            android_poll_source* source;

            const int pollResult = ALooper_pollAll ( core.IsSuspend () ? 0 : -1,
                nullptr,
                &events,
                reinterpret_cast<void**> ( &source )
            );

            if ( pollResult < 0 || !source ) break;

            source->process ( app, source );
        }

        if ( app->destroyRequested ) break;

        core.OnFrame ();
    }

#ifdef ANDROID_VULKAN_DEBUG

    android_vulkan::LogDebug ( "android_main - Application was finished." );

#endif // ANDROID_VULKAN_DEBUG

}
