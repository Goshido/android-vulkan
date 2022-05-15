#ifndef PBR_SCRIPTABLE_GAMEPAD_H
#define PBR_SCRIPTABLE_GAMEPAD_H


#include <gamepad.h>


namespace pbr {

class ScriptableGamepad final
{
    private:
        struct KeyContext final
        {
            ScriptableGamepad*              _instance;
            android_vulkan::eGamepadKey     _key;
            android_vulkan::eButtonState    _state;
        };

    private:
        KeyContext                          _keyContexts[ android_vulkan::ACTION_POOL_ELEMENT_COUNT ];

    public:
        ScriptableGamepad () = default;

        ScriptableGamepad ( ScriptableGamepad const & ) = delete;
        ScriptableGamepad& operator = ( ScriptableGamepad const & ) = delete;

        ScriptableGamepad ( ScriptableGamepad && ) = delete;
        ScriptableGamepad& operator = ( ScriptableGamepad && ) = delete;

        ~ScriptableGamepad () = default;

        void CaptureInput () noexcept;
        void ReleaseInput () noexcept;

    private:
        static void OnKeyDown ( void* context ) noexcept;
        static void OnKeyUp ( void* context ) noexcept;
};

} // namespace pbr


#endif // PBR_SCRIPTABLE_GAMEPAD_H
