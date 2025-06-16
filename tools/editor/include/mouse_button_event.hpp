#ifndef EDITOR_MOUSE_BUTTON_EVENT_HPP
#define EDITOR_MOUSE_BUTTON_EVENT_HPP


#include "keys.hpp"


namespace editor {

struct MouseButtonEvent final
{
    int32_t     _x = 0;
    int32_t     _y = 0;
    eKey        _key = eKey::LeftMouseButton;
};

} // namespace editor


#endif // EDITOR_MOUSE_BUTTON_EVENT_HPP
