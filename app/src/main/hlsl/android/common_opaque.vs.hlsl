#include "android/geometry_pass_attributes.hlsl"
#include "android/object_data.hlsl"
#include "tbn.hlsl"
#include "tbn32.hlsl"


struct InputData
{
    [[vk::location ( IN_SLOT_POSITION )]]
    float32_t3                      _position:          POSITION;

    [[vk::location ( IN_SLOT_UV )]]
    float32_t2                      _uv:                UV;

    [[vk::location ( IN_SLOT_TBN )]]
    TBN32F                          _tbn:               TBN;

    uint32_t                        _instanceIndex:     SV_InstanceID;
};

//----------------------------------------------------------------------------------------------------------------------

Attributes VS ( in InputData inputData )
{
    // Note there was a MALI-G76 bug in the driver:
    // https://community.arm.com/developer/tools-software/graphics/f/discussions/47814/mali-g76-mc4-vulkan-driver-bug
    // It was decided to ignore it after changing float16_t4 to float32_t4 for palette colors in [2021-12-15].

    Attributes result;

    result._vertexH = mul ( g_localViewProj[ inputData._instanceIndex ], float32_t4 ( inputData._position, 1.0F ) );
    result._uv = inputData._uv;
    result._instanceIndex = inputData._instanceIndex;

    float16_t4 const tbn = mad ( (float16_t4)inputData._tbn, 2.0H, -1.0H );
    float16_t3 normalView;
    float16_t3 tangentView;

    GetNormalAndTangent ( normalView,
        tangentView,

        Rotate ( Recover ( tbn.xyz ),
            Decompress ( g_localView[ inputData._instanceIndex >> 1U ], inputData._instanceIndex & 0x00000001U )
        )
    );

    result._tangentView = (float32_t3)tangentView;
    result._bitangentView = (float32_t3)( cross ( normalView, tangentView ) * tbn.w );
    result._normalView = (float32_t3)normalView;

    return result;
}
