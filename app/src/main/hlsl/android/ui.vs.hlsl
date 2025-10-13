#include "platform/android/pbr/ui_program.inc"


[[vk::binding ( BIND_TRANSFORM, SET_TRANSFORM )]]
cbuffer Transform:                                  register ( b0 )
{
    float32_t2x2                    _rotateScale;
    float32_t2                      _offset;
};

struct InputData
{
    [[vk::location ( IN_SLOT_POSITION )]]
    float32_t2                      _position:                          POSITION;

    [[vk::location ( IN_SLOT_IMAGE_UV )]]
    float32_t2                      _uv:                                UV;

    [[vk::location ( IN_SLOT_ATLAS_LAYER_AND_UI_PRIMITIVE_TYPE )]]
    float32_t2                      _atlasLayerAndUIPrimitiveType:      ATLAS_LAYER_AND_UI_PRIMITIVE_TYPE;

    [[vk::location ( IN_SLOT_COLOR )]]
    float32_t4                      _color:                             COLOR;
};

struct OutputData
{
    linear float32_t4               _vertexH:                           SV_Position;

    [[vk::location ( ATT_SLOT_UV )]]
    noperspective float32_t2        _uv:                                UV;

    [[vk::location ( ATT_SLOT_ATLAS_LAYER )]]
    nointerpolation float32_t       _atlasLayer:                        ATLAS_LAYER;

    [[vk::location ( ATT_SLOT_UI_PRIMITIVE_TYPE )]]
    nointerpolation uint32_t        _uiPrimitiveType:                   UI_PRIMITIVE_TYPE;

    [[vk::location ( ATT_SLOT_COLOR )]]
    nointerpolation float32_t4      _color:                             COLOR;
};

//----------------------------------------------------------------------------------------------------------------------

OutputData VS ( in InputData inputData )
{
    OutputData result;

    result._vertexH = float32_t4 ( mul ( _rotateScale, inputData._position + _offset ), 0.5F, 1.0F );
    result._uv = inputData._uv;

    float32_t2 const v = mad ( inputData._atlasLayerAndUIPrimitiveType, 255.0F, 0.5F );
    result._atlasLayer = v.x;
    result._uiPrimitiveType = (uint32_t)v.y;

    result._color = inputData._color;

    return result;
}
