#ifndef TONE_MAPPER_COMMON_HLSL
#define TONE_MAPPER_COMMON_HLSL


#include "aces.hlsl"
#include "pbr/full_screen_triangle.inc"
#include "platform/windows/pbr/samplers.inc"
#include "tone_mapper_push_constants.hlsl"


struct InputData
{
    [[vk::location ( ATT_SLOT_UV )]]
    noperspective float32_t2    _uv:    UV;
};

//----------------------------------------------------------------------------------------------------------------------

float16_t3 Execute ( in float32_t2 uv )
{
    Texture2D<float32_t4> hdrImage = ResourceDescriptorHeap[ g_toneMapperInfo._hdrImage ];
    SamplerState clampToEdgeSampler = SamplerDescriptorHeap[ CLAMP_TO_EDGE_SAMPLER ];
    StructuredBuffer<float32_t> exposure = ResourceDescriptorHeap[ g_toneMapperInfo._exposure ];
    return ApplyACES ( hdrImage, clampToEdgeSampler, uv, exposure[ 0U ] );
}


#endif // TONE_MAPPER_COMMON_HLSL
