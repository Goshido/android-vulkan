#ifndef EDITOR_KEYS_HPP
#define EDITOR_KEYS_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <cstdint>

GX_RESTORE_WARNING_STATE


namespace editor {

enum class eKey : uint16_t
{
    LeftMouseButton,
    MiddleMouseButton,
    RightMouseButton,

    Key0,
    Key1,
    Key2,
    Key3,
    Key4,
    Key5,
    Key6,
    Key7,
    Key8,
    Key9,

    KeyF1,
    KeyF2,
    KeyF3,
    KeyF4,
    KeyF5,
    KeyF6,
    KeyF7,
    KeyF8,
    KeyF9,
    KeyF10,
    KeyF11,
    KeyF12,

    KeyA,
    KeyB,
    KeyC,
    KeyD,
    KeyE,
    KeyF,
    KeyG,
    KeyH,
    KeyI,
    KeyJ,
    KeyK,
    KeyL,
    KeyM,
    KeyN,
    KeyO,
    KeyP,
    KeyQ,
    KeyR,
    KeyS,
    KeyT,
    KeyU,
    KeyV,
    KeyW,
    KeyX,
    KeyY,
    KeyZ,

    KeyDown,
    KeyLeft,
    KeyRight,
    KeyUp,

    KeyLeftSquareBracket,
    KeyRightSquareBracket,

    KeyNumpad0,
    KeyNumpad1,
    KeyNumpad2,
    KeyNumpad3,
    KeyNumpad4,
    KeyNumpad5,
    KeyNumpad6,
    KeyNumpad7,
    KeyNumpad8,
    KeyNumpad9,
    KeyNumpadAdd,
    KeyNumpadDiv,
    KeyNumpadDot,
    KeyNumpadMinus,
    KeyNumpadMul,

    KeyAlt,
    KeyApostrophe,
    KeyBackslash,
    KeyBackspace,
    KeyCapsLock,
    KeyComma,
    KeyCtrl,
    KeyDel,
    KeyEnd,
    KeyEnter,
    KeyEsc,
    KeyHome,
    KeyIns,
    KeyMenu,
    KeyMinus,
    KeyPause,
    KeyPeriod,
    KeyPgDown,
    KeyPgUp,
    KeyPlus,
    KeySemicolon,
    KeyShift,
    KeySlash,
    KeySpace,
    KeyTab,
    KeyTilde
};

class KeyModifier final
{
    public:
        bool    _leftAlt : 1U = false;
        bool    _rightAlt : 1U = false;

        bool    _leftCtrl : 1U = false;
        bool    _rightCtrl : 1U = false;

        bool    _leftShift : 1U = false;
        bool    _rightShift : 1U = false;

    public:
        explicit KeyModifier () = default;

        KeyModifier ( KeyModifier const & ) = default;
        KeyModifier &operator = ( KeyModifier const & ) = default;

        KeyModifier ( KeyModifier && ) = default;
        KeyModifier &operator = ( KeyModifier && ) = default;

        ~KeyModifier () = default;

        [[nodiscard]] bool AnyAltPressed () const noexcept;
        [[nodiscard]] bool AnyCtrlPressed () const noexcept;
        [[nodiscard]] bool AnyShiftPressed () const noexcept;
};

} // namespace editor


#endif // EDITOR_KEYS_HPP
