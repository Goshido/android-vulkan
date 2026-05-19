#ifndef EDITOR_IO_HPP
#define EDITOR_IO_HPP


#include "message_queue.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <thread>

GX_RESTORE_WARNING_STATE


namespace editor {

class IO final
{
    private:
        std::thread     _thread {};

    public:
        explicit IO () = default;

        IO ( IO const & ) = delete;
        IO &operator = ( IO const & ) = delete;

        IO ( IO && ) = delete;
        IO &operator = ( IO && ) = delete;

        ~IO () = default;

        void Init () noexcept;
        void Destroy () noexcept;

    private:
        void EventLoop () noexcept;

        void OnInvokeIO ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnShutdown ( MessageQueue &messageQueue, Message &&refund ) noexcept;
};

} // namespace editor


#endif // EDITOR_IO_HPP
