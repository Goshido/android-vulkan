#ifndef EDITOR_SDF_PALETTE_HPP
#define EDITOR_SDF_PALETTE_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <cstdint>

GX_RESTORE_WARNING_STATE


namespace editor {

enum class eSDFPalette : uint8_t
{
    Red = 0U,
    Green = 1U,
    Blue = 2U,
    White = 3U,
    Transparent = 4U,
    RedGhost = 5U,
    GreenGhost = 6U,
    BlueGhost = 7U,
    RedGlass = 8U,
    GreenGlass = 9U,
    BlueGlass = 10U,
    Grey = 11U,
    BlackGlass = 12U,
    Yellow = 13U,
    YellowGlass = 14U,
    Black = 15U
};

} // namespace editor


#endif // EDITOR_SDF_PALETTE_HPP
