#include <message_queue.hpp>


namespace editor {

void MessageQueue::Enqueue ( Message &&message ) noexcept
{
    _queue.push_back ( std::move ( message ) );
    _isQueueNotEmpty.notify_all ();
}

Message MessageQueue::Dequeue () noexcept
{
    std::unique_lock lock ( _mutex );

    while ( _queue.empty () )
        _isQueueNotEmpty.wait ( lock );

    Message m = std::move ( _queue.front () );
    _queue.pop_front ();
    return m;
}

} // namespace editor
