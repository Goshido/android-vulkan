#ifndef EDITOR_KEYBOARD_KEY_EVENT_HPP
#define EDITOR_KEYBOARD_KEY_EVENT_HPP


#include "keys.hpp"
#include "message.hpp"


namespace editor {

class KeyboardKeyEvent final
{
    public:
        eKey            _key = eKey::KeyG;
        KeyModifier     _modifier {};

    public:
        KeyboardKeyEvent () = delete;

        KeyboardKeyEvent ( KeyboardKeyEvent const & ) = delete;
        KeyboardKeyEvent &operator = ( KeyboardKeyEvent const & ) = delete;

        KeyboardKeyEvent ( KeyboardKeyEvent && ) = delete;
        KeyboardKeyEvent &operator = ( KeyboardKeyEvent && ) = delete;

        explicit KeyboardKeyEvent ( eKey key ) noexcept;
        explicit KeyboardKeyEvent ( Message const &message ) noexcept;

        ~KeyboardKeyEvent () = default;

        [[nodiscard]] Message Create ( eMessageType messageType ) const noexcept;
};

} // namespace editor


#endif // EDITOR_KEYBOARD_KEY_EVENT_HPP
