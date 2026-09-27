#ifndef TONE_MAPPER_COMMON_HLSL
#define TONE_MAPPER_COMMON_HLSL


#include "aces.hlsl"
#include "pbr/full_screen_triangle.inc"
#include "platform/windows/pbr/samplers.inc"
#include "windows/blur_kernel.hlsl"
#include "windows/tone_mapper_push_constants.hlsl"


// Note the color is out of LDR range intentionally. Otherwise the outline will be too dim.
#define OUTLINE_COLOR       float16_t3 ( 1.66164H, 1.43148H, 0.00954H )


struct InputData
{
    [[vk::location ( ATT_SLOT_UV )]]
    noperspective float32_t2    _uv:    UV;
};

//----------------------------------------------------------------------------------------------------------------------

float16_t BlurY ( in float32_t2 uv )
{
    Texture2D<float32_t4> outlineBlurX = ResourceDescriptorHeap[ g_pushConstants._outlineBlurX ];
    SamplerState pointSampler = SamplerDescriptorHeap[ POINT_SAMPLER ];

    float16_t2 const up = (float16_t2)outlineBlurX.GatherRed ( pointSampler, uv + g_pushConstants._halfPixelMove ).xw;
    uint32_t2 const pix = (uint32_t2)( uv * g_pushConstants._resolution );
    float16_t const right = pix.y == 0U ? up.y : (float16_t)outlineBlurX [ uint32_t2 ( pix.x, pix.y - 1U ) ].x;
    return dot ( float16_t3 ( up, right ), BLUR_KERNEL );
}

float16_t3 Execute ( in float32_t2 uv )
{
    Texture2D<float32_t4> hdrImage = ResourceDescriptorHeap[ g_pushConstants._hdrImage ];
    SamplerState clampToEdgeSampler = SamplerDescriptorHeap[ CLAMP_TO_EDGE_SAMPLER ];
    StructuredBuffer<float32_t> exposure = ResourceDescriptorHeap[ g_pushConstants._exposure ];
    float16_t3 const result = ApplyACES ( hdrImage, clampToEdgeSampler, uv, exposure[ 0U ] );

    if ( g_pushConstants._outlineBlurX == 0U )
        return result;

    float16_t const a = BlurY ( uv );
    return min ( (float16_t3)1.0H, mad ( OUTLINE_COLOR, a, result * ( 1.0H - a ) ) );
}


#endif // TONE_MAPPER_COMMON_HLSL
