#include <precompiled_headers.hpp>
#include <timer_manager.hpp>
#include <trace.hpp>


namespace editor {

namespace {

constexpr std::chrono::milliseconds IDLE ( 1U );

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

TimerManager::TimerManager ( MessageQueue& messageQueue ) noexcept:
    _messageQueue ( messageQueue )
{
    // NOTHING
}

void TimerManager::Init () noexcept
{
     AV_TRACE ( "Timer manager: init" )

    _thread = std::thread (
        [ this ] () noexcept {
            AV_THREAD_NAME ( "Timer" )
            EventLoop ();
        }
    );
}

void TimerManager::Destroy () noexcept
{
    AV_TRACE ( "Timer manager: destroy" )

    if ( _thread.joinable () ) [[likely]]
        _thread.join ();

    for ( Timer::State* timer : _timers )
        delete timer;

    _timers.clear ();
}

void TimerManager::EventLoop () noexcept
{
    std::optional<Message::SerialNumber> lastRefund {};

    for ( ; ; )
    {
        AV_TRACE ( "Event loop" )
        Message message = _messageQueue.DequeueBegin ( lastRefund );

        GX_DISABLE_WARNING ( 4061 )

        switch ( message._type )
        {
            case eMessageType::Shutdown:
                OnShutdown ( std::move ( message ) );
            return;

            case eMessageType::StartTimer:
                OnStartTimer ( std::move ( message ) );
            break;

            case eMessageType::StopTimer:
                OnStopTimer ( std::move ( message ) );
            break;

            default:
                lastRefund = message._serialNumber;
                _messageQueue.DequeueEnd ( std::move ( message ), MessageQueue::eRefundLocation::Front );
            break;
        }

        GX_ENABLE_WARNING ( 4061 )

        auto const now = std::chrono::steady_clock::now ();

        for ( auto it = _timers.begin (); it != _timers.end (); )
        {
            if ( Timer::State* timer = *it; timer->Invoke ( now ) ) [[unlikely]]
            {
                delete timer;
                it = _timers.erase ( it );
                continue;
            }

            ++it;
        }

        std::this_thread::sleep_for ( IDLE );
    }
}

void TimerManager::OnStartTimer ( Message &&message ) noexcept
{
    AV_TRACE ( "Start timer" )
    _messageQueue.DequeueEnd ();
    _timers.insert ( static_cast<Timer::State*> ( message._params ) );
}

void TimerManager::OnStopTimer ( Message &&message ) noexcept
{
    AV_TRACE ( "Stop timer" )
    _messageQueue.DequeueEnd ();

    auto* timer = static_cast<Timer::State*> ( message._params );
    _timers.erase ( timer );
    delete timer;
}

void TimerManager::OnShutdown ( Message &&refund ) noexcept
{
    AV_TRACE ( "Shutdown" )
    _messageQueue.DequeueEnd ( std::move ( refund ), MessageQueue::eRefundLocation::Back );

    _messageQueue.EnqueueFront (
        {
            ._type = eMessageType::ModuleStopped,
            ._params = nullptr,
            ._serialNumber = 0U
        }
    );
}

} // namespace editor
