#include "instance-layout.inc"


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

OutputData VS ( in InputData inputData, in uint instanceIndex: SV_InstanceID )
{
    OutputData result;

    const ObjectData objectData = g_instanceData[ instanceIndex ];
    result._vertexH = mul ( objectData._localViewProjection, float4 ( inputData._vertex, 1.0F ) );

    result._uv = (half2)inputData._uv;

    const float3x3 orientation = (float3x3)objectData._localView;
    result._normalView = (half3)mul ( orientation, inputData._normal );
    result._tangentView = (half3)mul ( orientation, inputData._tangent );
    result._bitangentView = (half3)mul ( orientation, inputData._bitangent );

    return result;
}
