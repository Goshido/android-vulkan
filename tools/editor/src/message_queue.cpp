#include <message_queue.hpp>
#include <trace.hpp>


namespace editor {

void MessageQueue::Enqueue ( Message &&message ) noexcept
{
    AV_TRACE ( "Enqueue message" )
    std::lock_guard const lock ( _mutex );
    _queue.push_back ( std::move ( message ) );
    _isQueueNotEmpty.notify_all ();
}

Message MessageQueue::Dequeue () noexcept
{
    AV_TRACE ( "Dequeue message" )
    std::unique_lock lock ( _mutex );

    while ( _queue.empty () )
        _isQueueNotEmpty.wait ( lock );

    Message m = std::move ( _queue.front () );
    _queue.pop_front ();
    return m;
}

} // namespace editor
