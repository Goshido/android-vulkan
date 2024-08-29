#ifndef EDITOR_MAIN_WINDOW_HPP
#define EDITOR_MAIN_WINDOW_HPP


#include "message_queue.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <Windows.h>

GX_RESTORE_WARNING_STATE


namespace editor {

class MainWindow final
{
    private:
        HWND                _nativeWindow = nullptr;
        ATOM                _classID = 0;
        MessageQueue*       _messageQueue = nullptr;

    public:
        MainWindow () = default;

        MainWindow ( MainWindow const & ) = delete;
        MainWindow &operator = ( MainWindow const & ) = delete;

        MainWindow ( MainWindow && ) = delete;
        MainWindow &operator = ( MainWindow && ) = delete;

        ~MainWindow () = default;

        [[nodiscard]] bool MakeWindow ( MessageQueue &messageQueue ) noexcept;
        [[nodiscard]] bool MakeSwapchain () noexcept;
        [[nodiscard]] bool Destroy () noexcept;

        void Execute () noexcept;

    private:
        void Connect ( HWND nativeWindow ) noexcept;
        void OnClose () noexcept;

        static LRESULT CALLBACK WindowHandler ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
};

} // namespace editor


#endif // EDITOR_MAIN_WINDOW_HPP
