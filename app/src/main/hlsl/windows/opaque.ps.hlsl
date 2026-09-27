#include "windows/gbuffer_push_constants.hlsl"
#include "windows/gbuffer_render_targets.hlsl"
#include "windows/opaque.hlsl"


[[vk::push_constant]]
PushConstants       g_pushConstants;

//----------------------------------------------------------------------------------------------------------------------

OutputData PS ( in Attributes attributes )
{
    OpaqueResult const r = Compute ( attributes, g_pushConstants._shadingStream );

    OutputData result;
    result._albedo = r._albedo;
    result._emission = r._emission;
    result._normal = r._normal;
    result._param = r._param;

    return result;
}
