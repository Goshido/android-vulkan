#include "platform/android/pbr/reflection_global.inc"
#include "reflection_common.hlsl"


#define VOID_SPACE_DEPTH    0.0F

[[vk::binding ( BIND_PREFILTER_TEXTURE, SET_REFLECTION )]]
TextureCube<float32_t4>     g_prefilter:    register ( t1 );

//----------------------------------------------------------------------------------------------------------------------

OutputData PS ( in linear float32_t4 pixel: SV_Position )
{
    float32_t const depth = GetDepth ();
    OutputData result;

    if ( depth == VOID_SPACE_DEPTH )
    {
        result._color = (float32_t4)0.0F;
        discard;
        return result;
    }

    float32_t4 const alpha = RestoreLocationViewUnnormalized ( pixel.xy, depth );
    result._color = float32_t4 ( (float32_t3)GetReflection ( (float16_t3)normalize ( alpha.xyz ), g_prefilter ), 0.0F );
    return result;
}
