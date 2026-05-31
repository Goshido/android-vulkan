#ifndef GEOMETRY_PASS_MATERIAL_HLSL
#define GEOMETRY_PASS_MATERIAL_HLSL


#include "platform/android/pbr/geometry_pass_binds.inc"


[[vk::binding ( BIND_SAMPLER, SET_SAMPLER )]]
SamplerState                        g_sampler:              register ( s0 );

[[vk::binding ( BIND_DIFFUSE_TEXTURE, SET_MATERIAL )]]
Texture2D<float32_t4>               g_diffuseTexture:       register ( t0 );

[[vk::binding ( BIND_EMISSION_TEXTURE, SET_MATERIAL )]]
Texture2D<float32_t4>               g_emissionTexture:      register ( t1 );

[[vk::binding ( BIND_MASK_TEXTURE, SET_MATERIAL )]]
Texture2D<float32_t4>               g_maskTexture:          register ( t2 );

[[vk::binding ( BIND_NORMAL_TEXTURE, SET_MATERIAL )]]
Texture2D<float32_t4>               g_normalTexture:        register ( t3 );

[[vk::binding ( BIND_PARAMS_TEXTURE, SET_MATERIAL )]]
Texture2D<float32_t4>               g_paramTexture:         register ( t4 );


#endif // GEOMETRY_PASS_MATERIAL_HLSL
