#include "windows/gbuffer_push_constants.hlsl"
#include "windows/gbuffer_render_targets.hlsl"
#include "windows/opaque.hlsl"


[[vk::push_constant]]
PushConstantsWithID     g_pushConstants;

//----------------------------------------------------------------------------------------------------------------------

OutputDataWithID PS ( in Attributes attributes )
{
    OutputDataWithID result;

    result._id = vk::RawBufferLoad<uint32_t2> (
        g_pushConstants._idStream + attributes._instanceID * sizeof ( uint32_t2 ),
        8U
    );

    OpaqueResult r = Compute ( attributes, g_pushConstants._shadingStream );
    result._albedo = r._albedo;
    result._emission = r._emission;
    result._normal = r._normal;
    result._param = r._param;

    return result;
}
