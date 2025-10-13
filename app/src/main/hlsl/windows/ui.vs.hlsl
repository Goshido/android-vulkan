#include "color_packing.hlsl"
#include "platform/windows/pbr/resource_heap.inc"
#include "ui_bindings.hlsl"
#include "ui_push_constants.hlsl"


// [2025/09/25] The whole UI vertex is described by 18 bytes. There is no way to read float32_t from offset
// non multiple of 4. So it's needed to use SoA pattern. One example where flexible vertex pulling is worse
// than classical input assembly.
struct UIVertexStream0
{
    float32_t2                      _position;
    float16_t2                      _uv;
    uint32_t                        _colorR: 8;
    uint32_t                        _colorG: 8;
    uint32_t                        _colorB: 8;
    uint32_t                        _colorA: 8;
};

struct UIVertexStream1
{
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
    UIVertexStream0 const s0 = vk::RawBufferLoad<UIVertexStream0> (
        g_pushConstants._bdaStream0 + vertexID * sizeof ( UIVertexStream0 ),
        4U
    );

    UIVertexStream1 const s1 = vk::RawBufferLoad<UIVertexStream1> (
        g_pushConstants._bdaStream1 + vertexID * sizeof ( UIVertexStream1 ),
        2U
    );

    OutputData result;

    result._vertexH = float32_t4 ( mul ( g_pushConstants._rotateScale, s0._position + g_pushConstants._offset ),
        0.5F,
        1.0F
    );

    result._uv = (float32_t2)s0._uv;
    result._image = (uint32_t)s1._image;
    result._uiPrimitiveType = (uint32_t)s1._uiPrimitiveType;
    result._color = UnpackColorF32x4 ( s0._colorR, s0._colorG, s0._colorB, s0._colorA );

    return result;
}
