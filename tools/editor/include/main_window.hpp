#ifndef EDITOR_MAIN_WINDOW_HPP
#define EDITOR_MAIN_WINDOW_HPP


#include "cursor.hpp"
#include "keys.hpp"
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
        bool                                    _handleTyping = false;
        std::optional<char16_t>                 _highSurrogate = std::nullopt;
        HWND                                    _hwnd = nullptr;
        MessageQueue*                           _messageQueue = nullptr;
        size_t                                  _mouseMoveEventID = 0U;
        std::unordered_map<WPARAM, eKey>        _keyboardKeyMapper {};

    public:
        explicit MainWindow () noexcept;

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

        void CaptureKeyboard () noexcept;
        void ReleaseKeyboard () noexcept;

        void ChangeCursor ( eCursor cursor ) noexcept;
        [[nodiscard]] float GetDPI () const noexcept;
        [[nodiscard]] HWND GetNativeWindow () const noexcept;

        void ReadClipboard () const noexcept;
        void WriteClipboard ( std::u32string const &string ) const noexcept;

    private:
        void OnChar ( WPARAM wParam ) noexcept;
        void OnClose () noexcept;
        void OnCreate ( HWND hwnd ) noexcept;
        void OnDPIChanged ( WPARAM wParam, LPARAM lParam ) noexcept;
        void OnGetMinMaxInfo ( LPARAM lParam ) noexcept;
        void OnKeyboardKey ( WPARAM wParam, eMessageType messageType ) noexcept;
        void OnMouseKey ( LPARAM lParam, eKey key, eMessageType messageType ) noexcept;
        void OnMouseMove ( LPARAM lParam ) noexcept;
        void OnSize ( WPARAM wParam ) noexcept;

        void CreateCursors () noexcept;
        void Save () noexcept;
        void Load () noexcept;

        [[nodiscard]] static KeyModifier MakeKeyModifier () noexcept;
        [[nodiscard]] static LRESULT CALLBACK WindowHandler ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
};

} // namespace editor


#endif // EDITOR_MAIN_WINDOW_HPP
