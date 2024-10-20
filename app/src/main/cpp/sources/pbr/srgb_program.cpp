#include <pbr/srgb_constants.hpp>
#include <pbr/srgb_program.hpp>


namespace pbr {

SRGBProgram::GammaInfo SRGBProgram::GetGammaInfo ( float brightnessBalance ) noexcept
{
    return
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
