#include <pbr/stipple_program.h>


namespace pbr {

StippleProgram::StippleProgram () noexcept:
    GeometryPassProgram ( "StippleProgram", "shaders/stipple-fs.spv" )
{
    // NOTHING
}

} // namespace pbr
