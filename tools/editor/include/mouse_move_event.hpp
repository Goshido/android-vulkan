#ifndef EDITOR_MOUSE_MOVE_EVENT_HPP
#define EDITOR_MOUSE_MOVE_EVENT_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <cstdint>

GX_RESTORE_WARNING_STATE


namespace editor {

struct MouseMoveEvent final
{
    int32_t     _x = 0;
    int32_t     _y = 0;
    size_t      _eventID = 0U;
};

} // namespace editor


#endif // EDITOR_MOUSE_MOVE_EVENT_HPP
