#ifndef LIGHTUP_COMMON_PS_HLSL
#define LIGHTUP_COMMON_PS_HLSL


#include "pbr/lightup_common.inc"


#define MINIMUM_F0              4.0e-2H

// See https://github.com/Goshido/android-vulkan/issues/74
#define MINIMUM_ROUGHNESS       1.0e-2H

[[vk::binding ( BIND_ALBEDO_TEXTURE, SET_COMMON )]]
[[vk::input_attachment_index ( 0 )]]
SubpassInput<float32_t4>    g_albedo;

[[vk::binding ( BIND_NORMAL_TEXTURE, SET_COMMON )]]
[[vk::input_attachment_index ( 1 )]]
SubpassInput<float32_t4>    g_normal;

[[vk::binding ( BIND_PARAMS_TEXTURE, SET_COMMON )]]
[[vk::input_attachment_index ( 2 )]]
SubpassInput<float32_t4>    g_params;

[[vk::binding ( BIND_DEPTH_TEXTURE, SET_COMMON )]]
[[vk::input_attachment_index ( 3 )]]
SubpassInput<float32_t>     g_depth;

[[vk::binding ( BIND_BRDF_TEXTURE, SET_COMMON )]]
Texture2D<float32_t4>       g_brdf:                 register ( t0 );

[[vk::binding ( BIND_BRDF_SAMPLER, SET_COMMON )]]
SamplerState                g_brdfSampler:          register ( s1 );

[[vk::binding ( BIND_PREFILTER_SAMPLER, SET_COMMON )]]
SamplerState                g_prefilterSampler:     register ( s2 );

[[vk::binding ( BIND_SHADOW_SAMPLER, SET_COMMON )]]
SamplerComparisonState      g_shadowSampler:        register ( s3 );

[[vk::binding ( BIND_VIEW_DATA, SET_COMMON )]]
cbuffer ViewData:                                   register ( b0 )
{
    float32_t4x4            _cvvToView;
    float32_t4x4            _viewToWorld;

    // Optimization:
    // must be equal: 2.0F / resolution.
    float32_t2              _invResolutionFactor;
    float32_t2              _padding0_0;
};

struct OutputData
{
    [[vk::location ( OUT_COLOR )]]
    float32_t4              _color:                 SV_Target0;
};

//----------------------------------------------------------------------------------------------------------------------

float16_t3 GetAlbedo ()
{
    return (float16_t3)g_albedo.SubpassLoad ().xyz;
}

float32_t GetDepth ()
{
    return g_depth.SubpassLoad ();
}

float16_t3 GetFresnel ( in float16_t dotFactor, in float16_t metallic, in float16_t3 albedo )
{
    float16_t3 const f0 = lerp ( (float16_t3)MINIMUM_F0, albedo, (float16_t3)metallic );
    return mad ( 1.0H - f0, pow ( max ( 0.0H, 1.0H - dotFactor ), 5.0H ), f0 );
}

void GetParams ( out float16_t roughness, out float16_t metallic )
{
    float32_t4 const paramSample = (float32_t4)g_params.SubpassLoad ();
    roughness = max ( MINIMUM_ROUGHNESS, (float16_t)paramSample.x );
    metallic = (float16_t)paramSample.z;
}

float16_t3 RestoreNormalView ()
{
    float16_t3 const alpha = (float16_t3)g_normal.SubpassLoad ().xyz;
    return mad ( alpha, (float16_t3)2.0H, (float16_t3)( -1.0H ) );
}


#endif // LIGHTUP_COMMON_PS_HLSL
