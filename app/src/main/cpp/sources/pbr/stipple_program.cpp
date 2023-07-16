#include <pbr/stipple_program.hpp>


namespace pbr {

StippleProgram::StippleProgram () noexcept:
    GeometryPassProgram ( "pbr::StippleProgram", "shaders/stipple.ps.spv" )
{
    // NOTHING
}

} // namespace pbr
