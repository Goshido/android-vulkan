#ifndef EDITOR_THEME_HPP
#define EDITOR_THEME_HPP


#include <GXCommon/GXMath.hpp>
#include <pbr/length_value.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <string_view>

GX_RESTORE_WARNING_STATE


namespace editor::theme {

constexpr std::string_view BOLD_FONT_FAMILY = "pbr/assets/Props/experimental/world-1-1/ui/fonts/trebucbd.ttf";
constexpr std::string_view NORMAL_FONT_FAMILY = "pbr/assets/Props/experimental/world-1-1/ui/fonts/trebuc.ttf";

constexpr GXColorUNORM BACKGROUND_COLOR ( 30U, 30U, 30U, 217U );
constexpr GXColorUNORM HEADER_COLOR ( 15U, 15U, 15U, 217U );
constexpr GXColorUNORM TEXT_COLOR_NORMAL ( 178U, 178U, 178U, 255U );
constexpr GXColorUNORM TRANSPARENT_COLOR ( 0U, 0U, 0U, 0U );

constexpr pbr::LengthValue FONT_SIZE ( pbr::LengthValue::eType::PX, 12.0F );
constexpr pbr::LengthValue AUTO_LENGTH ( pbr::LengthValue::eType::Auto, 42.0F );
constexpr pbr::LengthValue HEADER_VERTICAL_PADDING ( pbr::LengthValue::eType::PX, 3.0F );
constexpr pbr::LengthValue ZERO_LENGTH ( pbr::LengthValue::eType::PX, 0.0F );

} // namespace editor::theme


#endif // EDITOR_THEME_HPP
