#ifndef ANDROID_VULKAN_CORE_H
#define ANDROID_VULKAN_CORE_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <chrono>
#include <thread>
#include <android/asset_manager.h>
#include <android/native_window.h>

GX_RESTORE_WARNING_STATE

#include "game.h"
#include "gamepad.h"


namespace android_vulkan {

class Core final
{
    private:
        using Timestamp = std::chrono::time_point<std::chrono::system_clock>;
        using CommandHandler = bool ( Core::* ) () noexcept;
        using RendererBodyHandler = void ( Core::* ) () noexcept;

        enum class eCommand : uint8_t
        {
            SwapchainCreated = 0U,
            SwapchainDestroyed,
            Quit,
            COUNT
        };

    private:
        Game*                           _game = nullptr;
        [[maybe_unused]] Gamepad&       _gamepad = Gamepad::GetInstance ();

        Renderer                        _renderer {};
        Timestamp                       _fpsTimestamp {};
        Timestamp                       _frameTimestamp {};

        jobject                         _assetManagerJVM = nullptr;
        ANativeWindow*                  _nativeWindow = nullptr;

        std::vector<eCommand>           _readQueue {};
        std::vector<eCommand>           _writeQueue {};

        CommandHandler                  _commandHandlers[ static_cast<size_t>(eCommand::COUNT) ] {};
        RendererBodyHandler             _rendererBodyHandler = &Core::OnIdle;

        std::thread                     _thread {};
        std::mutex                      _mutex {};

    public:
        Core () = delete;

        Core ( Core const & ) = delete;
        Core& operator = ( Core const & ) = delete;

        Core ( Core && ) = delete;
        Core& operator = ( Core && ) = delete;

        explicit Core ( JNIEnv* env, jobject assetManager ) noexcept;

        ~Core () = default;

        void OnAboutDestroy ( JNIEnv* env ) noexcept;
        void OnSurfaceCreated ( JNIEnv* env, jobject surface ) noexcept;
        void OnSurfaceDestroyed () noexcept;

    private:
        [[nodiscard]] bool ExecuteMessageQueue () noexcept;
        void InitCommandHandlers () noexcept;

        void OnFrame () noexcept;
        void OnIdle () noexcept;

        bool OnQuit () noexcept;
        bool OnSwapchainCreated () noexcept;
        bool OnSwapchainDestroyed () noexcept;

        void UpdateFPS ( Timestamp now );
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_CORE_H
