#include "gpgpu_limits.inc"


// 1.0H / 255.0H
#define UNORM_FACTOR    3.922e-3H

//----------------------------------------------------------------------------------------------------------------------

struct ObjectData
{
    float32_t4x4                    _localView;
    float32_t4x4                    _localViewProjection;
    uint32_t                        _color0;
    uint32_t                        _color1;
    uint32_t                        _color2;
    uint32_t                        _color3;
};

[[vk::binding ( 0, 1 )]]
cbuffer InstanceData:                                   register ( b0 )
{
    // sizeof ( ObjectData ) = 160 bytes
    // sizeof ( InstanceData ) = 6720 bytes, less than minimum "Supported Limit"
    // see https://vulkan.lunarg.com/doc/view/1.1.108.0/mac/chunked_spec/chap36.html#limits-minmax
    ObjectData                      g_instanceData[ PBR_OPAQUE_MAX_INSTANCE_COUNT ];
}

struct InputData
{
    [[vk::location ( 0 )]]
    float32_t3                      _vertex:            VERTEX;

    [[vk::location ( 1 )]]
    float32_t2                      _uv:                UV;

    [[vk::location ( 2 )]]
    float32_t3                      _normal:            NORMAL;

    [[vk::location ( 3 )]]
    float32_t3                      _tangent:           TANGENT;

    [[vk::location ( 4 )]]
    float32_t3                      _bitangent:         BITANGENT;

    uint32_t                        _instanceIndex:     SV_InstanceID;
};

struct OutputData
{
    linear float32_t4               _vertexH:           SV_Position;

    [[vk::location ( 0 )]]
    linear float16_t2               _uv:                UV;

    [[vk::location ( 1 )]]
    linear float16_t3               _normalView:        NORMAL;

    [[vk::location ( 2 )]]
    linear float16_t3               _tangentView:       TANGENT;

    [[vk::location ( 3 )]]
    linear float16_t3               _bitangentView:     BITANGENT;

    [[vk::location ( 4 )]]
    nointerpolation float16_t4      _color0:            COLOR_0;

    [[vk::location ( 5 )]]
    nointerpolation float16_t4      _color1:            COLOR_1;

    [[vk::location ( 6 )]]
    nointerpolation float16_t4      _color2:            COLOR_2;

    [[vk::location ( 7 )]]
    nointerpolation float16_t4      _color3:            COLOR_3;
};

//----------------------------------------------------------------------------------------------------------------------

OutputData VS ( in InputData inputData )
{
    // Note current implementation is suboptimal because of known MALI driver bug.
    // MALI-G76 optimizes _colorX members and breaks memory layout if no any references to those members in the shader.
    // https://community.arm.com/developer/tools-software/graphics/f/discussions/47814/mali-g76-mc4-vulkan-driver-bug

    OutputData result;

    const ObjectData objectData = g_instanceData[ inputData._instanceIndex ];
    result._vertexH = mul ( objectData._localViewProjection, float32_t4 ( inputData._vertex, 1.0F ) );
    result._uv = (float16_t2)inputData._uv;

    const float32_t3x3 orientation = (float32_t3x3)objectData._localView;

    // So pass the color data to the fragment shader :(
    // Disclimer: the color data is visible in the fragment shader already but MALI driver bug... :(
    const uint16_t4 c0u = unpack_u8u16 ( objectData._color0 );

    result._normalView = (float16_t3)mul ( orientation, inputData._normal );
    const uint16_t4 c1u = unpack_u8u16 ( objectData._color1 );
    const float16_t4 c0h = (float16_t4)c0u;

    const uint16_t4 c2u = unpack_u8u16 ( objectData._color2 );
    const float16_t4 c1h = (float16_t4)c1u;
    result._color0 = c0h * UNORM_FACTOR;

    const uint16_t4 c3u = unpack_u8u16 ( objectData._color3 );
    const float16_t4 c2h = (float16_t4)c2u;
    result._color1 = c1h * UNORM_FACTOR;

    const float16_t4 c3h = (float16_t4)c3u;
    result._tangentView = (float16_t3)mul ( orientation, inputData._tangent );
    result._color2 = c2h * UNORM_FACTOR;

    result._bitangentView = (float16_t3)mul ( orientation, inputData._bitangent );
    result._color3 = c3h * UNORM_FACTOR;

    return result;
}
