#ifndef EDITOR_GLYPHS_HPP
#define EDITOR_GLYPHS_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <string_view>

GX_RESTORE_WARNING_STATE


namespace editor::glyph {

constexpr std::string_view FONT_FAMILY = "../editor-assets/fonts/avicons.otf";

constexpr std::string_view CHECKBOX_CHECK = ",";
constexpr std::string_view CHECKBOX_UNCHECK = "-";
constexpr std::string_view CHECKBOX_MULTI = ".";

constexpr std::string_view CLOSE_BUTTON_BACKGROUND = R"__(")__";
constexpr std::string_view CLOSE_BUTTON_BORDER = "#";
constexpr std::string_view CLOSE_BUTTON_CROSS = "$";

} // namespace editor::glyph


#endif // EDITOR_GLYPHS_HPP
