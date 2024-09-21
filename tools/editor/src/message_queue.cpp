#include <message_queue.hpp>
#include <trace.hpp>


namespace editor {

namespace {

constexpr std::chrono::microseconds MAX_WAIT ( 50U );

} // end of anonumous namespace

void MessageQueue::EnqueueFront ( Message &&message ) noexcept
{
    AV_TRACE ( "Enqueue message" )
    std::lock_guard const lock ( _mutex );
    message._serialNumber = ++_serialNumber;
    _queue.push_front ( std::move ( message ) );
    _isQueueNotEmpty.notify_all ();
}

void MessageQueue::EnqueueBack ( Message &&message ) noexcept
{
    AV_TRACE ( "Enqueue message" )
    std::lock_guard const lock ( _mutex );
    message._serialNumber = ++_serialNumber;
    _queue.push_back ( std::move ( message ) );
    _isQueueNotEmpty.notify_all ();
}

Message MessageQueue::DequeueBegin ( std::optional<uint32_t> waitOnSerialNumber ) noexcept
{
    AV_TRACE ( "Dequeue message begin" )
    std::unique_lock lock ( _mutex );

    while ( _queue.empty () || ( waitOnSerialNumber && _queue.front ()._serialNumber == *waitOnSerialNumber ) )
        _isQueueNotEmpty.wait_for ( lock, MAX_WAIT );

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
