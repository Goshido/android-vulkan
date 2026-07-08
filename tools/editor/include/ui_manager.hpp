#ifndef EDITOR_UI_MANAGER_HPP
#define EDITOR_UI_MANAGER_HPP


#include "message_queue.hpp"
#include "widget.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <shared_mutex>
#include <thread>

GX_RESTORE_WARNING_STATE


namespace editor {

class UIManager final
{
    private:
        size_t                                  _eventID = 0U;

        Widget*                                 _hoverWidget = nullptr;
        Widget*                                 _mouseCapture = nullptr;
        std::shared_mutex                       _mutex {};
        std::thread                             _thread {};
        Widget*                                 _typingCapture = nullptr;
        std::deque<std::unique_ptr<Widget>>     _widgets {};

        int32_t                                 _lastMouseX = std::numeric_limits<int32_t>::min ();
        int32_t                                 _lastMouseY = std::numeric_limits<int32_t>::min ();

        size_t                                  _neededUIVertices = 0U;
        bool                                    _needRefill = false;

    public:
        explicit UIManager () = default;

        UIManager ( UIManager const & ) = delete;
        UIManager &operator = ( UIManager const & ) = delete;

        UIManager ( UIManager && ) = delete;
        UIManager &operator = ( UIManager && ) = delete;

        ~UIManager () = default;

        void Init () noexcept;
        void Destroy () noexcept;

        void ComputeLayout ( android_vulkan::Renderer &renderer, pbr::UIPass &pass ) noexcept;
        void Submit ( android_vulkan::Renderer &renderer, pbr::UIPass &pass ) noexcept;

    private:
        void EventLoop () noexcept;
        void OnDoubleClick ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnInvokeUI ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnKeyboardKeyDown ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnKeyboardKeyUp ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnKillFocus ( MessageQueue &messageQueue ) noexcept;
        void OnSetFocus ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnMouseHover ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnMouseButtonDown ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnMouseButtonUp ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnMouseMoved ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnReadClipboardResponse ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnShutdown ( MessageQueue &messageQueue, Message &&refund ) noexcept;
        void OnStartWidgetCaptureMouse ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnStopWidgetCaptureMouse ( MessageQueue &messageQueue ) noexcept;
        void OnTyping ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnUIAddWidget ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnUIRemoveWidget ( MessageQueue &messageQueue, Message &&message ) noexcept;
};

} // namespace editor


#endif // EDITOR_UI_MANAGER_HPP
