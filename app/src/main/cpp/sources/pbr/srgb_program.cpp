#include <pbr/srgb_program.hpp>


namespace pbr {

namespace {

constexpr float GAMMA_MID_POINT = 2.4F;
constexpr float GAMMA_RANGE = 1.4F;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

SRGBProgram::GammaInfo SRGBProgram::GetGammaInfo ( float brightnessBalance ) noexcept
{
    return GammaInfo
    {
        ._inverseGamma = 1.0F / ( GAMMA_MID_POINT + GAMMA_RANGE * GXClampf ( brightnessBalance, -1.0F, 1.0F ) )
    };
}

SRGBProgram::SRGBProgram ( std::string_view name ) noexcept:
    GraphicsProgram ( name )
{
    // NOTHING
}

} // namespace pbr
