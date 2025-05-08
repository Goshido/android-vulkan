#include <precompiled_headers.hpp>
#include <timer.hpp>


namespace editor {

Timer::Timer ( MessageQueue &messageQueue, eType type, Interval const &interval, Callback &&callback ) noexcept:
    _callback ( std::move ( callback ) ),
    _interval ( interval ),
    _messageQueue ( messageQueue ),
    _schedule ( std::chrono::steady_clock::now () + interval ),
    _type ( type )
{
    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::StartTimer,
            ._params = this,
            ._serialNumber = 0U
        }
    );
}

void Timer::Destroy () noexcept
{
    _messageQueue.EnqueueBack (
        {
            ._type = eMessageType::StopTimer,
            ._params = this,
            ._serialNumber = 0U
        }
    );
}

bool Timer::Invoke ( Timestamp const &now ) noexcept
{
    if ( now < _schedule ) [[likely]]
        return false;

    _callback ( _interval + _schedule - now );

    switch ( _type )
    {
        case eType::Repeat:
            _schedule = now + _interval;
        return false;

        case eType::SingleShot:
            [[fallthrough]];
        default:
            // IMPOSSIBLE
        return true;
    }
}

} // namespace editor
