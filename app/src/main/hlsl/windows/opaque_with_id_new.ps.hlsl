#include "windows/gbuffer_push_constants.hlsl"
#include "windows/gbuffer_render_targets.hlsl"
#include "windows/opaque.hlsl"


[[vk::push_constant]]
PushConstantsWithID     g_pushConstants;

//----------------------------------------------------------------------------------------------------------------------

OutputDataWithID PS ( in Attributes attributes )
{
    OutputDataWithID result;

    uint64_t const idOffset = (uint64_t)( attributes._instanceID * sizeof ( uint32_t2 ) );
    result._id = IDs ( (uint64_t)g_pushConstants._idStream + idOffset ).Get ();

    OpaqueResult r = Compute ( attributes, g_pushConstants._shadingStream );
    result._albedo = r._albedo;
    result._emission = r._emission;
    result._normal = r._normal;
    result._param = r._param;

    return result;
}
