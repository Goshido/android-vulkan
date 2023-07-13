#ifndef PBR_SCRIPTABLE_GAMEPAD_HPP
#define PBR_SCRIPTABLE_GAMEPAD_HPP


#include <gamepad.hpp>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lstate.h>

} // extern "C"

#include <vector>

GX_RESTORE_WARNING_STATE


namespace pbr {

class ScriptableGamepad final
{
    private:
        enum class eType : lua_Integer
        {
            KeyDown = 0,
            KeyUp = 1,
            LeftStick = 2,
            RightStick = 3,
            LeftTrigger = 4,
            RightTrigger = 5,
            COUNT
        };

        struct Key final
        {
            android_vulkan::eGamepadKey     _key;
            android_vulkan::eButtonState    _state;
        };

        struct Stick final
        {
            float                           _x;
            float                           _y;
        };

        struct Trigger final
        {
            float                           _value;
        };

        struct Action final
        {
            eType                           _type;

            union
            {
                Key                         _key;
                Stick                       _stick;
                Trigger                     _trigger;
            };
        };

        struct KeyContext final
        {
            ScriptableGamepad*              _instance;
            android_vulkan::eGamepadKey     _key;
            android_vulkan::eButtonState    _state;
            eType                           _type;
        };

        struct AnalogContext final
        {
            ScriptableGamepad*              _instance;
            eType                           _type;
        };

        using ActionHandler = void ( ScriptableGamepad::* ) ( Action const &action,
            lua_State &vm,
            int sceneHandleIndex,
            int onInputIndex
        ) noexcept;

    private:
        static constexpr size_t             TOTAL_KEYS = android_vulkan::TOTAL_GAMEPAD_KEYS;
        static constexpr size_t             TOTAL_STATES = android_vulkan::TOTAL_GAMEPAD_KEY_STATES;
        static constexpr size_t             LEFT_INDEX = 0U;
        static constexpr size_t             RIGHT_INDEX = 1U;

        ActionHandler                       _actionHandlers[ static_cast<size_t> ( eType::COUNT ) ] {};

        KeyContext                          _keyContexts[ TOTAL_KEYS * TOTAL_STATES ] {};
        AnalogContext                       _stickContexts[ 2U ] {};
        AnalogContext                       _triggerContexts[ 2U ] {};

        std::mutex                          _mutex {};

        std::vector<Action>                 _readQueue {};
        std::vector<Action>                 _writeQueue {};

        int                                 _keyDownIndex = std::numeric_limits<int>::max ();
        int                                 _keyUpIndex = std::numeric_limits<int>::max ();

        int                                 _leftStickIndex = std::numeric_limits<int>::max ();
        int                                 _rightStickIndex = std::numeric_limits<int>::max ();

        int                                 _leftTriggerIndex = std::numeric_limits<int>::max ();
        int                                 _rightTriggerIndex = std::numeric_limits<int>::max ();

    public:
        ScriptableGamepad () noexcept;

        ScriptableGamepad ( ScriptableGamepad const & ) = delete;
        ScriptableGamepad &operator = ( ScriptableGamepad const & ) = delete;

        ScriptableGamepad ( ScriptableGamepad && ) = delete;
        ScriptableGamepad &operator = ( ScriptableGamepad && ) = delete;

        ~ScriptableGamepad () = default;

        void CaptureInput () noexcept;
        void ReleaseInput () const noexcept;

        [[nodiscard]] bool Execute ( lua_State &vm, int sceneHandleIndex, int onInputIndex ) noexcept;

        [[nodiscard]] bool Init ( lua_State &vm ) noexcept;
        void Destroy () noexcept;

    private:
        void KeyAction ( Action const &action, lua_State &vm, int sceneHandleIndex, int onInputIndex ) noexcept;
        void StickAction ( Action const &action, lua_State &vm, int sceneHandleIndex, int onInputIndex ) noexcept;
        void TriggerAction ( Action const &action, lua_State &vm, int sceneHandleIndex, int onInputIndex ) noexcept;

        [[nodiscard]] static bool AllocateKeyInputEvent ( int &eventIndex, lua_State &vm, eType type ) noexcept;
        [[nodiscard]] static bool AllocateStickInputEvent ( int &eventIndex, lua_State &vm, eType type ) noexcept;
        [[nodiscard]] static bool AllocateTriggerInputEvent ( int &eventIndex, lua_State &vm, eType type ) noexcept;

        static void OnKey ( void* context ) noexcept;
        static void OnStick ( void* context, float x, float y ) noexcept;
        static void OnTrigger ( void* context, float value ) noexcept;
};

} // namespace pbr


#endif // PBR_SCRIPTABLE_GAMEPAD_HPP
