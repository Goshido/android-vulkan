#include "windows/opaque.hlsl"


[[vk::push_constant]]
PushConstants       g_pushConstants;

//----------------------------------------------------------------------------------------------------------------------

OutputData PS ( in Attributes attributes )
{
    return Compute ( attributes, g_pushConstants._shadingStream );
}
