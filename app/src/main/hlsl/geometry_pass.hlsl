#ifndef GEOMETRY_PASS_HLSL
#define GEOMETRY_PASS_HLSL


#include "color_packing.hlsl"


// Intensity should take range from 0 to 6000.0H
// Intensity is packed as 24bit fixed point value.
// 6000.0F / (float32_t)( 0x00FFFFFFU )
#define INTENSITY_FACTOR    3.57628e-4F

//----------------------------------------------------------------------------------------------------------------------

struct GBufferResult
{
    float32_t4      _albedo;
    float32_t4      _emission;
    float32_t4      _normal;
    float32_t4      _param;
};

//----------------------------------------------------------------------------------------------------------------------

float32_t4 ComputeAlbedo ( in Texture2D<float32_t4> maskTexture,
    in SamplerState linearSampler,
    in float32_t2 uv,
    in ColorData colorData,
    in float16_t3 diffuseSample
)
{
    float16_t3 maskSample = (float16_t3)maskTexture.Sample ( linearSampler, uv ).xyz;
    float16_t const maskSum = maskSample.x + maskSample.y + maskSample.z;

    if ( maskSum > 1.0H )
        maskSample *= 1.0H / maskSum;

    float16_t3 albedoOverlay = (float16_t3)max ( 1.0H - maskSum, 0.0H );

    albedoOverlay = mad ( UnpackColorF16x3 ( colorData._col0R, colorData._col0G, colorData._col0B ),
        maskSample.x,
        albedoOverlay
    );

    albedoOverlay = mad ( UnpackColorF16x3 ( colorData._col1R, colorData._col1G, colorData._col1B ),
        maskSample.y,
        albedoOverlay
    );

    albedoOverlay = mad ( UnpackColorF16x3 ( colorData._col2R, colorData._col2G, colorData._col2B ),
        maskSample.z,
        albedoOverlay
    );

    return float32_t4 ( (float32_t3)( albedoOverlay * diffuseSample ), 1.0F );
}

float32_t4 ComputeEmission ( in Texture2D<float32_t4> emissionTexture,
    in SamplerState linearSampler,
    in float32_t2 uv,
    in ColorData colorData
)
{
    float16_t3 const emissionSample = (float16_t3)emissionTexture.Sample ( linearSampler, uv ).xyz;
    float16_t3 const alpha = UnpackColorF16x3 ( colorData._emiR, colorData._emiG, colorData._emiB );
    float32_t const intensity = INTENSITY_FACTOR * (float32_t)colorData._emiIntens;
    return float32_t4 ( (float32_t3)( emissionSample * alpha * (float16_t)intensity ), 1.0F );
}

float32_t4 ComputeNormalView ( in Texture2D<float32_t4> normalTexture,
    in SamplerState linearSampler,
    in float32_t2 uv,
    in float32_t3 tangentView,
    in float32_t3 bitangentView,
    in float32_t3 normalView
)
{
    float16_t2 const normalData = mad ( (float16_t2)normalTexture.Sample ( linearSampler, uv ).xw, 2.0H, -1.0H );

    float16_t3x3 const tbnView = float16_t3x3 (
        (float16_t3)tangentView,
        (float16_t3)bitangentView,
        (float16_t3)normalView
    );

    // DISCLAIMER: You might guess it's a good idea for optimization to not perform normalize operation here.
    // In reality it's bad idea. In fact you will see noticeable artifacts with specular on the light-up pass.
    // And you will NOT see any performance benefits in terms of FPS on real game scenes.
    // Trust me. I tried (2021-01-27)...
    return float32_t4 ( mad ( normalize ( mul ( float16_t3 ( normalData.xy, 1.0H ), tbnView ) ), 0.5H, 0.5H ), 0.5F );
}

GBufferResult FillGBuffer ( in float32_t2 uv,
    in float32_t3 tangentView,
    in float32_t3 bitangentView,
    in float32_t3 normalView,
    in SamplerState linearSampler,
    in Texture2D<float32_t4> emissionTexture,
    in Texture2D<float32_t4> maskTexture,
    in Texture2D<float32_t4> normalTexture,
    in Texture2D<float32_t4> paramTexture,
    in ColorData colorData,
    in float16_t3 diffuseSample
)
{
    GBufferResult result;

    result._albedo = ComputeAlbedo ( maskTexture, linearSampler, uv, colorData, diffuseSample );
    result._emission = ComputeEmission ( emissionTexture, linearSampler, uv, colorData );
    result._param = paramTexture.Sample ( linearSampler, uv );

    result._normal = ComputeNormalView ( normalTexture,
        linearSampler,
        uv,
        tangentView,
        bitangentView,
        normalView
    );

    return result;
}


#endif // GEOMETRY_PASS_HLSL
