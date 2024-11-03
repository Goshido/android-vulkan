#ifndef EDITOR_KEYS_HPP
#define EDITOR_KEYS_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <cstdint>

GX_RESTORE_WARNING_STATE


namespace editor {

enum class eKey : uintptr_t
{
    LeftMouseButton,
    MiddleMouseButton,
    RightMouseButton
};

} // namespace editor


#endif // EDITOR_KEYS_HPP
