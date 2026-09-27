#ifndef OUTLINE_BLUR_X_CS_HLSL
#define OUTLINE_BLUR_X_CS_HLSL


#include "platform/windows/pbr/outline_border.inc"
#include "platform/windows/pbr/samplers.inc"
#include "windows/blur_kernel.hlsl"


struct PushConstants
{
    uint32_t        _border;
    uint32_t        _blurX;
    uint32_t2       _resolution;
    float32_t2      _invResolution;
    float32_t2      _halfPixelMove;
};

[[vk::push_constant]]
PushConstants       g_pushConstants;

//----------------------------------------------------------------------------------------------------------------------

float32_t BlurX ( in uint32_t2 pix )
{
    Texture2D<float32_t4> border = ResourceDescriptorHeap[ g_pushConstants._border ];
    SamplerState pointSampler = SamplerDescriptorHeap[ POINT_SAMPLER ];

    float32_t2 const location = mad ( ( (float32_t2)pix + 0.5F ),
        g_pushConstants._invResolution,
        g_pushConstants._halfPixelMove
    );

    float16_t2 const up = (float16_t2)border.GatherRed ( pointSampler, location ).xy;

    uint32_t const p = pix.x + 1U;
    float16_t const right = p >= g_pushConstants._resolution.x ? up.y : (float16_t)border[ uint32_t2 ( p, pix.y ) ].x;
    return (float32_t)dot ( float16_t3 ( up, right ), BLUR_KERNEL );
}

//----------------------------------------------------------------------------------------------------------------------

[numthreads ( THREADS_X, THREADS_Y, 1 )]
void CS ( in uint32_t3 pix: SV_DispatchThreadID )
{
    if ( all ( pix.xy < g_pushConstants._resolution ) )
    {
        RWTexture2D<float32_t4> blurX = ResourceDescriptorHeap[ g_pushConstants._blurX ];
        blurX[ pix.xy ] = float32_t4 ( BlurX ( pix.xy ), 0.0F, 0.0F, 0.0F );
    }
}


#endif // OUTLINE_BLUR_X_CS_HLSL
