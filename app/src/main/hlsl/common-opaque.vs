[[vk::binding ( 0, 0 )]]
cbuffer FrameData:                          register ( b0 )
{
    matrix              _viewProjection;
    matrix              _normalTransform;
};

struct ObjectData
{
    matrix              _localTransform;
};

[[vk::push_constant]]
const ObjectData        g_objectData;

struct InputData
{
    [[vk::location ( 0 )]]
    float3              _vertex:            VERTEX;

    [[vk::location ( 1 )]]
    float2              _uv:                UV;

    [[vk::location ( 2 )]]
    float3              _normal:            NORMAL;

    [[vk::location ( 3 )]]
    float3              _tangent:           TANGENT;

    [[vk::location ( 4 )]]
    float3              _bitangent:         BITANGENT;
};

struct OutputData
{
    linear float4       _vertexH:           SV_Position;

    [[vk::location ( 0 )]]
    linear half2        _uv:                UV;

    [[vk::location ( 1 )]]
    linear half3        _normalView:        NORMAL;

    [[vk::location ( 2 )]]
    linear half3        _tangentView:       TANGENT;

    [[vk::location ( 3 )]]
    linear half3        _bitangentView:     BITANGENT;
};

//----------------------------------------------------------------------------------------------------------------------

OutputData VS ( in InputData inputData )
{
    OutputData result;
    result._vertexH = mul ( _viewProjection, mul ( g_objectData._localTransform, float4 ( inputData._vertex, 1.0f ) ) );

    result._uv = (half2)inputData._uv;

    const float3x3 normalTransform = mul ( (float3x3)g_objectData._localTransform, (float3x3)_normalTransform );
    result._normalView = (half3)mul ( normalTransform, inputData._normal );
    result._tangentView = (half3)mul ( normalTransform, inputData._tangent );
    result._bitangentView = (half3)mul ( normalTransform, inputData._bitangent );

    return result;
}
