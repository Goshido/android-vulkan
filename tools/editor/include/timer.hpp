#ifndef EDITOR_TIMER_HPP
#define EDITOR_TIMER_HPP


#include "message_queue.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <chrono>
#include <functional>

GX_RESTORE_WARNING_STATE


namespace editor {

class TimerManager;

// Time resolution is 1ms.
class Timer final
{
    friend class TimerManager;

    public:
        enum class eType : uint8_t
        {
            Repeat,
            SingleShot
        };

        using ElapsedTime = std::chrono::duration<float>;
        using Callback = std::function<void ( ElapsedTime &&elapsedTime )>;
        using Interval = std::chrono::duration<std::chrono::steady_clock::rep, std::chrono::steady_clock::period>;

    private:
        using Timestamp = std::chrono::steady_clock::time_point;

    private:
        Callback const      _callback;
        Interval const      _interval {};
        MessageQueue        &_messageQueue;
        Timestamp           _schedule {};
        eType const         _type = eType::SingleShot;

    public:
        Timer () = delete;

        Timer ( Timer const & ) = delete;
        Timer &operator = ( Timer const & ) = delete;

        Timer ( Timer && ) = delete;
        Timer &operator = ( Timer && ) = delete;

        explicit Timer ( MessageQueue &messageQueue,
            eType type,
            Interval const &interval,
            Callback &&callback
        ) noexcept;

        void Destroy () noexcept;

        // Method returns true if this timer should be excluded from the scheduling process.
        // Otherwise method returns false.
        [[nodiscard]] bool Invoke ( Timestamp const &now ) noexcept;

    private:
        ~Timer () = default;
};

} // namespace editor


#endif // EDITOR_TIMER_HPP
