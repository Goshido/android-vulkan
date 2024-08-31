#ifndef EDITOR_EDITOR_HPP
#define EDITOR_EDITOR_HPP


#include "command_line.hpp"
#include "main_window.hpp"
#include <render_session.hpp>
#include <renderer.hpp>


namespace editor {

class Editor final
{
    private:
        constexpr static float                  DEFAULT_DPI = 96.0F;
        constexpr static std::string_view       DEFAULT_GPU = "";
        constexpr static bool                   DEFAULT_VSYNC = true;

        struct Config final
        {
            float                               _dpi = DEFAULT_DPI;
            std::string                         _gpu = std::string ( DEFAULT_GPU );
            bool                                _vSync = DEFAULT_VSYNC;
        };

    private:
        CommandLine                             _commandLine {};
        MainWindow                              _mainWindow {};
        MessageQueue                            _messageQueue {};
        RenderSession                           _renderSession {};
        android_vulkan::Renderer                _renderer {};
        bool                                    _stopRendering = false;

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
        void OnRunEvent () noexcept;
        void OnWindowVisibilityChanged ( Message &&message ) noexcept;
        void ScheduleEventLoop () noexcept;
        void Shutdown () noexcept;

        [[nodiscard]] std::string_view GetUserGPU () const noexcept;
        [[nodiscard]] Config LoadConfig () noexcept;
};

} // namespace editor


#endif // EDITOR_EDITOR_HPP
