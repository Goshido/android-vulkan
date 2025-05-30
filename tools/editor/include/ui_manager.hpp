#ifndef EDITOR_UI_MANAGER_HPP
#define EDITOR_UI_MANAGER_HPP


#include "message_queue.hpp"
#include <pbr/ui_pass.hpp>
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
        MessageQueue                            &_messageQueue;
        Widget*                                 _mouseCapture = nullptr;
        std::shared_mutex                       _mutex {};
        std::thread                             _thread {};
        std::deque<std::unique_ptr<Widget>>     _widgets {};

    public:
        UIManager () = delete;

        UIManager ( UIManager const & ) = delete;
        UIManager &operator = ( UIManager const & ) = delete;

        UIManager ( UIManager && ) = delete;
        UIManager &operator = ( UIManager && ) = delete;

        explicit UIManager ( MessageQueue &messageQueue ) noexcept;

        ~UIManager () = default;

        void Init () noexcept;
        void Destroy () noexcept;

        void RenderUI ( android_vulkan::Renderer &renderer, pbr::UIPass &pass ) noexcept;

    private:
        void CreateWidgets () noexcept;
        void EventLoop () noexcept;
        void OnKeyboardKeyDown ( Message &&message ) noexcept;
        void OnKeyboardKeyUp ( Message &&message ) noexcept;
        void OnMouseHover ( Message &&message ) noexcept;
        void OnMouseButtonDown ( Message &&message ) noexcept;
        void OnMouseButtonUp ( Message &&message ) noexcept;
        void OnMouseMoved ( Message &&message ) noexcept;
        void OnShutdown ( Message &&refund ) noexcept;
        void OnStartWidgetCaptureMouse ( Message &&message ) noexcept;
        void OnStopWidgetCaptureMouse () noexcept;
        void OnUIAddWidget ( Message &&message ) noexcept;
        void OnUIRemoveWidget ( Message &&message ) noexcept;
};

} // namespace editor


#endif // EDITOR_UI_MANAGER_HPP
