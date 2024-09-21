#ifndef EDITOR_UI_MANAGER_HPP
#define EDITOR_UI_MANAGER_HPP


#include "message_queue.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <thread>

GX_RESTORE_WARNING_STATE


namespace editor {

class UIManager final
{
    private:
        MessageQueue*       _messageQueue = nullptr;
        std::thread         _thread {};

    public:
        explicit UIManager () = default;

        UIManager ( UIManager const & ) = delete;
        UIManager &operator = ( UIManager const & ) = delete;

        UIManager ( UIManager && ) = delete;
        UIManager &operator = ( UIManager && ) = delete;

        ~UIManager () = default;

        void Init ( MessageQueue &messageQueue ) noexcept;
        void Destroy () noexcept;

    private:
        void EventLoop () noexcept;
        void OnShutdown ( Message &&refund ) noexcept;
};

} // namespace editor


#endif // EDITOR_UI_MANAGER_HPP
