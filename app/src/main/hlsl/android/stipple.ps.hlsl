#include "android/gbuffer_render_targets.hlsl"
#include "android/geometry_pass_attributes.hlsl"
#include "android/geometry_pass_material.hlsl"
#include "android/object_data.hlsl"
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

//----------------------------------------------------------------------------------------------------------------------

OutputData PS ( in Attributes attributes )
{
    ColorData const colorData = g_colorData[ attributes._instanceIndex ];
    float16_t4 const diffuseSample = (float16_t4)g_diffuseTexture.Sample ( g_sampler, attributes._uv );
    uint32_t2 const bayerCoord = (uint32_t2)attributes._vertexH.xy % 4U;
    float16_t const alpha = ( (float16_t)colorData._col0A ) * UNORM_FACTOR_F16;

    // Applying stipple pattern...
    clip ( mad ( diffuseSample.w, alpha, g_bayerMatrix[ bayerCoord.x ][ bayerCoord.y ] ) );

    GBufferResult const r = FillGBuffer ( attributes._uv,
        attributes._tangentView,
        attributes._bitangentView,
        attributes._normalView,
        g_sampler,
        g_emissionTexture,
        g_maskTexture,
        g_normalTexture,
        g_paramTexture,
        g_colorData[ attributes._instanceIndex ],
        diffuseSample.xyz
    );

    OutputData result;
    result._albedo = r._albedo;
    result._emission = r._emission;
    result._normal = r._normal;
    result._param = r._param;

    return result;
}
