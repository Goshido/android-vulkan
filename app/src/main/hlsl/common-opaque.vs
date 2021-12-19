#include "object-data.inc"


// 1.0F / 255.0F
#define UNORM_FACTOR    3.922e-3F

//----------------------------------------------------------------------------------------------------------------------

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
    linear float32_t2               _uv:                UV;

    [[vk::location ( 1 )]]
    linear float32_t3               _normalView:        NORMAL;

    [[vk::location ( 2 )]]
    linear float32_t3               _tangentView:       TANGENT;

    [[vk::location ( 3 )]]
    linear float32_t3               _bitangentView:     BITANGENT;

    [[vk::location ( 4 )]]
    nointerpolation uint32_t        _instanceIndex:     INSTANCE_INDEX;
};

//----------------------------------------------------------------------------------------------------------------------

OutputData VS ( in InputData inputData )
{
    // Note there was a MALI-G76 bug in the driver:
    // https://community.arm.com/developer/tools-software/graphics/f/discussions/47814/mali-g76-mc4-vulkan-driver-bug
    // It was decided to ignore it after changing float16_t4 to float32_t4 for palette colors in [2021-12-15].

    OutputData result;

    const ObjectData objectData = g_instanceData[ inputData._instanceIndex ];
    result._vertexH = mul ( objectData._localViewProjection, float32_t4 ( inputData._vertex, 1.0F ) );
    result._uv = inputData._uv;

    const float32_t3x3 orientation = (float32_t3x3)objectData._localView;
    result._normalView = mul ( orientation, inputData._normal );
    result._tangentView = mul ( orientation, inputData._tangent );
    result._bitangentView = mul ( orientation, inputData._bitangent );

    result._instanceIndex = inputData._instanceIndex;

    return result;
}
