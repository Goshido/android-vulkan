#include "hello_triangle_program.inc"


struct InputData
{
    [[vk::location ( IN_SLOT_VERTEX )]]
    float32_t2                  _vertex:        VERTEX;

    [[vk::location ( IN_SLOT_COLOR )]]
    float32_t3                  _color:         COLOR;
};

struct OutputData
{
    linear float32_t4           _vertexH:       SV_Position;

    [[vk::location ( ATT_SLOT_COLOR )]]
    noperspective float32_t3    _color:         COLOR;
};

//----------------------------------------------------------------------------------------------------------------------

OutputData VS ( in InputData inputData )
{
    OutputData result;
    result._vertexH = float32_t4 ( inputData._vertex, 0.5F, 1.0F );
    result._color = inputData._color;
    return result;
}
