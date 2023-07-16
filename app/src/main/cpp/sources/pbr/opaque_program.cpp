#include <pbr/opaque_program.hpp>


namespace pbr {

OpaqueProgram::OpaqueProgram () noexcept:
    GeometryPassProgram ( "pbr::OpaqueProgram", "shaders/opaque.ps.spv" )
{
    // NOTHING
}

} // namespace pbr
