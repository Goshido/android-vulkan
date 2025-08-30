#ifndef UI_COMMON_HLSL
#define UI_COMMON_HLSL


#include "pbr/ui_primitive_type.inc"
#include "platform/windows/pbr/samplers.inc"
#include "ui_bindings.hlsl"
#include "ui_push_constants.hlsl"


struct InputData
{
    [[vk::location ( ATT_SLOT_UV )]]
    noperspective float32_t2        _uv:                    UV;

    [[vk::location ( ATT_SLOT_IMAGE )]]
    nointerpolation uint32_t        _image:                 IMAGE;

    [[vk::location ( ATT_SLOT_UI_PRIMITIVE_TYPE )]]
    nointerpolation uint32_t        _uiPrimitiveType:       UI_PRIMITIVE_TYPE;

    [[vk::location ( ATT_SLOT_COLOR )]]
    nointerpolation float32_t4      _color:                 COLOR;
};

//----------------------------------------------------------------------------------------------------------------------

float16_t4 HandleImage ( in float16_t4 color, in InputData inputData )
{
    Texture2D<float32_t4> img = ResourceDescriptorHeap[ NonUniformResourceIndex ( inputData._image ) ];
    SamplerState s = SamplerDescriptorHeap[ MATERIAL_SAMPLER ];
    return color * (float16_t4)img.SampleLevel ( s, inputData._uv, 0.0F );
}

float16_t4 HandleText ( in float16_t4 color, in InputData inputData )
{
    Texture2D<float32_t4> atlas = ResourceDescriptorHeap[ NonUniformResourceIndex ( inputData._image ) ];
    SamplerState pointSampler = SamplerDescriptorHeap[ POINT_SAMPLER ];
    float32_t const glyphLuma = atlas.SampleLevel ( pointSampler, inputData._uv, 0.0F ).x;

    // [2025/02/23] It turns out that this gives real frame time reduction benefit on mobile hardware.
    if ( glyphLuma == 0.0F )
        return 0.0H;

    float32_t2 const uv = float32_t2 ( glyphLuma, inputData._color.w );
    Texture2D<float32_t4> textLUT = ResourceDescriptorHeap[ g_uiInfo._textLUT ];
    SamplerState lutSampler = SamplerDescriptorHeap[ CLAMP_TO_EDGE_SAMPLER ];
    return float16_t4 ( color.xyz, (float16_t)textLUT.SampleLevel ( lutSampler, uv, 0.0F ).x );
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


#endif // UI_COMMON_HLSL
