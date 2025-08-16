#ifndef GEOMETRY_PASS_PS_HLSL
#define GEOMETRY_PASS_PS_HLSL


#include "color_packing.hlsl"
#include "gbuffer_render_targets.ps.hlsl"
#include "object_data.hlsl"


// Intensity should take range from 0 to 6000.0H
// Intensity is packed as 24bit fixed point value.
// 6000.0F / (float32_t)( 0x00FFFFFFU )
#define INTENSITY_FACTOR    3.57628e-4F


[[vk::binding ( BIND_SAMPLER, SET_SAMPLER )]]
SamplerState                        g_sampler:              register ( s0 );

[[vk::binding ( BIND_DIFFUSE_TEXTURE, SET_MATERIAL )]]
Texture2D<float32_t4>               g_diffuseTexture:       register ( t0 );

[[vk::binding ( BIND_EMISSION_TEXTURE, SET_MATERIAL )]]
Texture2D<float32_t4>               g_emissionTexture:      register ( t1 );

[[vk::binding ( BIND_MASK_TEXTURE, SET_MATERIAL )]]
Texture2D<float32_t4>               g_maskTexture:          register ( t2 );

[[vk::binding ( BIND_NORMAL_TEXTURE, SET_MATERIAL )]]
Texture2D<float32_t4>               g_normalTexture:        register ( t3 );

[[vk::binding ( BIND_PARAMS_TEXTURE, SET_MATERIAL )]]
Texture2D<float32_t4>               g_paramTexture:         register ( t4 );

struct InputData
{
    [[vk::location ( ATT_SLOT_UV )]]
    linear float32_t2               _uv:                    UV;

    [[vk::location ( ATT_SLOT_TANGENT_VIEW )]]
    linear float32_t3               _tangentView:           TANGENT;

    [[vk::location ( ATT_SLOT_BITANGENT_VIEW )]]
    linear float32_t3               _bitangentView:         BITANGENT;

    [[vk::location ( ATT_SLOT_NORMAL_VIEW )]]
    linear float32_t3               _normalView:            NORMAL;

    [[vk::location ( ATT_SLOT_INSTANCE_INDEX )]]
    nointerpolation uint32_t        _instanceIndex:         INSTANCE_INDEX;

    linear float4                   _vertexH:               SV_Position;
};

//----------------------------------------------------------------------------------------------------------------------

float32_t4 ComputeAlbedo ( in float32_t2 uv, in ColorData colorData, in float16_t3 diffuseSample )
{
    float16_t3 maskSample = (float16_t3)g_maskTexture.Sample ( g_sampler, uv ).xyz;
    float16_t const maskSum = maskSample.x + maskSample.y + maskSample.z;

    if ( maskSum > 1.0H )
        maskSample *= 1.0H / maskSum;

    float16_t3 albedoOverlay = (float16_t3)max ( 1.0H - maskSum, 0.0H );
    albedoOverlay = mad ( UnpackColorF16x3 ( colorData._emiRcol0rgb ), maskSample.x, albedoOverlay );
    albedoOverlay = mad ( UnpackColorF16x3 ( colorData._emiGcol1rgb ), maskSample.y, albedoOverlay );
    albedoOverlay = mad ( UnpackColorF16x3 ( colorData._emiBcol2rgb ), maskSample.z, albedoOverlay );

    return float32_t4 ( (float32_t3)( albedoOverlay * diffuseSample ), 1.0F );
}

float32_t4 ComputeEmission ( in float32_t2 uv, in ColorData colorData )
{
    float16_t3 const emissionSample = (float16_t3)g_emissionTexture.Sample ( g_sampler, uv ).xyz;

    uint32_t3 const emission = uint32_t3 ( colorData._emiRcol0rgb, colorData._emiGcol1rgb, colorData._emiBcol2rgb ) &
        0x000000FFU;

    float16_t3 const alpha = (float16_t3)emission * UNORM_FACTOR_F16;
    float32_t const intensity = INTENSITY_FACTOR * (float32_t)( colorData._col0aEmiIntens >> 8U );
    return float32_t4 ( (float32_t3)( emissionSample * alpha * (float16_t)intensity ), 1.0F );
}

float32_t4 ComputeNormalView ( in float32_t2 uv,
    in float32_t3 tangentView,
    in float32_t3 bitangentView,
    in float32_t3 normalView
)
{
    float16_t2 const normalData = mad ( (float16_t2)g_normalTexture.Sample ( g_sampler, uv ).xw, 2.0H, -1.0H );

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

OutputData FillGBuffer ( in InputData inputData, in ColorData colorData, in float16_t3 diffuseSample )
{
    OutputData result;

    result._albedo = ComputeAlbedo ( inputData._uv, colorData, diffuseSample );
    result._emission = ComputeEmission ( inputData._uv, colorData );
    result._param = g_paramTexture.Sample ( g_sampler, inputData._uv );

    result._normal = ComputeNormalView ( inputData._uv,
        inputData._tangentView,
        inputData._bitangentView,
        inputData._normalView
    );

    return result;
}


#endif // GEOMETRY_PASS_PS_HLSL
