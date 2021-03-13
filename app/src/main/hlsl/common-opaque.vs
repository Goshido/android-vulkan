#include "gpgpu_limits.inc"


struct ObjectData
{
    matrix                      _localView;
    matrix                      _localViewProjection;
    half4                       _color0;
    half4                       _color1;
    half4                       _color2;
    half4                       _color3;
};

[[vk::binding ( 0, 1 )]]
cbuffer InstanceData:                               register ( b0 )
{
    // sizeof ( ObjectData ) = 160 bytes
    // sizeof ( InstanceData ) = 6720 bytes, less than minimum "Supported Limit"
    // see https://vulkan.lunarg.com/doc/view/1.1.108.0/mac/chunked_spec/chap36.html#limits-minmax
    ObjectData                  g_instanceData[ PBR_OPAQUE_MAX_INSTANCE_COUNT ];
}

struct InputData
{
    [[vk::location ( 0 )]]
    float3                      _vertex:            VERTEX;

    [[vk::location ( 1 )]]
    float2                      _uv:                UV;

    [[vk::location ( 2 )]]
    float3                      _normal:            NORMAL;

    [[vk::location ( 3 )]]
    float3                      _tangent:           TANGENT;

    [[vk::location ( 4 )]]
    float3                      _bitangent:         BITANGENT;

    uint                        _instanceIndex:     SV_InstanceID;
};

struct OutputData
{
    linear float4               _vertexH:           SV_Position;

    [[vk::location ( 0 )]]
    linear half2                _uv:                UV;

    [[vk::location ( 1 )]]
    linear half3                _normalView:        NORMAL;

    [[vk::location ( 2 )]]
    linear half3                _tangentView:       TANGENT;

    [[vk::location ( 3 )]]
    linear half3                _bitangentView:     BITANGENT;

    [[vk::location ( 4 )]]
    nointerpolation half4       _color0:            COLOR_0;

    [[vk::location ( 5 )]]
    nointerpolation half4       _color1:            COLOR_1;

    [[vk::location ( 6 )]]
    nointerpolation half4       _color2:            COLOR_2;

    [[vk::location ( 7 )]]
    nointerpolation half4       _color3:            COLOR_3;
};

//----------------------------------------------------------------------------------------------------------------------

OutputData VS ( in InputData inputData )
{
    // Note current implementation is suboptimal because of known MALI driver bug.
    // MALI-G76 optimizes _colorX members and breaks memory layout if no any references to those members in the shader.
    // https://community.arm.com/developer/tools-software/graphics/f/discussions/47814/mali-g76-mc4-vulkan-driver-bug

    OutputData result;

    const ObjectData objectData = g_instanceData[ inputData._instanceIndex ];
    result._vertexH = mul ( objectData._localViewProjection, float4 ( inputData._vertex, 1.0F ) );
    result._uv = (half2)inputData._uv;

    const float3x3 orientation = (float3x3)objectData._localView;
    result._normalView = (half3)mul ( orientation, inputData._normal );
    result._tangentView = (half3)mul ( orientation, inputData._tangent );
    result._bitangentView = (half3)mul ( orientation, inputData._bitangent );

    // So pass the color data to the fragment shader :(
    // Disclimer: the color data is visible in the fragment shader already but MALI driver bug... :(
    result._color0 = objectData._color0;
    result._color1 = objectData._color1;
    result._color2 = objectData._color2;
    result._color3 = objectData._color3;

    return result;
}
