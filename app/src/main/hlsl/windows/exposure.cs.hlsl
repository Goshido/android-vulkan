#include "exposure.hlsl"


// See https://github.com/microsoft/DirectXShaderCompiler/blob/main/docs/SPIR-V.rst#resourcedescriptorheaps-samplerdescriptorheaps
#define RESOURCE_DESCRIPTOR_HEAP_SET        0
#define RESOURCE_DESCRIPTOR_HEAP_BIND       0


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

// [2025/09/11] DXC has issue with 'globallycoherent' and 'ResourceDescriptorHeap'.
// So the workaround is used.
// See https://github.com/microsoft/DirectXShaderCompiler/issues/7740

[[vk::binding ( RESOURCE_DESCRIPTOR_HEAP_BIND, RESOURCE_DESCRIPTOR_HEAP_SET )]]
Texture2D<float32_t4>                               g_images[]:             register ( t0 );

[[vk::binding ( RESOURCE_DESCRIPTOR_HEAP_BIND, RESOURCE_DESCRIPTOR_HEAP_SET )]]
globallycoherent RWTexture2D<float32_t>             g_coherentImages[]:     register ( u0 );

[[vk::binding ( RESOURCE_DESCRIPTOR_HEAP_BIND, RESOURCE_DESCRIPTOR_HEAP_SET )]]
RWStructuredBuffer<float32_t>                       g_buffers[]:            register ( u0 );

[[vk::binding ( RESOURCE_DESCRIPTOR_HEAP_BIND, RESOURCE_DESCRIPTOR_HEAP_SET )]]
globallycoherent RWStructuredBuffer<uint32_t>       g_coherentBuffers[]:    register ( u0 );

//----------------------------------------------------------------------------------------------------------------------

[numthreads ( THREAD_X, THREAD_Y, THREAD_Z )]
void CS ( in uint32_t threadID: SV_GroupIndex, in uint32_t3 workGroupID: SV_GroupID )
{
    Execute ( g_images[ g_exposureInfo._hdrImage ],
        g_coherentImages[ g_exposureInfo._syncMip5 ],
        g_buffers[ g_exposureInfo._exposure ],
        g_coherentBuffers[ g_exposureInfo._globalAtomic ],
        g_buffers[ g_exposureInfo._temporalLuma ],
        g_exposureInfo._exposureCompensation,
        g_exposureInfo._eyeAdaptation,
        g_exposureInfo._maxLuma,
        g_exposureInfo._minLuma,
        threadID,
        workGroupID
    );
}
