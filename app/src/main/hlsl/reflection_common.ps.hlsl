#ifndef REFLECTION_COMMON_PS_HLSL
#define REFLECTION_COMMON_PS_HLSL


#include "lightup_common.ps.hlsl"


#define PREFILTER_MAX_MIP_LEVEL     8.0H


struct InputData
{
    linear float32_t4       _pixel:         SV_Position;
};

//----------------------------------------------------------------------------------------------------------------------

float32_t4 RestoreLocationViewUnnormalized ( in float32_t2 locationImage, in float32_t depth )
{
    float32_t2 const alpha = mad ( locationImage, _invResolutionFactor, (float32_t2)( -1.0F ) );
    return mul ( _cvvToView, float32_t4 ( alpha, depth, 1.0F ) );
}

float32_t Roughness2Mip ( in float16_t roughness )
{
    // It's empirical ease function. Please plot the graph to see what happens.
    float16_t const mip = 8.42H - 8.41H / ( 1.0H + pow ( roughness / 0.2584H, 2.164H ) );
    return (float32_t)clamp ( mip, 0.0H, PREFILTER_MAX_MIP_LEVEL );
}

float16_t3 GetReflection ( in float16_t3 toFragmentView, in TextureCube<float32_t4> prefilter )
{
    float16_t roughness;
    float16_t metallic;
    GetParams ( roughness, metallic );

    float16_t3 const directionView = -toFragmentView;
    float16_t3 const normalView = RestoreNormalView ();
    float16_t const dN = max ( 0.0H, dot ( directionView, normalView ) );
    float16_t3 const fresnel = GetFresnel ( dN, metallic, GetAlbedo () );

    float32_t3 const refl = -mul ( (float32_t3x3)_viewToWorld, (float32_t3)( reflect ( directionView, normalView ) ) );
    float32_t const mip = Roughness2Mip ( roughness );

    float16_t3 const prefilterColor = (float16_t3)prefilter.SampleLevel ( g_prefilterSampler, refl, mip ).xyz;

    float16_t2 const brdf = (float16_t2)g_brdf.Sample (
        g_brdfSampler, float32_t2 ( (float32_t)dN, 1.0F - roughness )
    ).xy;

    return prefilterColor * mad ( fresnel, (float16_t3)brdf.x, (float16_t3)brdf.y );
}


#endif // REFLECTION_COMMON_PS_HLSL
