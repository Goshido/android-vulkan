#ifndef UI_COMMON_PS_HLSL
#define UI_COMMON_PS_HLSL


#include "pbr/ui_program.inc"


[[vk::binding ( BIND_ATLAS_SAMPLER, SET_COMMON )]]
SamplerState                        g_atlasSampler:     register ( s0 );

[[vk::binding ( BIND_ATLAS_TEXTURE, SET_COMMON )]]
Texture2DArray<float32_t>           g_atlasTexture:     register ( t0 );

[[vk::binding ( BIND_IMAGE_SAMPLER, SET_COMMON )]]
SamplerState                        g_imageSampler:     register ( s1 );

[[vk::binding ( BIND_IMAGE_TEXTURE, SET_IMAGE_TEXTURE )]]
Texture2D<float32_t4>               g_imageTexture:     register ( t1 );

struct InputData
{
    [[vk::location ( ATT_SLOT_IMAGE_UV )]]
    noperspective float32_t2        _imageUV:           IMAGE_UV;

    [[vk::location ( ATT_SLOT_ATLAS_UV )]]
    noperspective float32_t2        _atlasUV:           ATLAS_UV;

    [[vk::location ( ATT_SLOT_ATLAS_LAYER )]]
    nointerpolation float32_t       _atlasLayer:        ATLAS_LAYER;

    [[vk::location ( ATT_SLOT_COLOR )]]
    nointerpolation float32_t4      _color:             COLOR;
};

//----------------------------------------------------------------------------------------------------------------------

float16_t4 Compute ( in InputData inputData )
{
    float16_t4 const color = (float16_t4)inputData._color;

    float16_t4 const imagePart =
        color * (float16_t4)g_imageTexture.SampleLevel ( g_imageSampler, inputData._imageUV, 0.0F );

    float32_t3 const atlasCoordinate = float32_t3 ( inputData._atlasUV, inputData._atlasLayer );
    float16_t const atlasSample = (float16_t)g_atlasTexture.SampleLevel ( g_atlasSampler, atlasCoordinate, 0.0F ).x;
    float16_t4 const textPart = float16_t4 ( color.xyz, atlasSample * color.a );

    return lerp ( textPart, imagePart, (float16_t4)( atlasSample == 0.0F ) );
}


#endif // UI_COMMON_PS_HLSL
