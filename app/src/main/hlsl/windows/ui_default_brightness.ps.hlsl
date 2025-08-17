#include "ui_common.hlsl"


//----------------------------------------------------------------------------------------------------------------------

float32_t4 PS ( in InputData inputData ): SV_Target0
{
    return float32_t4 ( Compute ( inputData ) );
}
