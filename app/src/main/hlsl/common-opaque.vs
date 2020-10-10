#include "instance-layout.inc"


struct InputData
{
    [[vk::location ( 0 )]]
    float3                  _vertex:            VERTEX;

    [[vk::location ( 1 )]]
    float2                  _uv:                UV;

    [[vk::location ( 2 )]]
    float3                  _normal:            NORMAL;

    [[vk::location ( 3 )]]
    float3                  _tangent:           TANGENT;

    [[vk::location ( 4 )]]
    float3                  _bitangent:         BITANGENT;

    uint                    _instanceIndex:     SV_InstanceID;
};

struct OutputData
{
    linear float4           _vertexH:           SV_Position;

    [[vk::location ( 0 )]]
    linear half2            _uv:                UV;

    [[vk::location ( 1 )]]
    linear half3            _normalView:        NORMAL;

    [[vk::location ( 2 )]]
    linear half3            _tangentView:       TANGENT;

    [[vk::location ( 3 )]]
    linear half3            _bitangentView:     BITANGENT;

    [[vk::location ( 4 )]]
    nointerpolation uint    _instanceIndex:     INSTANCE_INDEX;
};

//----------------------------------------------------------------------------------------------------------------------

OutputData VS ( in InputData inputData )
{
    OutputData result;

    const ObjectData objectData = g_instanceData[ inputData._instanceIndex ];
    result._vertexH = mul ( objectData._localViewProjection, float4 ( inputData._vertex, 1.0F ) );

    result._uv = (half2)inputData._uv;

    const float3x3 orientation = (float3x3)objectData._localView;

    // MALI-G76 optimizes _colorX members and breaks memory layout if no any references to those members in the shader.
    // Investigating...
    /*result._normalView = (half3)mul ( orientation, inputData._normal );
    result._tangentView = (half3)mul ( orientation, inputData._tangent );
    result._bitangentView = (half3)mul ( orientation, inputData._bitangent );*/
    result._normalView = (half3)mul ( orientation, inputData._normal ) + (half3)objectData._color0.xyz;
    result._tangentView = (half3)mul ( orientation, inputData._tangent ) + (half3)objectData._color1.xyz;
    result._bitangentView = (half3)mul ( orientation, inputData._bitangent ) + (half3)objectData._color2.xyz + (half3)objectData._color3.xyz;
    result._instanceIndex = inputData._instanceIndex;

    return result;
}
