#ifndef ANDROID_VULKAN_ANDROID_CORE_HPP
#define ANDROID_VULKAN_ANDROID_CORE_HPP


#include "game.hpp"
#include "gamepad.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <chrono>
#include <thread>
#include <jni.h>
#include <android/asset_manager.h>
#include <android/native_window.h>

GX_RESTORE_WARNING_STATE


namespace android_vulkan::android {

class Core final
{
    private:
        using Timestamp = std::chrono::time_point<std::chrono::steady_clock>;
        using CommandHandler = bool ( Core::* ) () noexcept;
        using RendererBodyHandler = void ( Core::* ) () noexcept;

        enum class eCommand : uint8_t
        {
            Quit = 0U,
            QuitRequest,
            SwapchainCreated,
            SwapchainDestroyed,
            COUNT
        };

    private:
        std::string const           _cacheDirectory {};

        Game*                       _game = nullptr;
        Gamepad                     &_gamepad = Gamepad::GetInstance ();

        Renderer                    _renderer {};
        Timestamp                   _fpsTimestamp {};
        uint32_t                    _frameCount = 0U;
        Timestamp                   _frameTimestamp {};

        jobject                     _activity = nullptr;
        jobject                     _assetManager = nullptr;
        jmethodID                   _finishMethod = nullptr;
        JavaVM*                     _vm = nullptr;

        ANativeWindow*              _nativeWindow = nullptr;

        std::vector<eCommand>       _readQueue {};
        std::vector<eCommand>       _writeQueue {};

        CommandHandler              _commandHandlers[ static_cast<size_t> ( eCommand::COUNT ) ] {};
        RendererBodyHandler         _rendererBodyHandler = &Core::OnIdle;

        std::thread                 _thread {};
        std::mutex                  _mutex {};

    public:
        Core () = delete;

        Core ( Core const & ) = delete;
        Core &operator = ( Core const & ) = delete;

        Core ( Core && ) = delete;
        Core &operator = ( Core && ) = delete;

        explicit Core ( JNIEnv* env,
            jobject activity,
            jobject assetManager,
            std::string &&cacheDirectory,
            float dpi
        ) noexcept;

        ~Core () = default;

        void OnAboutDestroy ( JNIEnv* env ) noexcept;

        [[nodiscard]] bool OnKeyDown ( int32_t key ) const noexcept;
        [[nodiscard]] bool OnKeyUp ( int32_t key ) const noexcept;

        void OnLeftStick ( float x, float y ) const noexcept;
        void OnRightStick ( float x, float y ) const noexcept;

        void OnLeftTrigger ( float value ) const noexcept;
        void OnRightTrigger ( float value ) const noexcept;

        void OnSurfaceCreated ( JNIEnv* env, jobject surface ) noexcept;
        void OnSurfaceDestroyed () noexcept;

        [[nodiscard]] static std::string const &GetCacheDirectory () noexcept;
        static void Quit () noexcept;

    private:
        [[nodiscard]] bool ExecuteMessageQueue () noexcept;
        void InitCommandHandlers () noexcept;

        void OnFrame () noexcept;
        void OnIdle () noexcept;

        [[nodiscard]] bool OnQuit () noexcept;
        [[nodiscard]] bool OnQuitRequest () noexcept;
        [[nodiscard]] bool OnSwapchainCreated () noexcept;
        [[nodiscard]] bool OnSwapchainDestroyed () noexcept;

        void UpdateFPS ( Timestamp now ) noexcept;

        static void OnHomeUp ( void* context ) noexcept;
};

} // namespace android_vulkan::android


#endif // ANDROID_VULKAN_ANDROID_CORE_HPP
