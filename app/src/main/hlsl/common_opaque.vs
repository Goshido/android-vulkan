#include "object_data.inc"


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

float16_t3x3 GetNormalMatrix ( in float32_t4 quaternion )
{
    float16_t4 const q = (float16_t4)quaternion;
    float16_t3 const abc2 = q.yzw + q.yzw;

    // Note 'T' is just notation for variable separation. Nothing more.
    float16_t4 const rXrTabc2 = q.x * float16_t4 ( q.x, abc2 );
    float16_t4 const caaaXcaTbc2 = float16_t4 ( q.wyyy ) * float16_t4 ( q.wy, abc2.yz );
    float16_t2 const bXbTc2 = q.z * float16_t2 ( q.z, abc2.z );

    float16_t4 const left0 = float16_t4 ( rXrTabc2.w, caaaXcaTbc2.w, caaaXcaTbc2.z, rXrTabc2.y );
    float16_t4 const right0 = float16_t4 ( caaaXcaTbc2.z, -rXrTabc2.z, -rXrTabc2.w, bXbTc2.y );

    float16_t2 const tmp1 = float16_t2 ( rXrTabc2.z, bXbTc2.y ) + float16_t2 ( caaaXcaTbc2.w, -rXrTabc2.y );
    float16_t4 const tmp0 = left0 + right0;

    // Note quaternion unpacks to matrix with column-major like behaviour.
    return float16_t3x3
    (
        // First row.
        rXrTabc2.x + caaaXcaTbc2.y - bXbTc2.x - caaaXcaTbc2.x,
        tmp0.x,
        tmp0.y,

        // Second row.
        tmp0.z,
        rXrTabc2.x - caaaXcaTbc2.y + bXbTc2.x - caaaXcaTbc2.x,
        tmp0.w,

        // Third row.
        tmp1.x,
        tmp1.y,
        rXrTabc2.x - caaaXcaTbc2.y - bXbTc2.x + caaaXcaTbc2.x
    );
}

//----------------------------------------------------------------------------------------------------------------------

OutputData VS ( in InputData inputData )
{
    // Note there was a MALI-G76 bug in the driver:
    // https://community.arm.com/developer/tools-software/graphics/f/discussions/47814/mali-g76-mc4-vulkan-driver-bug
    // It was decided to ignore it after changing float16_t4 to float32_t4 for palette colors in [2021-12-15].

    OutputData result;

    ObjectData const objectData = g_instanceData[ inputData._instanceIndex ];
    result._vertexH = mul ( objectData._localViewProjection, float32_t4 ( inputData._vertex, 1.0F ) );
    result._uv = inputData._uv;
    result._instanceIndex = inputData._instanceIndex;

    float16_t3x3 const normalMatrix = GetNormalMatrix ( objectData._localViewQuat );

    // Note matrix multiplication order is in reverse order compare to the rest of engine code.
    // The reason is that quaternion unpacks to matrix with column-major behaviour.
    // Same time the engine has row-major matrix convention.
    float16_t3 const normalView = mul ( (float16_t3)inputData._normal, normalMatrix );
    float16_t3 const tangentView = mul ( (float16_t3)inputData._tangent, normalMatrix );

    result._normalView = (float32_t3)normalView;
    result._tangentView = (float32_t3)tangentView;
    result._bitangentView = (float32_t3)cross ( normalView, tangentView );

    return result;
}
