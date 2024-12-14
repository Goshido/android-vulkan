#include "gbuffer_render_targets.ps.hlsl"


OutputData PS ()
{
    OutputData result;
    result._albedo = (float32_t4)0.0F;
    result._emission = (float32_t4)0.0F;
    result._normal = (float32_t4)0.5F;
    result._param = (float32_t4)0.0F;

    return result;
}
