#include "brightness_factor.ps.hlsl"
#include "tone_mapper_common.ps.hlsl"


//----------------------------------------------------------------------------------------------------------------------

float32_t4 PS ( in InputData inputData ): SV_Target0
{
    return float32_t4 ( pow ( ApplyACES ( inputData._uv ), (float16_t)g_brightnessFactor ), 1.0F );
}
