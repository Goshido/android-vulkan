#include <precompiled_headers.hpp>
#include <io.hpp>
#include <message_queue.hpp>
#include <trace.hpp>


namespace editor {

void IO::Init () noexcept
{
    AV_TRACE ( "IO: init" )

    _thread = std::thread (
        [ this ]() noexcept
        {
            AV_THREAD_NAME ( "IO" )
            EventLoop ();
        }
    );
}

void IO::Destroy () noexcept
{
    AV_TRACE ( "IO: destroy" )

    if ( _thread.joinable () ) [[likely]]
    {
        _thread.join ();
    }
}

void IO::EventLoop () noexcept
{
    MessageQueue &messageQueue = MessageQueue::Instance ();
    std::optional<Message::SerialNumber> lastRefund {};

    for ( ; ; )
    {
        AV_TRACE ( "Event loop" )
        Message message = messageQueue.DequeueBegin ( lastRefund );

        GX_DISABLE_WARNING ( 4061 )

        switch ( message._type )
        {
            case eMessageType::InvokeIO:
                OnInvokeIO ( std::move ( message ) );
            break;

            case eMessageType::Shutdown:
                OnShutdown ( std::move ( message ) );
            return;

            default:
                lastRefund = message._serialNumber;
                messageQueue.DequeueEnd ( std::move ( message ), MessageQueue::eRefundLocation::Front );
            break;
        }

        GX_ENABLE_WARNING ( 4061 )
    }
}

void IO::OnInvokeIO ( Message &&message ) noexcept
{
    AV_TRACE ( "Invoke" )
    MessageQueue::Instance ().DequeueEnd ();
    std::ignore = message._action ();
}

void IO::OnShutdown ( Message &&refund ) noexcept
{
    AV_TRACE ( "Shutdown" )
    MessageQueue &messageQueue = MessageQueue::Instance ();
    messageQueue.DequeueEnd ( std::move ( refund ), MessageQueue::eRefundLocation::Back );

    messageQueue.EnqueueFront (
        {
            ._type = eMessageType::ModuleStopped,
            ._action = nullptr,
            ._serialNumber = 0U
        }
    );
}

} // namespace editor
