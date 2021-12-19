#ifndef ANDROID_VULKAN_GAMEPAD_H
#define ANDROID_VULKAN_GAMEPAD_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <cinttypes>
#include <mutex>
#include <thread>

GX_RESTORE_WARNING_STATE

#include "dpad.h"
#include "key_action.h"
#include "stick.h"
#include "trigger.h"


namespace android_vulkan {

enum class eGamepadKey : uint8_t
{
    A = 0U,
    B,
    X,
    Y,
    Down,
    Left,
    Right,
    Up,
    LeftStick,
    RightStick,
    LeftBumper,
    RightBumper,
    Home,
    Menu,
    View
};

constexpr auto const TOTAL_GAMEPAD_KEYS = static_cast<size_t> (
    static_cast<uint8_t> ( eGamepadKey::View ) + 1U
);

enum class eButtonState : uint8_t
{
    Down = 0U,
    Up
};

constexpr auto const TOTAL_BUTTON_STATES = static_cast<size_t> ( static_cast<uint8_t> ( eButtonState::Up ) + 1U );
constexpr size_t const ACTION_POOL_ELEMENT_COUNT = TOTAL_GAMEPAD_KEYS * TOTAL_BUTTON_STATES;

class Gamepad final
{
    private:
        struct KeyActionQueue final
        {
            KeyAction       _keyActionPool[ ACTION_POOL_ELEMENT_COUNT ] {};
            KeyAction*      _freeKeyActions = nullptr;
            KeyAction*      _readyKeyActions = nullptr;
        };

    private:
        KeyBind             _downKeyBinds[ TOTAL_GAMEPAD_KEYS ];
        KeyBind             _upKeyBinds[ TOTAL_GAMEPAD_KEYS ];

        DPad                _dPadCurrent;
        DPad                _dPadOld;
        bool                _loop;

        KeyActionQueue*     _queueRead;
        KeyActionQueue*     _queueWrite;
        KeyActionQueue      _queue0;
        KeyActionQueue      _queue1;

        Stick               _leftStick;
        Stick               _rightStick;

        Trigger             _leftTrigger;
        Trigger             _rightTrigger;

        std::mutex          _mutex;
        std::thread         _thread;

    public:
        static Gamepad& GetInstance () noexcept;

        Gamepad ( Gamepad const &other ) = delete;
        Gamepad& operator = ( Gamepad const &other ) = delete;

        Gamepad ( Gamepad &&other ) = delete;
        Gamepad& operator = ( Gamepad &&other ) = delete;

        void BindKey ( void* context, KeyHandler handler, eGamepadKey key, eButtonState state ) noexcept;
        void UnbindKey ( eGamepadKey key, eButtonState state ) noexcept;

        void BindLeftStick ( void* context, StickHandler handler ) noexcept;
        void UnbindLeftStick () noexcept;

        void BindRightStick ( void* context, StickHandler handler ) noexcept;
        void UnbindRightStick () noexcept;

        [[maybe_unused]] void BindLeftTrigger ( void* context, TriggerHandler handler ) noexcept;
        [[maybe_unused]] void UnbindLeftTrigger () noexcept;

        void BindRightTrigger ( void* context, TriggerHandler handler ) noexcept;
        void UnbindRightTrigger () noexcept;

        [[nodiscard]] int32_t OnOSInputEvent ( AInputEvent* event ) noexcept;

        void Start () noexcept;
        void Stop () noexcept;

    private:
        Gamepad () noexcept;
        ~Gamepad () = default;

        void AddAction ( KeyBind const &bind ) noexcept;

        void ExecuteKeyEvents () noexcept;
        void ExecuteStickEvents () noexcept;
        void ExecuteTriggerEvents () noexcept;

        void HandleDPad ( AInputEvent* event ) noexcept;

        [[nodiscard]] int32_t HandleKey ( AInputEvent* event ) noexcept;
        [[nodiscard]] int32_t HandleMotion ( AInputEvent* event ) noexcept;

        void HandleSticks ( AInputEvent* event ) noexcept;
        void HandleTriggers ( AInputEvent* event ) noexcept;

        void InitActionPool () noexcept;
        void SwapQueues () noexcept;
        void ResolveDPad () noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_GAMEPAD_H
