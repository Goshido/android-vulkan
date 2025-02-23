#ifndef UI_COMMON_PS_HLSL
#define UI_COMMON_PS_HLSL


#include "pbr/ui_primitive_type.inc"
#include "pbr/ui_program.inc"


[[vk::binding ( BIND_ATLAS_SAMPLER, SET_COMMON )]]
SamplerState                        g_atlasSampler:         register ( s0 );

[[vk::binding ( BIND_ATLAS_TEXTURE, SET_COMMON )]]
Texture2DArray<float32_t>           g_atlasTexture:         register ( t0 );

[[vk::binding ( BIND_TEXT_LUT_SAMPLER, SET_COMMON )]]
SamplerState                        g_textLUTSampler:       register ( s1 );

[[vk::binding ( BIND_TEXT_LUT_TEXTURE, SET_COMMON )]]
Texture2D<float32_t>                g_textLUTTexture:       register ( t1 );

[[vk::binding ( BIND_IMAGE_SAMPLER, SET_COMMON )]]
SamplerState                        g_imageSampler:         register ( s2 );

[[vk::binding ( BIND_IMAGE_TEXTURE, SET_IMAGE_TEXTURE )]]
Texture2D<float32_t4>               g_imageTexture:         register ( t2 );

struct InputData
{
    [[vk::location ( ATT_SLOT_IMAGE_UV )]]
    noperspective float32_t2        _imageUV:               IMAGE_UV;

    [[vk::location ( ATT_SLOT_ATLAS_UV )]]
    noperspective float32_t2        _atlasUV:               ATLAS_UV;

    [[vk::location ( ATT_SLOT_ATLAS_LAYER )]]
    nointerpolation float32_t       _atlasLayer:            ATLAS_LAYER;

    [[vk::location ( ATT_SLOT_UI_PRIMITIVE_TYPE )]]
    nointerpolation uint32_t        _uiPrimitiveType:       UI_PRIMITIVE_TYPE;

    [[vk::location ( ATT_SLOT_COLOR )]]
    nointerpolation float32_t4      _color:                 COLOR;
};

//----------------------------------------------------------------------------------------------------------------------

float16_t4 HandleImage ( in float16_t4 color, in InputData inputData )
{
    return color * (float16_t4)g_imageTexture.SampleLevel ( g_imageSampler, inputData._imageUV, 0.0F );
}

float16_t4 HandleText ( in float16_t4 color, in InputData inputData )
{
    float32_t3 const atlasCoordinate = float32_t3 ( inputData._atlasUV, inputData._atlasLayer );
    float32_t const glyphLuma = g_atlasTexture.SampleLevel ( g_atlasSampler, atlasCoordinate, 0.0F ).x;

    // [2025/02/23] It turns out that this gives real frame time reduction benefit on mobile hardware.
    if ( glyphLuma == 0.0F )
        return 0.0H;

    float32_t2 const uv = float32_t2 ( glyphLuma, inputData._color.w );
    return float16_t4 ( color.xyz, (float16_t)g_textLUTTexture.SampleLevel ( g_textLUTSampler, uv, 0.0F ).x );
}

float16_t4 Compute ( in InputData inputData )
{
    // See <repo>/docs/proper-ui.md
    float16_t4 const color = (float16_t4)inputData._color;

    switch ( inputData._uiPrimitiveType )
    {
        case PBR_UI_PRIMITIVE_TYPE_TEXT:
        return HandleText ( color, inputData );

        case PBR_UI_PRIMITIVE_TYPE_IMAGE:
        return HandleImage ( color, inputData );

        case PBR_UI_PRIMITIVE_TYPE_GEOMETRY:
        default:
        return color;
    }
}


#endif // UI_COMMON_PS_HLSL
