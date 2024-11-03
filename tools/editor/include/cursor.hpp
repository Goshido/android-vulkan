#ifndef EDITOR_CURSOR_HPP
#define EDITOR_CURSOR_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <cstdint>

GX_RESTORE_WARNING_STATE


namespace editor {

enum class eCursor : uintptr_t
{
    Arrow,
    Cross,
    IBeam,
    NorthEastSouthWest,
    NorthSouth,
    NorthWestSouthEast,
    WestEast
};

} // namespace editor


#endif // EDITOR_CURSOR_HPP
