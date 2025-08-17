#include "pbr/reflection_local.inc"
#include "reflection_common.hlsl"


[[vk::binding ( BIND_PREFILTER_TEXTURE, SET_REFLECTION )]]
TextureCube<float32_t4>     g_prefilter:    register ( t1 );

[[vk::binding ( BIND_LIGHT_DATA, SET_REFLECTION )]]
cbuffer LightData:                          register ( b1 )
{
    float32_t3              _locationView;
    float32_t               _invSize;
};

//----------------------------------------------------------------------------------------------------------------------

float16_t GetAttenuation ( in float32_t3 fragmentView )
{
    float16_t const alpha = (float16_t)min ( 1.0F, _invSize * distance ( _locationView, fragmentView ) );
    return mad ( alpha, -alpha, 1.0H );
}

OutputData PS ( in linear float32_t4 pixel: SV_Position )
{
    OutputData result;

    float32_t4 const alpha = RestoreLocationViewUnnormalized ( pixel.xy, GetDepth () );
    float32_t3 const fragmentView = alpha.xyz * ( 1.0F / alpha.w );

    float16_t3 const reflection = GetReflection ( (float16_t3)normalize ( fragmentView ), g_prefilter );
    float16_t const attenuation = GetAttenuation ( fragmentView );

    result._color = float32_t4 ( (float32_t3)( reflection * attenuation ), 0.0F );
    return result;
}
