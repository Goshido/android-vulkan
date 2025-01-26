#include "pbr/ui_program.inc"


[[vk::binding ( BIND_TRANSFORM, SET_TRANSFORM )]]
cbuffer Transform:                                  register ( b0 )
{
    float32_t2x2                    _rotateScale;
    float32_t2                      _offset;
};

struct InputData
{
    [[vk::location ( IN_SLOT_POSITION )]]
    float32_t2                      _position:      POSITION;

    [[vk::location ( IN_SLOT_IMAGE_UV )]]
    float32_t2                      _imageUV:       IMAGE_UV;

    [[vk::location ( IN_SLOT_ATLAS_UV )]]
    float32_t2                      _atlasUV:       ATLAS_UV;

    [[vk::location ( IN_SLOT_ATLAS_LAYER )]]
    float32_t                       _atlasLayer:    ATLAS_LAYER;

    [[vk::location ( IN_SLOT_COLOR )]]
    float32_t4                      _color:         COLOR;
};

struct OutputData
{
    linear float32_t4               _vertexH:       SV_Position;

    [[vk::location ( ATT_SLOT_IMAGE_UV )]]
    noperspective float32_t2        _imageUV:       IMAGE_UV;

    [[vk::location ( ATT_SLOT_ATLAS_UV )]]
    noperspective float32_t2        _atlasUV:       ATLAS_UV;

    [[vk::location ( ATT_SLOT_ATLAS_LAYER )]]
    nointerpolation float32_t       _atlasLayer:    ATLAS_LAYER;

    [[vk::location ( ATT_SLOT_COLOR )]]
    nointerpolation float32_t4      _color:         COLOR;
};

//----------------------------------------------------------------------------------------------------------------------

float32_t3 SRGBToLinear ( in float32_t3 sRGB )
{
    // See <repo>/docs/srgb#srgb-to-linear
    float16_t3 const c = (float16_t3)sRGB;
    float16_t3 const lin = c * 7.74e-2H;
    float16_t3 const ex = pow ( mad ( c, (float16_t3)9.479e-1H, (float16_t3)5.213e-2H ), 2.4H );
    return (float32_t3)lerp ( lin, ex, (float16_t3)( c > 4.045e-2H ) );
}

float32_t LinearToSRGB ( in float32_t linearColor )
{
    // See <repo>/docs/srgb#linear-to-srgb
    float16_t const c = (float16_t)linearColor;
    float16_t const srgb = c * 12.92H;
    float16_t const ex = mad ( 1.055H, pow ( c, 4.1667e-1H ), -5.5e-2H );
    return (float32_t)lerp ( srgb, ex, (float16_t)( c > 3.1308e-3H ) );
}

//----------------------------------------------------------------------------------------------------------------------

OutputData VS ( in InputData inputData )
{
    OutputData result;

    result._vertexH = float32_t4 ( mul ( _rotateScale, inputData._position + _offset ), 0.5F, 1.0F );
    result._imageUV = inputData._imageUV;
    result._atlasUV = inputData._atlasUV;

    // [2025/01/26] About alpha value: Mimicking for Google Chrome v132.0.6834.111. Need to investigate further...
    result._color = float32_t4 ( SRGBToLinear ( inputData._color.xyz ), LinearToSRGB ( inputData._color.w ) );

    result._atlasLayer = inputData._atlasLayer;

    return result;
}
