#ifndef ANDROID_VULKAN_GAMEPAD_H
#define ANDROID_VULKAN_GAMEPAD_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <cinttypes>
#include <shared_mutex>
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
        KeyBind                     _downKeyBinds[ TOTAL_GAMEPAD_KEYS ];
        KeyBind                     _upKeyBinds[ TOTAL_GAMEPAD_KEYS ];

        DPad                        _dPadCurrent;
        DPad                        _dPadOld;
        bool                        _loop;

        KeyAction                   _keyActionPool[ ACTION_POOL_ELEMENT_COUNT ];
        KeyAction*                  _freeKeyActions;
        KeyAction*                  _readyKeyActions;

        Stick                       _leftStick;
        Stick                       _rightStick;

        Trigger                     _leftTrigger;
        Trigger                     _rightTrigger;

        std::shared_timed_mutex     _mutex;
        std::thread                 _thread;

    public:
        static Gamepad& GetInstance ();

        Gamepad ( Gamepad const &other ) = delete;
        Gamepad& operator = ( Gamepad const &other ) = delete;

        Gamepad ( Gamepad &&other ) = delete;
        Gamepad& operator = ( Gamepad &&other ) = delete;

        [[maybe_unused]] void BindKey ( void* context, KeyHandler handler, eGamepadKey key, eButtonState state );
        [[maybe_unused]] void UnbindKey ( eGamepadKey key, eButtonState state );

        void BindLeftStick ( void* context, StickHandler handler );
        void UnbindLeftStick ();

        void BindRightStick ( void* context, StickHandler handler );
        void UnbindRightStick ();

        [[maybe_unused]] void BindLeftTrigger ( void* context, TriggerHandler handler );
        [[maybe_unused]] void UnbindLeftTrigger ();

        [[maybe_unused]] void BindRightTrigger ( void* context, TriggerHandler handler );
        [[maybe_unused]] void UnbindRightTrigger ();

        [[nodiscard]] int32_t OnOSInputEvent ( AInputEvent* event );

        void Start ();
        void Stop ();

    private:
        Gamepad ();
        ~Gamepad () = default;

        void AddAction ( KeyBind const &bind );

        void ExecuteKeyEvents ();
        void ExecuteStickEvents ();
        void ExecuteTriggerEvents ();

        void HandleDPad ( AInputEvent* event );

        [[nodiscard]] int32_t HandleKey ( AInputEvent* event );
        [[nodiscard]] int32_t HandleMotion ( AInputEvent* event );

        void HandleSticks ( AInputEvent* event );
        void HandleTriggers ( AInputEvent* event );

        void InitActionPool ();
        void ResolveDPad ();
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_GAMEPAD_H
