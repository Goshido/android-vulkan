#include "brightness_factor.hlsl"
#include "color_space.hlsl"
#include "tone_mapper_common.hlsl"


//----------------------------------------------------------------------------------------------------------------------

float32_t4 PS ( in InputData inputData ): SV_Target0
{
    float16_t3 const srgb = LinearToSRGB ( pow ( ApplyACES ( inputData._uv ), (float16_t)g_brightnessFactor ) );
    return float32_t4 ( (float32_t3)srgb, 1.0F );
}
