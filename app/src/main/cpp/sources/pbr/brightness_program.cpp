#include <precompiled_headers.hpp>
#include <pbr/brightness_program.hpp>


namespace pbr {

namespace {

constexpr float BRIGHTNESS_BASE = 5.0F;
constexpr float BRIGHTNESS_TOLERANCE = 5.0e-2F;

} // end of anonymous namespace

BrightnessProgram::BrightnessInfo BrightnessProgram::GetBrightnessInfo ( float brightnessBalance ) noexcept
{
    float const b = -std::clamp ( brightnessBalance, -1.0F, 1.0F );

    return BrightnessInfo
    {
        ._brightnessFactor = std::pow ( BRIGHTNESS_BASE, b ),
        ._isDefaultBrightness = BRIGHTNESS_TOLERANCE > std::abs ( b )
    };
}

BrightnessProgram::BrightnessProgram ( std::string_view name ) noexcept:
    GraphicsProgram ( name )
{
    // NOTHING
}

} // namespace pbr
