#include <precompiled_headers.hpp>
#include <io.hpp>
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
    messageQueue.EnqueueBack ( Message ( eMessageType::ModuleStarted ) );
    std::optional<Message::SerialNumber> lastRefund {};

    for ( ; ; )
    {
        AV_TRACE ( "Event loop" )
        Message message = messageQueue.DequeueBegin ( lastRefund );

        GX_DISABLE_WARNING ( 4061 )

        switch ( message._type )
        {
            case eMessageType::InvokeIO:
                OnInvokeIO ( messageQueue, std::move ( message ) );
            break;

            case eMessageType::StopIO:
                OnStopIO ( messageQueue );
            return;

            default:
                lastRefund = message._serialNumber;
                messageQueue.DequeueEnd ( std::move ( message ), MessageQueue::eRefundLocation::Front );
            break;
        }

        GX_ENABLE_WARNING ( 4061 )
    }
}

void IO::OnInvokeIO ( MessageQueue &messageQueue, Message &&message ) noexcept
{
    AV_TRACE ( "Invoke" )
    messageQueue.DequeueEnd ();
    std::ignore = message._action ();
}

void IO::OnStopIO ( MessageQueue &messageQueue ) noexcept
{
    AV_TRACE ( "Shutdown" )
    messageQueue.DequeueEnd ();
    messageQueue.EnqueueFront ( Message ( eMessageType::ModuleStopped ) );
}

} // namespace editor
