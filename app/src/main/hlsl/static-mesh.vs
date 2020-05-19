[[ vk::binding ( 0 ) ]]
cbuffer Transform:                      register ( b0 )
{
    matrix              _transform;
};

struct InputData
{
    [[ vk::location ( 0 ) ]]
    float3              _vertex:        VERTEX;

    [[ vk::location ( 1 ) ]]
    float2              _uv:            UV;
};

struct OutputData
{
    linear float4       _vertexH:       SV_Position;

    [[ vk::location ( 0 ) ]]
    linear half2        _uv:            UV;
};

//----------------------------------------------------------------------------------------------------------------------

OutputData VS ( in InputData inputData )
{
    OutputData result;
    result._vertexH = mul ( _transform, float4 ( inputData._vertex, 1.0f ) );
    result._uv = (half2)inputData._uv;

    return result;
}
