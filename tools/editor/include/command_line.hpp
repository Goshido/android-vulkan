#ifndef EDITOR_COMMAND_LINE_HPP
#define EDITOR_COMMAND_LINE_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <span>

GX_RESTORE_WARNING_STATE


namespace editor {

using CommandLine = std::span<char const* const>;

} // namespace editor


#endif // EDITOR_COMMAND_LINE_HPP
