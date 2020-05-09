struct InputData
{
    [[ vk::location ( 0 ) ]]
    float4              _vertex:    VERTEX;

    [[ vk::location ( 1 ) ]]
    float2              _uv:        UV;
};

struct OutputData
{
    linear float4      _vertexH:    SV_Position;

    [[ vk::location ( 0 ) ]]
    linear float2      _uv:         UV;
};

//----------------------------------------------------------------------------------------------------------------------

OutputData VS ( in InputData inputData )
{
    OutputData result;
    result._vertexH = inputData._vertex;
    result._uv = inputData._uv;

    return result;
}
