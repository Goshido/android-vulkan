#include <message_queue.hpp>
#include <trace.hpp>


namespace editor {

void MessageQueue::EnqueueFront ( Message &&message ) noexcept
{
    AV_TRACE ( "Enqueue message" )
    std::lock_guard const lock ( _mutex );
    _queue.push_front ( std::move ( message ) );
    _isQueueNotEmpty.notify_all ();
}

void MessageQueue::EnqueueBack ( Message &&message ) noexcept
{
    AV_TRACE ( "Enqueue message" )
    std::lock_guard const lock ( _mutex );
    _queue.push_back ( std::move ( message ) );
    _isQueueNotEmpty.notify_all ();
}

Message MessageQueue::DequeueBegin () noexcept
{
    AV_TRACE ( "Dequeue message begin" )
    std::unique_lock lock ( _mutex );

    while ( _queue.empty () )
        _isQueueNotEmpty.wait ( lock );

    lock.release ();
    Message m = std::move ( _queue.front () );
    _queue.pop_front ();
    return m;
}

void MessageQueue::DequeueEnd () noexcept
{
    AV_TRACE ( "Dequeue message end" )
    _mutex.unlock ();
}

void MessageQueue::DequeueEnd ( Message &&refund ) noexcept
{
    AV_TRACE ( "Dequeue message end" )
    _queue.push_front ( std::move ( refund ) );
    _mutex.unlock ();
}

} // namespace editor
