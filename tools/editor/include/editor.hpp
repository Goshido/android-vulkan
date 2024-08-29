#ifndef EDITOR_EDITOR_HPP
#define EDITOR_EDITOR_HPP


#include "command_line.hpp"
#include "main_window.hpp"
#include "message_queue.hpp"
#include <renderer.hpp>


namespace editor {

class Editor final
{
    private:
        constexpr static float                  DEFAULT_DPI = 96.0F;
        constexpr static std::string_view       DEFAULT_GPU = "";

        struct Config final
        {
            float                               _dpi = DEFAULT_DPI;
            std::string                         _gpu = std::string ( DEFAULT_GPU );
        };

    private:
        CommandLine                             _commandLine {};
        MainWindow                              _mainWindow {};
        MessageQueue                            _messageQueue {};
        android_vulkan::Renderer                _renderer {};

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
        void ScheduleEventLoop () noexcept;

        [[nodiscard]] std::string_view GetUserGPU () const noexcept;
        [[nodiscard]] Config LoadConfig () noexcept;
};

} // namespace editor


#endif // EDITOR_EDITOR_HPP
