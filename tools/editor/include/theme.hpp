#ifndef EDITOR_THEME_HPP
#define EDITOR_THEME_HPP


#include <pbr/color_value.hpp>
#include <pbr/length_value.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <string_view>

GX_RESTORE_WARNING_STATE


namespace editor::theme {

constexpr std::string_view BOLD_FONT_FAMILY = "pbr/assets/Props/experimental/world-1-1/ui/fonts/trebucbd.ttf";
constexpr std::string_view NORMAL_FONT_FAMILY = "pbr/assets/Props/experimental/world-1-1/ui/fonts/trebuc.ttf";

constexpr pbr::ColorValue BACKGROUND_COLOR ( 30U, 30U, 30U, 217U );
constexpr pbr::ColorValue BORDER_COLOR ( 7U, 7U, 7U, 255U );
constexpr pbr::ColorValue HEADER_COLOR ( 15U, 15U, 15U, 217U );
constexpr pbr::ColorValue HOVER_COLOR ( 152U, 241U, 7U, 255U );
constexpr pbr::ColorValue INHERIT_COLOR {};
constexpr pbr::ColorValue MAIN_COLOR ( 106U, 172U, 0U, 255U );
constexpr pbr::ColorValue PRESS_COLOR ( 233U, 7U, 7U, 255U );
constexpr pbr::ColorValue TEXT_COLOR_NORMAL ( 178U, 178U, 178U, 255U );
constexpr pbr::ColorValue TRANSPARENT_COLOR ( 0U, 0U, 0U, 0U );
constexpr pbr::ColorValue WIDGET_BACKGROUND_COLOR ( 0U, 0U, 0U, 51U );

constexpr pbr::LengthValue NORMAL_FONT_SIZE ( pbr::LengthValue::eType::PX, 14.0F );
constexpr pbr::LengthValue NORMAL_LINE_HEIGHT ( pbr::LengthValue::eType::Auto, 42.0F );

constexpr pbr::LengthValue HEADER_FONT_SIZE ( pbr::LengthValue::eType::PX, 14.0F );
constexpr pbr::LengthValue HEADER_VERTICAL_PADDING ( pbr::LengthValue::eType::PX, 6.0F );
constexpr pbr::LengthValue HEADER_HEIGHT ( pbr::LengthValue::eType::PX, 30.0F );

constexpr pbr::LengthValue MENU_ITEM_HEIGHT ( pbr::LengthValue::eType::PX, 20.0F );
constexpr pbr::LengthValue SMALL_BUTTON_HEIGHT ( pbr::LengthValue::eType::PX, 40.0F );

constexpr pbr::LengthValue AUTO_LENGTH ( pbr::LengthValue::eType::Auto, 42.0F );
constexpr pbr::LengthValue INHERIT_LENGTH ( pbr::LengthValue::eType::Inherit, 42.0F );
constexpr pbr::LengthValue ZERO_LENGTH ( pbr::LengthValue::eType::PX, 0.0F );

} // namespace editor::theme


#endif // EDITOR_THEME_HPP
