#include "geometry_pass.hlsl"


// The implementation is based on ideas from
// https://digitalrune.github.io/DigitalRune-Documentation/html/fa431d48-b457-4c70-a590-d44b0840ab1e.htm
// It is used modified Bayer matrix 4x4 which is used in HLSL::clip and HLSL::mad intrinsics
// from https://en.wikipedia.org/wiki/Ordered_dithering
// Basically it was added 1.0F to numerator and denominator and taking negative value.
static float16_t4x4 const g_bayerMatrix =
{
    -0.05882H, -0.52941H, -0.17647H, -0.64705H,
    -0.76470H, -0.29411H, -0.88235H, -0.41176H,
    -0.23529H, -0.70588H, -0.11764H, -0.58823H,
    -0.94117H, -0.47058H, -0.82352H, -0.35294H
};

OutputData PS ( in InputData inputData )
{
    ColorData const colorData = g_colorData[ inputData._instanceIndex ];
    float16_t4 const diffuseSample = (float16_t4)g_diffuseTexture.Sample ( g_sampler, inputData._uv );
    uint32_t2 const bayerCoord = (uint32_t2)inputData._vertexH.xy % 4U;
    float16_t const alpha = ( (float16_t)colorData._col0A ) * UNORM_FACTOR_F16;

    // Applying stipple pattern...
    clip ( mad ( diffuseSample.w, alpha, g_bayerMatrix[ bayerCoord.x ][ bayerCoord.y ] ) );

    return FillGBuffer ( inputData, colorData, diffuseSample.xyz );
}
