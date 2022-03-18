#include <pbr/stipple_program.h>


namespace pbr {

StippleProgram::StippleProgram () noexcept:
    GeometryPassProgram ( "StippleProgram", "shaders/stipple-ps.spv" )
{
    // NOTHING
}

} // namespace pbr
