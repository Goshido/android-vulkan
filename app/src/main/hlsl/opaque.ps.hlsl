#include "geometry_pass.ps.hlsl"


OutputData PS ( in InputData inputData )
{
    return FillGBuffer ( inputData,
        g_colorData[ inputData._instanceIndex ],
        (float16_t3)g_diffuseTexture.Sample ( g_sampler, inputData._uv ).xyz
    );
}
