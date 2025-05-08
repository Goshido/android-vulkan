#ifndef EDITOR_TIMER_MANAGER_HPP
#define EDITOR_TIMER_MANAGER_HPP


#include "timer.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <thread>

GX_RESTORE_WARNING_STATE


namespace editor {

class TimerManager final
{
    private:
        MessageQueue                    &_messageQueue;
        std::thread                     _thread {};
        std::unordered_set<Timer*>      _timers {};

    public:
        TimerManager () = delete;

        TimerManager ( TimerManager const & ) = delete;
        TimerManager &operator = ( TimerManager const & ) = delete;

        TimerManager ( TimerManager && ) = delete;
        TimerManager &operator = ( TimerManager && ) = delete;

        explicit TimerManager ( MessageQueue &messageQueue ) noexcept;

        ~TimerManager () = default;

        void Init () noexcept;
        void Destroy () noexcept;

    private:
        void EventLoop () noexcept;

        void OnStartTimer ( Message &&message ) noexcept;
        void OnStopTimer ( Message &&message ) noexcept;
        void OnShutdown ( Message &&refund ) noexcept;
};

} // namespace editor


#endif // EDITOR_TIMER_MANAGER_HPP
