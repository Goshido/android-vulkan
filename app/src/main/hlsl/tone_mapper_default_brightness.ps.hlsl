#include "tone_mapper_common.ps.hlsl"


//----------------------------------------------------------------------------------------------------------------------

float32_t4 PS ( in InputData inputData ): SV_Target0
{
    return float32_t4 ( ApplyACES ( inputData._uv ), 1.0F );
}
