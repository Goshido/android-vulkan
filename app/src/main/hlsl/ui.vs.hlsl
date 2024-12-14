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

float32_t4 SRGBToLinear ( in float32_t4 sRGB )
{
    // See <repo>/docs/srgb#srgb-to-linear
    float16_t3 const c = (float16_t3)sRGB.xyz;
    float16_t3 const lin = c * 7.74e-2H;
    float16_t3 const ex = pow ( mad ( c, (float16_t3)9.479e-1H, (float16_t3)5.213e-2H ), 2.4H );
    return float32_t4 ( (float32_t3)lerp ( lin, ex, (float16_t3)( c > 4.045e-2H ) ), sRGB.w );
}

//----------------------------------------------------------------------------------------------------------------------

OutputData VS ( in InputData inputData )
{
    OutputData result;

    result._vertexH = float32_t4 ( mul ( _rotateScale, inputData._position + _offset ), 0.5F, 1.0F );
    result._imageUV = inputData._imageUV;
    result._atlasUV = inputData._atlasUV;
    result._color = SRGBToLinear ( inputData._color );
    result._atlasLayer = inputData._atlasLayer;

    return result;
}
