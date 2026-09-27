#include "windows/gbuffer_mesh.hlsl"


[[vk::push_constant]]
PushConstantsWithID     g_pushConstants;

//----------------------------------------------------------------------------------------------------------------------

Attributes VS ( in InputData inputData )
{
    return Compute ( inputData,
        g_pushConstants._frameStream,
        g_pushConstants._transformStream,
        g_pushConstants._indexStream,
        g_pushConstants._indexType,
        g_pushConstants._positionStream,
        g_pushConstants._restStream
    );
}
