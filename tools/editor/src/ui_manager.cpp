#include <trace.hpp>
#include <ui_manager.hpp>


namespace editor {

void UIManager::Init ( MessageQueue &messageQueue ) noexcept
{
    AV_TRACE ( "UI: init" )
    _messageQueue = &messageQueue;

    _thread = std::thread (
        [ this ]() noexcept
        {
            AV_THREAD_NAME ( "UI" )
            EventLoop ();
        }
    );
}

void UIManager::Destroy () noexcept
{
    AV_TRACE ( "UI: destroy" )

    if ( _thread.joinable () ) [[likely]]
        _thread.join ();

    _messageQueue = nullptr;
}

void UIManager::EventLoop () noexcept
{
    MessageQueue &messageQueue = *_messageQueue;
    std::optional<Message::SerialNumber> lastRefund {};

    for ( ; ; )
    {
        AV_TRACE ( "Event loop" )
        Message message = messageQueue.DequeueBegin ( lastRefund );

        GX_DISABLE_WARNING ( 4061 )

        switch ( message._type )
        {
            case eMessageType::Shutdown:
                OnShutdown ( std::move ( message ) );
            return;

            default:
                lastRefund = message._serialNumber;
                messageQueue.DequeueEnd ( std::move ( message ) );
            break;
        }

        GX_ENABLE_WARNING ( 4061 )
    }
}

void UIManager::OnShutdown ( Message &&refund ) noexcept
{
    AV_TRACE ( "Shutdown" )
    _messageQueue->DequeueEnd ( std::move ( refund ) );

    _messageQueue->EnqueueFront (
        Message
        {
            ._type = eMessageType::ModuleStopped,
            ._params = nullptr,
            ._serialNumber = 0U
        }
    );
}

} // namespace editor
