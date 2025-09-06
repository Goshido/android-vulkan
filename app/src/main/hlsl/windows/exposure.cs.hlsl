#include "exposure.hlsl"


struct PushConstants
{
    uint32_t        _hdrImage;
    uint32_t        _syncMip5;
    uint32_t        _exposure;
    uint32_t        _globalAtomic;
    uint32_t        _temporalLuma;

    float32_t       _exposureCompensation;
    float32_t       _eyeAdaptation;
    float32_t       _maxLuma;
    float32_t       _minLuma;
};

[[vk::push_constant]]
PushConstants       g_exposureInfo;

//----------------------------------------------------------------------------------------------------------------------

[numthreads ( THREAD_X, THREAD_Y, THREAD_Z )]
void CS ( in uint32_t threadID: SV_GroupIndex, in uint32_t3 workGroupID: SV_GroupID )
{
    Texture2D<float32_t4> hdrImage = ResourceDescriptorHeap[ g_exposureInfo._hdrImage ];
    globallycoherent RWTexture2D<float32_t> syncMip5 = ResourceDescriptorHeap[ g_exposureInfo._syncMip5 ];

    RWStructuredBuffer<float32_t> exposure = ResourceDescriptorHeap[ g_exposureInfo._exposure ];
    globallycoherent RWStructuredBuffer<uint32_t> globalAtomic = ResourceDescriptorHeap[ g_exposureInfo._globalAtomic ];
    RWStructuredBuffer<float32_t> temporalLuma = ResourceDescriptorHeap[ g_exposureInfo._temporalLuma ];

    Execute ( hdrImage,
        syncMip5,
        exposure,
        globalAtomic,
        temporalLuma,
        g_exposureInfo._exposureCompensation,
        g_exposureInfo._eyeAdaptation,
        g_exposureInfo._maxLuma,
        g_exposureInfo._minLuma,
        threadID,
        workGroupID
    );
}
