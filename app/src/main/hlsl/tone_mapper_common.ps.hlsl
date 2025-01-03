#ifndef TONE_MAPPER_COMMON_PS_HLSL
#define TONE_MAPPER_COMMON_PS_HLSL


#include "pbr/full_screen_triangle.inc"
#include "pbr/tone_mapper.inc"


[[vk::binding ( BIND_HDR_IMAGE, SET_RESOURCE )]]
Texture2D<float32_t4>           g_hdrImage:                 register ( t0 );

[[vk::binding ( BIND_CLAMP_TO_EDGE_SAMPLER, SET_RESOURCE )]]
SamplerState                    g_clampToEdgeSampler:       register ( s0 );

[[vk::binding ( BIND_EXPOSURE, SET_RESOURCE )]]
cbuffer Exposure:                                           register ( b0 )
{
    float32_t                   g_exposure;
}

struct InputData
{
    [[vk::location ( ATT_SLOT_UV )]]
    noperspective float32_t2    _uv:                        UV;
};

//----------------------------------------------------------------------------------------------------------------------

float16_t3 ApplyACES ( in float32_t2 uv )
{
    // See https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
    float16_t3 const hdr = (float16_t3)g_hdrImage.SampleLevel ( g_clampToEdgeSampler, uv, 0.0F ).xyz;
    float16_t3 const raw = (float16_t)g_exposure * hdr;

    float16_t3 const alpha = mad ( (float16_t3)2.51H, raw, (float16_t3)0.03H );
    float16_t3 const beta = mad ( (float16_t3)2.43H, raw, (float16_t3)0.59H );
    return saturate ( ( raw * alpha ) / mad ( raw, beta, (float16_t3)0.14H ) );
}


#endif // TONE_MAPPER_COMMON_PS_HLSL
