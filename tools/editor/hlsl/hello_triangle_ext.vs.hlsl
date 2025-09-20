#include "hello_triangle_program.inc"


struct PushConstants
{
    uint64_t                    _bda;
};

[[vk::push_constant]]
PushConstants                   g_pushConstants;

struct Vertex
{
    float32_t2                  _vertex;
    float32_t3                  _color;
};

struct OutputData
{
    linear float32_t4           _vertexH:       SV_Position;

    [[vk::location ( ATT_SLOT_COLOR )]]
    noperspective float32_t3    _color:         COLOR;
};

//----------------------------------------------------------------------------------------------------------------------

OutputData VS ( in uint32_t vertexID: SV_VertexID )
{
    Vertex const vertex = vk::RawBufferLoad<Vertex> ( g_pushConstants._bda + vertexID * sizeof ( Vertex ), 4U );

    OutputData result;
    result._vertexH = float32_t4 ( vertex._vertex, 0.5F, 1.0F );
    result._color = vertex._color;
    return result;
}
