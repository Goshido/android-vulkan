#ifndef ANDROID_VULKAN_CORE_H
#define ANDROID_VULKAN_CORE_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <chrono>
#include <android_native_app_glue.h>

GX_RESTORE_WARNING_STATE

#include "game.h"
#include "gamepad.h"


namespace android_vulkan {

class Core final
{
    using timestamp = std::chrono::time_point<std::chrono::system_clock>;

    private:
        Game&           _game;
        Gamepad&        _gamepad;

        Renderer        _renderer;
        timestamp       _fpsTimestamp;
        timestamp       _frameTimestamp;

    public:
        explicit Core ( android_app &app, Game &game ) noexcept;

        Core ( Core const & ) = delete;
        Core& operator = ( Core const & ) = delete;

        Core ( Core && ) = delete;
        Core& operator = ( Core && ) = delete;

        ~Core () = default;

        [[nodiscard]] bool IsSuspend () const;
        void OnFrame ();
        void OnQuit ();

    private:
        void OnInitWindow ( ANativeWindow &window );
        void OnLowMemory ();
        void OnTerminateWindow ();
        void UpdateFPS ( timestamp now );

        static void ActivateFullScreen ( android_app &app );
        static void OnOSCommand ( android_app* app, int32_t cmd );
        [[nodiscard]] static int32_t OnOSInputEvent ( android_app* app, AInputEvent *event );
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_CORE_H
