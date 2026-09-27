#ifndef EDITOR_TIMER_MANAGER_HPP
#define EDITOR_TIMER_MANAGER_HPP


#include "message_queue.hpp"
#include "timer.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <thread>

GX_RESTORE_WARNING_STATE


namespace editor {

class TimerManager final
{
    private:
        std::thread                             _thread {};
        std::unordered_set<Timer::State*>       _timers {};

    public:
        explicit TimerManager () = default;

        TimerManager ( TimerManager const & ) = delete;
        TimerManager &operator = ( TimerManager const & ) = delete;

        TimerManager ( TimerManager && ) = delete;
        TimerManager &operator = ( TimerManager && ) = delete;

        ~TimerManager () = default;

        void Init () noexcept;
        void Destroy () noexcept;

    private:
        void EventLoop () noexcept;

        void OnStartTimer ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnStopTimer ( MessageQueue &messageQueue, Message &&message ) noexcept;
        void OnShutdown ( MessageQueue &messageQueue, Message &&refund ) noexcept;
};

} // namespace editor


#endif // EDITOR_TIMER_MANAGER_HPP
