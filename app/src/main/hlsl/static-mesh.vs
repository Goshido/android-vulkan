[[ vk::binding ( 3 ) ]]
cbuffer PETransform:                register ( b0 )
{
    matrix              _transform;
};

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
    result._vertexH = mul ( inputData._vertex, _transform );
    result._uv = inputData._uv;

    return result;
}
