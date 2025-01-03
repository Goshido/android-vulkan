#include "brightness_factor.ps.hlsl"
#include "ui_common.ps.hlsl"


//----------------------------------------------------------------------------------------------------------------------

float32_t4 PS ( in InputData inputData ): SV_Target0
{
    float16_t4 const color = Compute ( inputData );
    return float32_t4 ( pow ( color.xyz, (float16_t)g_brightnessFactor ), color.w );
}
