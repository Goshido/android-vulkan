#include "ui_bindings.hlsl"
#include "ui_push_constants.hlsl"


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
    uint64_t const vertexOffset0 = (uint64_t)( vertexID * sizeof ( UIVertex0 ) );
    UIVertex0 const v0 = UIVertices0 ( (uint64_t)g_pushConstants._uiVertices0 + vertexOffset0 ).Get ();

    uint64_t const vertexOffset1 = (uint64_t)( vertexID * sizeof ( UIVertex1 ) );
    UIVertex1 const v1 = UIVertices1 ( (uint64_t)g_pushConstants._uiVertices1 + vertexOffset1 ).Get ();

    OutputData result;

    result._vertexH = float32_t4 ( mul ( g_pushConstants._rotateScale, v0._position + g_pushConstants._offset ),
        0.5F,
        1.0F
    );

    result._uv = (float32_t2)v0._uv;
    result._image = (uint32_t)v1._image;
    result._uiPrimitiveType = (uint32_t)v1._uiPrimitiveType;
    result._color = UnpackColorF32x4 ( v0._color );

    return result;
}
