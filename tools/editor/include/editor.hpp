#ifndef EDITOR_EDITOR_HPP
#define EDITOR_EDITOR_HPP


#include "command_line.hpp"
#include "main_window.hpp"
#include <render_session.hpp>
#include <renderer.hpp>
#include "ui_manager.hpp"


namespace editor {

class Editor final
{
    private:
        constexpr static std::string_view       DEFAULT_GPU = "";
        constexpr static float                  DEFAULT_UI_ZOOM = 1.0F;
        constexpr static bool                   DEFAULT_VSYNC = true;

        struct Config final
        {
            std::string                         _gpu = std::string ( DEFAULT_GPU );
            float                               _uiZoom = DEFAULT_UI_ZOOM;
            bool                                _vSync = DEFAULT_VSYNC;
        };

    private:
        CommandLine                             _commandLine {};
        bool                                    _frameComplete = true;
        MainWindow                              _mainWindow {};
        MessageQueue                            _messageQueue {};
        RenderSession                           _renderSession {};
        android_vulkan::Renderer                _renderer {};
        bool                                    _stopRendering = false;
        uint16_t                                _runningModules = 0U;
        UIManager                               _uiManager {};
        float                                   _uiZoom = DEFAULT_UI_ZOOM;

    public:
        Editor () = delete;

        Editor ( Editor const & ) = delete;
        Editor &operator = ( Editor const & ) = delete;

        Editor ( Editor && ) = delete;
        Editor &operator = ( Editor && ) = delete;

        explicit Editor ( CommandLine &&commandLine ) noexcept;

        ~Editor () = default;

        [[nodiscard]] bool Run () noexcept;

    private:
        [[nodiscard]] bool InitModules () noexcept;
        void DestroyModules () noexcept;

        void EventLoop () noexcept;
        void OnCaptureMouse () noexcept;
        void OnReleaseMouse () noexcept;
        void OnChangeCursor ( Message &&message ) noexcept;
        void OnDPIChanged ( Message &&message ) noexcept;
        void OnFrameComplete () noexcept;
        void OnModuleStopped () noexcept;
        void OnRecreateSwapchain () noexcept;
        void OnRunEvent () noexcept;
        void OnShutdown () noexcept;
        void OnWindowVisibilityChanged ( Message &&message ) noexcept;
        void ScheduleEventLoop () noexcept;

        [[nodiscard]] std::string_view GetUserGPU () const noexcept;
        [[nodiscard]] Config LoadConfig () noexcept;
};

} // namespace editor


#endif // EDITOR_EDITOR_HPP
