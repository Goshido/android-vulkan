#ifndef ANDROID_VULKAN_CORE_H
#define ANDROID_VULKAN_CORE_H


#include <chrono>
#include <android_native_app_glue.h>
#include "game.h"


namespace android_vulkan {

class Core final
{
    using timestamp = std::chrono::time_point<std::chrono::system_clock>;

    private:
        Game&           _game;

        Renderer        _renderer;
        timestamp       _fpsTimestamp;
        timestamp       _frameTimestamp;

    public:
        explicit Core ( android_app &app, Game &game );
        ~Core () = default;

        Core ( const Core &other ) = delete;
        Core& operator = ( const Core &other ) = delete;

        bool IsSuspend () const;
        void OnFrame ();

    private:
        void ActivateFullScreen ( android_app &app );
        void UpdateFPS ( timestamp now );

        static void OnOSCommand ( android_app* app, int32_t cmd );
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_CORE_H