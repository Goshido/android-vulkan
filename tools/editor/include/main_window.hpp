#ifndef EDITOR_MAIN_WINDOW_HPP
#define EDITOR_MAIN_WINDOW_HPP


#include "cursor.hpp"
#include "message_queue.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <Windows.h>
#include <unordered_map>

GX_RESTORE_WARNING_STATE


namespace editor {

class MainWindow final
{
    private:
        ATOM                                    _classID = 0;
        std::unordered_map<eCursor, HCURSOR>    _cursors {};
        HWND                                    _hwnd = nullptr;
        MessageQueue*                           _messageQueue = nullptr;
        size_t                                  _mouseMoveEventID = 0U;

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

        void CaptureMouse () noexcept;
        void ReleaseMouse () noexcept;

        void ChangeCursor ( eCursor cursor ) noexcept;
        [[nodiscard]] float GetDPI () const noexcept;
        [[nodiscard]] HWND GetNativeWindow () const noexcept;

    private:
        void OnClose () noexcept;

        // Note we can't use '_hwnd' at this point because CreateWindowEx did not finish yet.
        // 'hwnd' should be used from WM_CREATE message. In reality 'hwnd' will be exactly the same as
        // result of CreateWindowEx. Once again the call sequence is the following:
        // MakeWindow
        //      CreateWindowEx
        //          WM_CREATE
        //              OnCreate: -> HWND is connected to MainWindow object and assigned to '_hwnd'
        // control returns to caller code
        void OnCreate ( HWND hwnd ) noexcept;

        void OnDPIChanged ( WPARAM wParam, LPARAM lParam ) noexcept;
        void OnGetMinMaxInfo ( LPARAM lParam ) noexcept;
        void OnLButtonDown ( LPARAM lParam ) noexcept;
        void OnLButtonUp ( LPARAM lParam ) noexcept;
        void OnMButtonDown ( LPARAM lParam ) noexcept;
        void OnMButtonUp ( LPARAM lParam ) noexcept;
        void OnMouseMove ( LPARAM lParam ) noexcept;
        void OnRButtonDown ( LPARAM lParam ) noexcept;
        void OnRButtonUp ( LPARAM lParam ) noexcept;
        void OnSize ( WPARAM wParam ) noexcept;

        void CreateCursors () noexcept;
        void Save () noexcept;
        void Load () noexcept;

        static LRESULT CALLBACK WindowHandler ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
};

} // namespace editor


#endif // EDITOR_MAIN_WINDOW_HPP
