#include "exposure.hlsl"
#include "platform/android/pbr/exposure.inc"


struct PushConstants
{
    float32_t                                       _exposureCompensation;
    float32_t                                       _eyeAdaptation;
    float32_t                                       _maxLuma;
    float32_t                                       _minLuma;
};

[[vk::push_constant]]
PushConstants                                       g_exposureInfo;

[[vk::binding ( BIND_HDR_IMAGE, SET_RESOURCE )]]
Texture2D<float32_t4>                               g_hdrImage:         register ( t0 );

[[vk::binding ( BIND_SYNC_MIP_5, SET_RESOURCE )]]
[[vk::image_format ( "r16f" )]]
globallycoherent RWTexture2D<float32_t>             g_syncMip5:         register ( u0 );

[[vk::binding ( BIND_EXPOSURE, SET_RESOURCE )]]
RWStructuredBuffer<float32_t>                       g_exposure:         register ( u1 );

[[vk::binding ( BIND_GLOBAL_ATOMIC, SET_RESOURCE )]]
globallycoherent RWStructuredBuffer<uint32_t>       g_globalAtomic:     register ( u2 );

[[vk::binding ( BIND_TEMPORAL_LUMA, SET_RESOURCE )]]
RWStructuredBuffer<float32_t>                       g_temporalLuma:     register ( u3 );

//----------------------------------------------------------------------------------------------------------------------

[numthreads ( THREAD_X, THREAD_Y, THREAD_Z )]
void CS ( in uint32_t threadID: SV_GroupIndex, in uint32_t3 workGroupID: SV_GroupID )
{
    Execute ( g_hdrImage,
        g_syncMip5,
        g_exposure,
        g_globalAtomic,
        g_temporalLuma,
        g_exposureInfo._exposureCompensation,
        g_exposureInfo._eyeAdaptation,
        g_exposureInfo._maxLuma,
        g_exposureInfo._minLuma,
        threadID,
        workGroupID
    );
}
