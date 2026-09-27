#include <precompiled_headers.hpp>
#include <platform/android/pbr/stipple_program.hpp>


namespace pbr {

StippleProgram::StippleProgram () noexcept:
    GeometryPassProgram ( "Stipple", "shaders/android/stipple.ps.spv" )
{
    // NOTHING
}

} // namespace pbr
