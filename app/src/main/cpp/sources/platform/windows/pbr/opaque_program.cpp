#include <precompiled_headers.hpp>
#include <platform/windows/pbr/opaque_program.hpp>


namespace pbr {

OpaqueProgram::OpaqueProgram () noexcept:
    GBufferProgram ( "shaders/windows/gbuffer_mesh.vs.spv",
        "shaders/windows/opaque.ps.spv",
        "Opaque",
        sizeof ( PushConstants )
    )
{
    // NOTHING
}

} // namespace pbr
