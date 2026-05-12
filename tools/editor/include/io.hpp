#ifndef EDITOR_IO_HPP
#define EDITOR_IO_HPP


#include "message_queue.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <mutex>

GX_RESTORE_WARNING_STATE


namespace editor {

class IO final
{
    private:
        MessageQueue    &_messageQueue;
        std::thread     _thread {};

    public:
        IO () = delete;

        IO ( IO const & ) = delete;
        IO &operator = ( IO const & ) = delete;

        IO ( IO && ) = delete;
        IO &operator = ( IO && ) = delete;

        explicit IO ( MessageQueue &messageQueue ) noexcept;

        ~IO () = default;

        void Init () noexcept;
        void Destroy () noexcept;

    private:
        void EventLoop () noexcept;

        void OnInvokeIO ( Message &&message ) noexcept;
        void OnShutdown ( Message &&refund ) noexcept;
};

} // namespace editor


#endif // EDITOR_IO_HPP
