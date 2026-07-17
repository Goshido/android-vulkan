#include "platform/windows/pbr/outline_border.inc"
#include "platform/windows/pbr/samplers.inc"


#define BORDER_THRESHOLD    0.1F


struct PushConstants
{
    uint32_t        _idMask;
    uint32_t        _outline;
    uint32_t2       _resolution;
    float32_t2      _invResolution;
    float32_t4      _halfPixelMove;
};

[[vk::push_constant]]
PushConstants       g_pushConstants;

//----------------------------------------------------------------------------------------------------------------------

float32_t ComputeBorder ( in uint32_t2 pix )
{
    Texture2D<float32_t4> idMask = ResourceDescriptorHeap[ g_pushConstants._idMask ];
    SamplerState pointSampler = SamplerDescriptorHeap[ POINT_SAMPLER ];

    float32_t2 const pivot = ( (float32_t2)pix + 0.5F ) * g_pushConstants._invResolution;
    float32_t4 const locations = pivot.xyxy + g_pushConstants._halfPixelMove;
    float32_t4 const up = idMask.GatherRed ( pointSampler, locations.xy );

    if ( up.y < BORDER_THRESHOLD )
        return 0.0F;

    if ( any ( abs ( up.yy - up.xz ) > BORDER_THRESHOLD ) )
        return 1.0F;

    return (float32_t)(
        any ( abs ( up.yy - idMask.GatherRed ( pointSampler, locations.zw ).xz ) > BORDER_THRESHOLD )
    );
}

//----------------------------------------------------------------------------------------------------------------------

[numthreads ( THREADS_X, THREADS_Y, 1 )]
void CS ( in uint32_t3 pix: SV_DispatchThreadID )
{
    if ( all ( pix.xy < g_pushConstants._resolution ) )
    {
        RWTexture2D<float32_t4> outline = ResourceDescriptorHeap[ g_pushConstants._outline ];
        outline[ pix.xy ] = float32_t4 ( ComputeBorder ( pix.xy ), 0.0F, 0.0F, 0.0F );
    }
}
