#ifndef EDITOR_GLYPHS_HPP
#define EDITOR_GLYPHS_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <string_view>

GX_RESTORE_WARNING_STATE


namespace editor::glyph {

constexpr std::string_view FONT_FAMILY = "../editor-assets/fonts/avicons.otf";

constexpr std::string_view CHECKBOX_CHECK = "\u002C";
constexpr std::string_view CHECKBOX_UNCHECK = "\u002D";
constexpr std::string_view CHECKBOX_MULTI = "\u002E";

constexpr std::string_view CLOSE_BUTTON_BACKGROUND = "\u0022";
constexpr std::string_view CLOSE_BUTTON_BORDER = "\u0023";
constexpr std::string_view CLOSE_BUTTON_CROSS = "\u0024";

constexpr std::string_view COMBOBOX_DOWN = "\u0042";
constexpr std::string_view COMBOBOX_UP = "\u0043";

} // namespace editor::glyph


#endif // EDITOR_GLYPHS_HPP
