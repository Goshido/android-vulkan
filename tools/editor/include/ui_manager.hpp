#ifndef EDITOR_UI_MANAGER_HPP
#define EDITOR_UI_MANAGER_HPP


#include "message_queue.hpp"
#include <pbr/ui_pass.hpp>
#include "widget.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <thread>

GX_RESTORE_WARNING_STATE


namespace editor {

class UIManager final
{
    private:
        size_t                                  _eventID = 0U;
        MessageQueue*                           _messageQueue = nullptr;
        Widget*                                 _mouseCapture = nullptr;
        std::thread                             _thread {};
        std::deque<std::unique_ptr<Widget>>     _widgets {};

    public:
        explicit UIManager () = default;

        UIManager ( UIManager const & ) = delete;
        UIManager &operator = ( UIManager const & ) = delete;

        UIManager ( UIManager && ) = delete;
        UIManager &operator = ( UIManager && ) = delete;

        ~UIManager () = default;

        void Init ( MessageQueue &messageQueue ) noexcept;
        void Destroy () noexcept;

        void RenderUI ( android_vulkan::Renderer &renderer, pbr::UIPass &pass ) noexcept;

    private:
        void CreateWidgets () noexcept;
        void EventLoop () noexcept;
        void OnMouseKeyDown ( Message &&message ) noexcept;
        void OnMouseKeyUp ( Message &&message ) noexcept;
        void OnMouseMoved ( Message &&message ) noexcept;
        void OnShutdown ( Message &&refund ) noexcept;
        void OnStartWidgetCaptureMouse ( Message &&message ) noexcept;
        void OnStopWidgetCaptureMouse () noexcept;
};

} // namespace editor


#endif // EDITOR_UI_MANAGER_HPP
