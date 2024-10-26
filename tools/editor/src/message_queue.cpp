#include <precompiled_headers.hpp>
#include <message_queue.hpp>
#include <trace.hpp>


namespace editor {

void MessageQueue::EnqueueFront ( Message &&message ) noexcept
{
    AV_TRACE ( "Enqueue message" )
    std::lock_guard const lock ( _mutex );
    message._serialNumber = ++_serialNumber;
    _queue.push_front ( std::move ( message ) );
    _isQueueChanged.notify_all ();
}

void MessageQueue::EnqueueBack ( Message &&message ) noexcept
{
    AV_TRACE ( "Enqueue message" )
    std::lock_guard const lock ( _mutex );
    message._serialNumber = ++_serialNumber;

    // Main thread has to sleep in order to avoid buzy loop. Same time having pending operation in message queue is
    // suboptimal for throughput. The solution is to move eMessageType::RunEventLoop to the end of the queue.

    if ( _queue.empty () || _queue.back ()._type != eMessageType::RunEventLoop )
    {
        _queue.push_back ( std::move ( message ) );
        _isQueueChanged.notify_all ();
        return;
    }

    Message &back = _queue.back ();
    Message::SerialNumber const serialNumber = back._serialNumber;
    back = std::move ( message );

    _queue.push_back (
        Message
        {
            ._type = eMessageType::RunEventLoop,
            ._params = nullptr,
            ._serialNumber = serialNumber
        }
    );

    _isQueueChanged.notify_all ();
}

Message MessageQueue::DequeueBegin ( std::optional<Message::SerialNumber> waitOnSerialNumber ) noexcept
{
    AV_TRACE ( "Dequeue message begin" )
    std::unique_lock lock ( _mutex );

    while ( _queue.empty () || ( waitOnSerialNumber && _queue.front ()._serialNumber == *waitOnSerialNumber ) )
        _isQueueChanged.wait ( lock );

    lock.release ();
    Message m = std::move ( _queue.front () );
    _queue.pop_front ();
    return m;
}

void MessageQueue::DequeueEnd () noexcept
{
    AV_TRACE ( "Dequeue message end" )
    _isQueueChanged.notify_all ();
    _mutex.unlock ();
}

void MessageQueue::DequeueEnd ( Message &&refund ) noexcept
{
    AV_TRACE ( "Dequeue message end" )
    _queue.push_front ( std::move ( refund ) );
    _mutex.unlock ();
}

} // namespace editor
