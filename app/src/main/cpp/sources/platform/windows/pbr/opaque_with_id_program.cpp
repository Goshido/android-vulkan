#include <precompiled_headers.hpp>
#include <platform/windows/pbr/opaque_with_id_program.hpp>


namespace pbr {

OpaqueWithIDProgram::OpaqueWithIDProgram () noexcept:
    GBufferProgram ( "shaders/windows/gbuffer_mesh_with_id.vs.spv",
        "shaders/windows/opaque_with_id.ps.spv",
        "Opaque with ID",
        sizeof ( PushConstants )
    )
{
    // NOTHING
}

} // namespace pbr
