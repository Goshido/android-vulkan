#include "color_packing.hlsl"
#include "platform/windows/pbr/resource_heap.inc"
#include "ui_bindings.hlsl"
#include "ui_push_constants.hlsl"


struct UIVertex
{
    float32_t2                      _position;
    float16_t2                      _uv;
    uint32_t                        _colorR: 8;
    uint32_t                        _colorG: 8;
    uint32_t                        _colorB: 8;
    uint32_t                        _colorA: 8;
    uint16_t                        _image: UI_IMAGE_BITS;
    uint16_t                        _uiPrimitiveType: ( 16 - UI_IMAGE_BITS );
};

struct OutputData
{
    linear float32_t4               _vertexH:               SV_Position;

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

OutputData VS ( in uint32_t vertexID: SV_VertexID )
{
    UIVertex const uiVertex = vk::RawBufferLoad<UIVertex> ( g_pushConstants._bda + vertexID * sizeof ( UIVertex ), 2U );

    OutputData result;

    result._vertexH = float32_t4 ( mul ( g_pushConstants._rotateScale, uiVertex._position + g_pushConstants._offset ),
        0.5F,
        1.0F
    );

    result._uv = (float32_t2)uiVertex._uv;
    result._image = (uint32_t)uiVertex._image;
    result._uiPrimitiveType = (uint32_t)uiVertex._uiPrimitiveType;
    result._color = UnpackColorF32x4 ( uiVertex._colorR, uiVertex._colorG, uiVertex._colorB, uiVertex._colorA );

    return result;
}
