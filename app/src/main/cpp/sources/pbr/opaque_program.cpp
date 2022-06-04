#include <pbr/opaque_program.h>


namespace pbr {

OpaqueProgram::OpaqueProgram () noexcept:
    GeometryPassProgram ( "pbr::OpaqueProgram", "shaders/opaque-ps.spv" )
{
    // NOTHING
}

} // namespace pbr
