#include "windows/gbuffer_push_constants.hlsl"
#include "windows/gbuffer_render_targets.hlsl"
#include "windows/opaque.hlsl"


[[vk::push_constant]]
PushConstantsWithID     g_pushConstants;

//----------------------------------------------------------------------------------------------------------------------

OutputDataWithID PS ( in Attributes attributes )
{
    OutputDataWithID result;
    RWTexture2D<uint32_t4> id = ResourceDescriptorHeap[ g_pushConstants._idImage ];

    result._id = (uint32_t4)vk::RawBufferLoad<uint16_t4> (
        g_pushConstants._idStream + attributes._instanceID * sizeof ( uint16_t4 ),
        8U
    );

    OpaqueResult r = Compute ( attributes, g_pushConstants._shadingStream );
    result._albedo = r._albedo;
    result._emission = r._emission;
    result._normal = r._normal;
    result._param = r._param;

    return result;
}
