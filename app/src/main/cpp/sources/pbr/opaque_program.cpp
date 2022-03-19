#include <pbr/opaque_program.h>


namespace pbr {

OpaqueProgram::OpaqueProgram () noexcept:
    GeometryPassProgram ( "OpaqueProgram", "shaders/opaque-ps.spv" )
{
    // NOTHING
}

} // namespace pbr
