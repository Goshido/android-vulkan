#ifndef ANDROID_VULKAN_GAMEPAD_HPP
#define ANDROID_VULKAN_GAMEPAD_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <mutex>
#include <unordered_map>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

enum class eGamepadKey : uint8_t
{
    A = 0U,
    B = 1U,
    X = 2U,
    Y = 3U,
    Down = 4U,
    Left = 5U,
    Right = 6U,
    Up = 7U,
    LeftStick = 8U,
    RightStick = 9U,
    LeftBumper = 10U,
    RightBumper = 11U,
    Home = 12U,
    Menu = 13U,
    View = 14U
};

constexpr inline static auto TOTAL_GAMEPAD_KEYS = static_cast<size_t> ( eGamepadKey::View ) + 1U;

enum class eButtonState : uint8_t
{
    Down = 0U,
    Up
};

constexpr inline static auto TOTAL_GAMEPAD_KEY_STATES = static_cast<size_t> ( eButtonState::Up ) + 1U;

typedef void ( *KeyHandler ) ( void* context ) noexcept;
typedef void ( *TriggerHandler ) ( void* context, float push ) noexcept;
typedef void ( *StickHandler ) ( void* context, float horizontal, float vertical ) noexcept;

class Gamepad final
{
    private:
        struct KeyBind final
        {
            void*                               _context;
            KeyHandler                          _handler;
        };

        struct Stick final
        {
            void*                               _context;
            StickHandler                        _handler;
        };

        struct Trigger final
        {
            void*                               _context;
            TriggerHandler                      _handler;
        };

    private:
        KeyBind                                 _downKeyBinds[ TOTAL_GAMEPAD_KEYS ] {};
        KeyBind                                 _upKeyBinds[ TOTAL_GAMEPAD_KEYS ] {};

        Stick                                   _leftStick {};
        Stick                                   _rightStick {};

        Trigger                                 _leftTrigger {};
        Trigger                                 _rightTrigger {};

        std::unordered_map<int32_t, size_t>     _mapper;
        std::mutex mutable                      _mutex {};

    public:
        [[nodiscard]] static Gamepad &GetInstance () noexcept;

        Gamepad ( Gamepad const & ) = delete;
        Gamepad &operator = ( Gamepad const & ) = delete;

        Gamepad ( Gamepad && ) = delete;
        Gamepad &operator = ( Gamepad && ) = delete;

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

        [[nodiscard]] bool OnKeyDown ( int32_t key ) const noexcept;
        [[nodiscard]] bool OnKeyUp ( int32_t key ) const noexcept;

        void OnLeftStick ( float x, float y ) const noexcept;
        void OnRightStick ( float x, float y ) const noexcept;

        void OnLeftTrigger ( float value ) const noexcept;
        void OnRightTrigger ( float value ) const noexcept;

    private:
        Gamepad () noexcept;
        ~Gamepad () = default;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_GAMEPAD_HPP
