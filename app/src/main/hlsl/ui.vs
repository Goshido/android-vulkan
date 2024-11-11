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

    [[vk::location ( IN_SLOT_COLOR )]]
    float32_t4                      _color:         COLOR;

    [[vk::location ( IN_SLOT_ATLAS )]]
    float32_t3                      _atlas:         ATLAS;

    [[vk::location ( IN_SLOT_IMAGE_UV )]]
    float32_t2                      _imageUV:       IMAGE;
};

struct OutputData
{
    linear float32_t4               _vertexH:       SV_Position;

    [[vk::location ( ATT_SLOT_COLOR )]]
    nointerpolation float32_t4      _color:         COLOR;

    [[vk::location ( ATT_SLOT_ATLAS_UV )]]
    noperspective float32_t2        _atlasUV:       ATLAS_UV;

    [[vk::location ( ATT_SLOT_ATLAS_LAYER )]]
    nointerpolation float32_t       _atlasLayer:    ATLAS_LAYER;

    [[vk::location ( ATT_SLOT_IMAGE_UV )]]
    noperspective float32_t2        _imageUV:       IMAGE;
};

//----------------------------------------------------------------------------------------------------------------------

OutputData VS ( in InputData inputData )
{
    OutputData result;

    result._vertexH = float32_t4 ( mul ( _rotateScale, inputData._position + _offset ), 0.5F, 1.0F );
    result._color = inputData._color;
    result._atlasUV = inputData._atlas.xy;
    result._atlasLayer = inputData._atlas.z;
    result._imageUV = inputData._imageUV;

    return result;
}
