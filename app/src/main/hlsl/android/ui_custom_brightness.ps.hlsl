// FUCK - separate to windows and android

#include "brightness_factor.ps.hlsl"
#include "color_space.hlsl"
#include "ui_common.ps.hlsl"


//----------------------------------------------------------------------------------------------------------------------

float32_t4 PS ( in InputData inputData ): SV_Target0
{
    float16_t4 const srgb = Compute ( inputData );
    float16_t3 const adjustedLinearRGB = pow ( SRGBToLinear ( srgb.xyz ), (float16_t)g_brightnessFactor );
    return float32_t4 ( (float32_t3)LinearToSRGB ( adjustedLinearRGB ), (float32_t)srgb.w );
}
