[[ vk::binding ( 0 ) ]]
cbuffer Transform:                          register ( b0 )
{
    matrix              _transform;
    matrix              _normalTransform;
};

struct InputData
{
    [[ vk::location ( 0 ) ]]
    float3              _vertex:            VERTEX;

    [[ vk::location ( 1 ) ]]
    float2              _uv:                UV;

    [[ vk::location ( 2 ) ]]
    float3              _normal:            NORMAL;

    [[ vk::location ( 3 ) ]]
    float3              _tangent:           TANGENT;

    [[ vk::location ( 4 ) ]]
    float3              _bitangent:         BITANGENT;
};

struct OutputData
{
    linear float4       _vertexH:           SV_Position;

    [[ vk::location ( 0 ) ]]
    linear float3       _fragmentView:      FRAGMENT;

    [[ vk::location ( 1 ) ]]
    linear half2        _uv:                UV;

    [[ vk::location ( 2 ) ]]
    linear half3        _normalView:        NORMAL;

    [[ vk::location ( 3 ) ]]
    linear half3        _tangentView:       TANGENT;

    [[ vk::location ( 4 ) ]]
    linear half3        _bitangentView:     BITANGENT;
};

//----------------------------------------------------------------------------------------------------------------------

OutputData VS ( in InputData inputData )
{
    const float4 vertex = float4 ( inputData._vertex, 1.0f );

    OutputData result;
    result._vertexH = mul ( _transform, vertex );
    result._fragmentView = ( mul ( _normalTransform, vertex ) ).xyz;

    result._uv = (half2)inputData._uv;

    const float3x3 normalTransform = (float3x3)_normalTransform;
    result._normalView = (half3)mul ( normalTransform, inputData._normal );
    result._tangentView = (half3)mul ( normalTransform, inputData._tangent );
    result._bitangentView = (half3)mul ( normalTransform, inputData._bitangent );

    return result;
}
