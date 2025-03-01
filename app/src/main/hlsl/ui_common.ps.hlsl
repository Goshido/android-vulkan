#ifndef UI_COMMON_PS_HLSL
#define UI_COMMON_PS_HLSL


#include "pbr/ui_primitive_type.inc"
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

float16_t ResolveTextAlpha ( in float16_t colorAlpha, in float16_t glyphLuma )
{
    // [2025/02/23] It turns out that this gives real frame time reduction benefit on mobile hardware.
    if ( glyphLuma == 0.0H )
        return 0.0H;

    float16_t2 const v1 = float16_t2 ( glyphLuma, colorAlpha );
    float16_t2 const v2 = v1 * v1;
    float16_t2 const v4 = v2 * v2;
    float16_t2 const v3 = v2 * v1;
    float16_t2 const v5 = v4 * v1;

    float16_t4 const l0 = float16_t4 ( v1.x, v2.x, v1.x, v3.x );
    float16_t4 const r0 = float16_t4 ( v1.y, v1.y, v2.y, v1.y );

    float16_t4 const l1 = float16_t4 ( v2.x, v1.x, v4.x, v3.x );
    float16_t4 const r1 = float16_t4 ( v2.y, v3.y, v1.y, v2.y );

    float16_t4 const w0 = l0 * r0;

    float16_t2 const l2 = float16_t2 ( v2.x, v1.x );
    float16_t2 const r2 = float16_t2 ( v3.y, v4.y );

    float16_t4 const w1 = l1 * r1;

    float16_t const d0 = dot ( float16_t4 ( v1.x, v1.y, v2.x, w0.x ),
        float16_t4 ( 0.5110411H, 0.3230329H, -0.1824622H, -0.6972448H )
    );

    float16_t2 const w2 = l2 * r2;

    float16_t const d1 = dot ( float16_t4 ( v2.y, v3.x, w0.y, w0.z ),
        float16_t4 ( -3.4005306H, 1.4717692H, 1.3813718H, 9.018358H )
    );

    float16_t const d2 = dot ( float16_t4 ( v3.y, v4.x, w0.w, w1.x ),
        float16_t4 ( 10.0230089H, -2.4570389H, 2.9943197H, -12.4283102H )
    );

    float16_t const d3 = dot ( float16_t4 ( w1.y, v4.y, v5.x, w1.z ),
        float16_t4 ( -7.8590004H, -11.1873564H, 1.5675098H, -3.1576838H )
    );

    float16_t const d4 = dot ( float16_t4 ( w1.w, w2.x, w2.y, v5.y ),
        float16_t4 ( 3.4968898H, 5.5737564H, 1.7995902H, 4.2639596H )
    );


    float16_t const f0 = d0 + d1;
    float16_t const f1 = d2 + d3;
    float16_t const f2 = f0 + d4;

    return f1 + f2;
}

float16_t4 HandleImage ( in float16_t4 color, in InputData inputData )
{
    return color * (float16_t4)g_imageTexture.SampleLevel ( g_imageSampler, inputData._imageUV, 0.0F );
}

float16_t4 HandleText ( in float16_t4 color, in InputData inputData )
{
    float32_t3 const atlasCoordinate = float32_t3 ( inputData._atlasUV, inputData._atlasLayer );
    float16_t const glyphLuma = (float16_t)( g_atlasTexture.SampleLevel ( g_atlasSampler, atlasCoordinate, 0.0F ).x );
    return float16_t4 ( color.xyz, ResolveTextAlpha ( color.a, glyphLuma ) );
}

float16_t4 Compute ( in InputData inputData )
{
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
