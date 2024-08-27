#ifndef EDITOR_EDITOR_HPP
#define EDITOR_EDITOR_HPP


#include "command_line.hpp"
#include "main_window.hpp"
#include <renderer.hpp>


namespace editor {

class Editor final
{
    private:
        MainWindow                  _mainWindow {};
        android_vulkan::Renderer    _renderer {};
        bool                        _run = true;

        float                       _dpi = 96.0F;
        std::string                 _gpu {};

    public:
        Editor () = delete;

        Editor ( Editor const & ) = delete;
        Editor &operator = ( Editor const & ) = delete;

        Editor ( Editor && ) = delete;
        Editor &operator = ( Editor && ) = delete;

        explicit Editor ( CommandLine &&args ) noexcept;

        ~Editor () = default;

        [[nodiscard]] bool Run () noexcept;

    private:
        void LoadConfig () noexcept;
        void SaveConfig () noexcept;
};

} // namespace editor


#endif // EDITOR_EDITOR_HPP
