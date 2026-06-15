#include "windows/opaque.hlsl"


[[vk::push_constant]]
PushConstantsWithID     g_pushConstants;

//----------------------------------------------------------------------------------------------------------------------

OutputData PS ( in Attributes attributes )
{
    RWTexture2D<uint32_t4> id = ResourceDescriptorHeap[ g_pushConstants._idImage ];

    id[ (uint32_t2)attributes._vertexH.xy ] = (uint32_t4)vk::RawBufferLoad<uint16_t4> (
        g_pushConstants._idStream + attributes._instanceID * sizeof ( uint16_t4 ),
        8U
    );

    return Compute ( attributes, g_pushConstants._shadingStream );
}
