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

        class State final
        {
            public:
                Callback const      _callback;
                Interval const      _interval {};
                Timestamp           _schedule {};
                eType const         _type = eType::SingleShot;

            public:
                State () = delete;

                State ( State const & ) = delete;
                State &operator = ( State const & ) = delete;

                State ( State && ) = delete;
                State &operator = ( State && ) = delete;

                explicit State ( eType type, Interval const &interval, Callback &&callback ) noexcept;

                ~State () = default;

                // Method returns true if this timer should be excluded from the scheduling process.
                // Otherwise method returns false.
                [[nodiscard]] bool Invoke ( Timestamp const &now ) noexcept;
        };

    private:
        MessageQueue                &_messageQueue;
        State*                      _state = nullptr;

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

        ~Timer () noexcept;
};

} // namespace editor


#endif // EDITOR_TIMER_HPP
