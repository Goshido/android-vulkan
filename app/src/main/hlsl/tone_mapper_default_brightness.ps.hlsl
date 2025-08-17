#include "color_space.hlsl"
#include "tone_mapper_common.hlsl"


//----------------------------------------------------------------------------------------------------------------------

float32_t4 PS ( in InputData inputData ): SV_Target0
{
    return float32_t4 ( (float32_t3)LinearToSRGB ( ApplyACES ( inputData._uv ) ), 1.0F );
}
