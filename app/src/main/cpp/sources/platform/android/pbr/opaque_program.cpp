#include <precompiled_headers.hpp>
#include <platform/android/pbr/opaque_program.hpp>


namespace pbr {

OpaqueProgram::OpaqueProgram () noexcept:
    GeometryPassProgram ( "Opaque", "shaders/android/opaque.ps.spv" )
{
    // NOTHING
}

} // namespace pbr
