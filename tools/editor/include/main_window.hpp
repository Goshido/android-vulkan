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
        [[nodiscard]] bool Destroy () noexcept;

        void Execute () noexcept;
        [[nodiscard]] HWND GetNativeWindow () const noexcept;

    private:
        // Note we can't use '_nativeWindow' at this point because CreateWindowEx did not finish yet.
        // This 'hwnd' should be used from WM_CREATE message. In reality 'hwnd' will be exactly the same as
        // result of CreateWindowEx. Once again the call sequence is the following:
        // MakeWindow
        //      CreateWindowEx
        //          WM_CREATE
        //              OnCreate: -> HWND is connected to MainWindow object
        // control returns to caller code
        void OnCreate ( HWND hwnd ) noexcept;

        void OnClose () noexcept;

        static LRESULT CALLBACK WindowHandler ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
};

} // namespace editor


#endif // EDITOR_MAIN_WINDOW_HPP
