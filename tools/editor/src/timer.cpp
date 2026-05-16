#include <precompiled_headers.hpp>
#include <message_queue.hpp>
#include <timer.hpp>


namespace editor {

Timer::State::State ( eType type, Interval const &interval, Callback &&callback ) noexcept:
    _callback ( std::move ( callback ) ),
    _interval ( interval ),
    _schedule ( std::chrono::steady_clock::now () + interval ),
    _type ( type )
{
    // NOTHING
}

bool Timer::State::Invoke ( Timestamp const &now ) noexcept
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

//----------------------------------------------------------------------------------------------------------------------

Timer::Timer ( eType type, Interval const &interval, Callback &&callback ) noexcept:
    _state ( new State ( type, interval, std::move ( callback ) ) )
{
    MessageQueue::Instance ().EnqueueBack (
        {
            ._type = eMessageType::StartTimer,

            ._action = [ state = _state ] () noexcept {
                return state;
            },

            ._serialNumber = 0U
        }
    );
}

Timer::~Timer () noexcept
{
    MessageQueue::Instance ().EnqueueBack (
        {
            ._type = eMessageType::StopTimer,

            ._action = [ state = _state ] () noexcept {
                return state;
            },

            ._serialNumber = 0U
        }
    );
}

} // namespace editor
