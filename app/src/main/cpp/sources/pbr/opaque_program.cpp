#include <pbr/opaque_program.hpp>


namespace pbr {

OpaqueProgram::OpaqueProgram () noexcept:
    GeometryPassProgram ( "Opaque", "shaders/opaque.ps.spv" )
{
    // NOTHING
}

} // namespace pbr
