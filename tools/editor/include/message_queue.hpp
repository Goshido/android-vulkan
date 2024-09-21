#ifndef EDITOR_MESSAGE_QUEUE_HPP
#define EDITOR_MESSAGE_QUEUE_HPP


#include "message.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <deque>
#include <mutex>
#include <optional>

GX_RESTORE_WARNING_STATE


namespace editor {

class MessageQueue final
{
    private:
        std::mutex                  _mutex {};
        std::condition_variable     _isQueueNotEmpty {};
        std::deque<Message>         _queue {};
        uint32_t                    _serialNumber = 0U;

    public:
        explicit MessageQueue () = default;

        MessageQueue ( MessageQueue const & ) = delete;
        MessageQueue &operator = ( MessageQueue const & ) = delete;

        MessageQueue ( MessageQueue && ) = delete;
        MessageQueue &operator = ( MessageQueue && ) = delete;

        ~MessageQueue () = default;

        void EnqueueFront ( Message &&message ) noexcept;
        void EnqueueBack ( Message &&message ) noexcept;

        [[nodiscard]] Message DequeueBegin ( std::optional<uint32_t> waitOnSerialNumber ) noexcept;
        void DequeueEnd () noexcept;
        void DequeueEnd ( Message &&refund ) noexcept;
};

} // namespace editor


#endif // EDITOR_MESSAGE_QUEUE_HPP
