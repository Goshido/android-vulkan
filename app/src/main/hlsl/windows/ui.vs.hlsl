#include "color_packing.hlsl"
#include "platform/windows/pbr/resource_heap.inc"
#include "ui_bindings.hlsl"
#include "ui_push_constants.hlsl"


using Position = float32_t2;

struct UIVertex
{
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
    // FUCK - use single vertex data buffer
    OutputData result;

    result._vertexH = float32_t4 (
        mul ( g_uiInfo._rotateScale,
            vk::RawBufferLoad<Position> ( g_uiInfo._positionBDA + vertexID * sizeof ( Position ) ) + g_uiInfo._offset
        ),

        0.5F,
        1.0F
    );

    UIVertex const rest = vk::RawBufferLoad<UIVertex> ( g_uiInfo._restBDA + vertexID * sizeof ( UIVertex ) );
    result._uv = (float32_t2)rest._uv;
    result._image = (uint32_t)rest._image;
    result._uiPrimitiveType = (uint32_t)rest._uiPrimitiveType;
    result._color = UnpackColorF32x4 ( rest._colorR, rest._colorG, rest._colorB, rest._colorA );

    return result;
}
