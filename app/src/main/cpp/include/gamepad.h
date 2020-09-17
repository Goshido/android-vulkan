#ifndef GAMEPAD_H
#define GAMEPAD_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <cinttypes>
#include <shared_mutex>
#include <thread>
#include <android/input.h>

GX_RESTORE_WARNING_STATE

#include <GXCommon/GXMath.h>


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
    RightBumper
};

constexpr auto const TOTAL_GAMEPAD_KEYS = static_cast<size_t> (
    static_cast<uint8_t> ( eGamepadKey::RightBumper ) + 1U
);

enum class eButtonState : uint8_t
{
    Down = 0U,
    Up
};

constexpr auto const TOTAL_BUTTON_STATES = static_cast<size_t> ( static_cast<uint8_t> ( eButtonState::Up ) + 1U );
constexpr size_t const ACTION_POOL_ELEMENT_COUNT = TOTAL_GAMEPAD_KEYS * TOTAL_BUTTON_STATES;

typedef void ( *KeyHandler ) ( void* context );
typedef void ( *StickHandler ) ( void* context, float horizontal, float vertical );
typedef void ( *TriggerHandler ) ( void* context, float push );

class Gamepad final
{
    private:
        class KeyBind final
        {
            public:
                void*               _context;
                KeyHandler          _handler;

            public:
                KeyBind ();

                KeyBind ( KeyBind const &other ) = default;
                KeyBind& operator = ( KeyBind const &other ) = default;

                KeyBind ( KeyBind &&other ) = default;
                KeyBind& operator = ( KeyBind &&other ) = default;

                ~KeyBind () = default;

                void Init ( void* context, KeyHandler handler );
                void Reset ();
        };

        class KeyAction final
        {
            public:
                KeyBind             _bind;
                KeyAction*          _next;

            public:
                KeyAction ();

                KeyAction ( KeyAction const &other ) = delete;
                KeyAction& operator = ( KeyAction const &other ) = delete;

                KeyAction ( KeyAction &&other ) = delete;
                KeyAction& operator = ( KeyAction &&other ) = delete;

                ~KeyAction () = default;
        };

        struct Stick final
        {
            bool                    _isEvent;
            GXVec2                  _state;
            void*                   _context;
            StickHandler            _handler;

            Stick ();

            Stick ( Stick const &other ) = delete;
            Stick& operator = ( Stick const &other ) = delete;

            Stick ( Stick &&other ) = delete;
            Stick& operator = ( Stick &&other ) = delete;

            ~Stick () = default;

            void Bind ( void* context, StickHandler handler );
            void Unbind ();

            void Execute ();
        };

        struct Trigger final
        {
            bool                    _isEvent;
            float                   _push;
            void*                   _context;
            TriggerHandler          _handler;

            Trigger ();

            Trigger ( Trigger const &other ) = delete;
            Trigger& operator = ( Trigger const &other ) = delete;

            Trigger ( Trigger &&other ) = delete;
            Trigger& operator = ( Trigger &&other ) = delete;

            ~Trigger () = default;

            void Bind ( void* context, TriggerHandler handler );
            void Unbind ();

            void Execute ();
        };

    private:
        KeyBind                     _downKeyBinds[ TOTAL_GAMEPAD_KEYS ];
        KeyBind                     _upKeyBinds[ TOTAL_GAMEPAD_KEYS ];

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

        [[maybe_unused]] void BindLeftStick ( void* context, StickHandler handler );
        [[maybe_unused]] void UnbindLeftStick ();

        [[maybe_unused]] void BindRightStick ( void* context, StickHandler handler );
        [[maybe_unused]] void UnbindRightStick ();

        [[maybe_unused]] void BindLeftTrigger ( void* context, TriggerHandler handler );
        [[maybe_unused]] void UnbindLeftTrigger ();

        [[maybe_unused]] void BindRightTrigger ( void* context, TriggerHandler handler );
        [[maybe_unused]] void UnbindRightTrigger ();

        int32_t OnOSInputEvent ( AInputEvent* event );

        void Start ();
        void Stop ();

    private:
        Gamepad ();
        ~Gamepad () = default;

        void AddAction ( KeyBind const &bind );

        void ExecuteKeyEvents ();
        void ExecuteStickEvents ();
        void ExecuteTriggerEvents ();

        int32_t HandleKey ( AInputEvent* event );

        void InitActionPool ();
};

} // namespace android_vulkan


#endif // GAMEPAD_H
