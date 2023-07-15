#include "object-data.inc"


struct InputData
{
    [[vk::location ( IN_SLOT_VERTEX )]]
    float32_t3                      _vertex:            VERTEX;

    [[vk::location ( IN_SLOT_UV )]]
    float32_t2                      _uv:                UV;

    [[vk::location ( IN_SLOT_NORMAL )]]
    float32_t3                      _normal:            NORMAL;

    [[vk::location ( IN_SLOT_TANGENT )]]
    float32_t3                      _tangent:           TANGENT;

    [[vk::location ( IN_SLOT_BITANGENT )]]
    float32_t3                      _bitangent:         BITANGENT;

    uint32_t                        _instanceIndex:     SV_InstanceID;
};

struct OutputData
{
    linear float32_t4               _vertexH:           SV_Position;

    [[vk::location ( ATT_SLOT_UV )]]
    linear float32_t2               _uv:                UV;

    [[vk::location ( ATT_SLOT_NORMAL_VIEW )]]
    linear float32_t3               _normalView:        NORMAL;

    [[vk::location ( ATT_SLOT_TANGENT_VIEW )]]
    linear float32_t3               _tangentView:       TANGENT;

    [[vk::location ( ATT_SLOT_BITANGENT_VIEW )]]
    linear float32_t3               _bitangentView:     BITANGENT;

    [[vk::location ( ATT_SLOT_INSTANCE_INDEX )]]
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
