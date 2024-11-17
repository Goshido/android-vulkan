#include "object_data.inc"
#include "tbn.inc"


struct InputData
{
    [[vk::location ( IN_SLOT_POSITION )]]
    float32_t3                      _postion:           POSITION;

    [[vk::location ( IN_SLOT_UV )]]
    float32_t2                      _uv:                UV;

    [[vk::location ( IN_SLOT_TBN )]]
    float32_t4                      _tbn:               TBN;

    uint32_t                        _instanceIndex:     SV_InstanceID;
};

struct OutputData
{
    linear float32_t4               _vertexH:           SV_Position;

    [[vk::location ( ATT_SLOT_UV )]]
    linear float32_t2               _uv:                UV;

    [[vk::location ( ATT_SLOT_TANGENT_VIEW )]]
    linear float32_t3               _tangentView:       TANGENT;

    [[vk::location ( ATT_SLOT_BITANGENT_VIEW )]]
    linear float32_t3               _bitangentView:     BITANGENT;

    [[vk::location ( ATT_SLOT_NORMAL_VIEW )]]
    linear float32_t3               _normalView:        NORMAL;

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

    result._vertexH = mul ( g_localViewProj[ inputData._instanceIndex ], float32_t4 ( inputData._postion, 1.0F ) );
    result._uv = inputData._uv;
    result._instanceIndex = inputData._instanceIndex;

    float16_t4 const compressedTBN = mad ( (float16_t4)inputData._tbn, 2.0H, -1.0H );
    float16_t3 normalView;
    float16_t3 tangentView;

    GetNormalAndTangent ( normalView,
        tangentView,
        RotateTBN ( RecoverTBN ( compressedTBN.xyz ), (float16_t4)g_localView[ inputData._instanceIndex ] )
    );

    result._tangentView = (float32_t3)tangentView;
    result._bitangentView = (float32_t3)( cross ( normalView, tangentView ) * compressedTBN.w );
    result._normalView = (float32_t3)normalView;

    return result;
}
