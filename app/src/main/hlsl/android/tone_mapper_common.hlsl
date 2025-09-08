#ifndef TONE_MAPPER_COMMON_HLSL
#define TONE_MAPPER_COMMON_HLSL


#include "aces.hlsl"
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
};

struct InputData
{
    [[vk::location ( ATT_SLOT_UV )]]
    noperspective float32_t2    _uv:                        UV;
};

//----------------------------------------------------------------------------------------------------------------------

float16_t3 Execute ( in float32_t2 uv )
{
    return ApplyACES ( g_hdrImage, g_clampToEdgeSampler, uv, g_exposure );
}


#endif // TONE_MAPPER_COMMON_HLSL
