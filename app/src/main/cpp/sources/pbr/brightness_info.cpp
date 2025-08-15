#include <precompiled_headers.hpp>
#include <pbr/brightness_info.hpp>


namespace pbr {

namespace {

constexpr float BRIGHTNESS_BASE = 5.0F;
constexpr float BRIGHTNESS_TOLERANCE = 5.0e-2F;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

BrightnessInfo::BrightnessInfo ( float brightnessBalance ) noexcept
{
    float const b = -std::clamp ( brightnessBalance, -1.0F, 1.0F );
    _brightnessFactor = std::pow ( BRIGHTNESS_BASE, b );
    _isDefaultBrightness = BRIGHTNESS_TOLERANCE > std::abs ( b );
}

} // namespace pbr
