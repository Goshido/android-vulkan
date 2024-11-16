#include "object_data.inc"


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

void GetTangentBitangent ( out float16_t3 normalView, out float16_t3 tangentView, in float16_t4 tbn )
{
    float16_t3 const abc2 = tbn.yzw + tbn.yzw;

    // Note 'T' is just notation for variable separation. Nothing more.
    float16_t4 const rXrTabc2 = tbn.x * float16_t4 ( tbn.x, abc2 );
    float16_t4 const caaaXcaTbc2 = float16_t4 ( tbn.wyyy ) * float16_t4 ( tbn.wy, abc2.yz );
    float16_t2 const bXbTc2 = tbn.z * float16_t2 ( tbn.z, abc2.z );

    float16_t4 const left = float16_t4 ( rXrTabc2.w, caaaXcaTbc2.w, rXrTabc2.z, bXbTc2.y );
    float16_t4 const right = float16_t4 ( caaaXcaTbc2.z, -rXrTabc2.z, caaaXcaTbc2.w, -rXrTabc2.y );
    float16_t4 const tmp = left + right;

    // Note quaternion unpacks to matrix with column-major like behaviour.
    normalView = float16_t3 ( tmp.zw, rXrTabc2.x - caaaXcaTbc2.y - bXbTc2.x + caaaXcaTbc2.x );
    tangentView = float16_t3 ( rXrTabc2.x + caaaXcaTbc2.y - bXbTc2.x - caaaXcaTbc2.x, tmp.xy );
}

float16_t4 RotateTBN ( in float16_t3 imaginaryTBN, in float32_t4 localView )
{
    // By convention xyz contains imaginary part of quaternion. The w component contains mirroring information.
    // Real part of quaternion must be restored. By convention it's used unit quaternions to represent TBN.
    // So the real component will be restored using this property.
    float16_t4 const rotation = (float16_t4)localView;

    // Note dot product could be a little bit bigger than 1.0H due to float16_t inaccurency. Fixing it with abs.
    float16_t4 tbn = float16_t4 ( sqrt ( abs ( 1.0H - dot ( imaginaryTBN, imaginaryTBN ) ) ), imaginaryTBN );

    return float16_t4 (
        dot ( rotation, float16_t4 ( tbn.x, -tbn.yzw ) ),
        dot ( rotation, float16_t4 ( tbn.yxw, -tbn.z ) ),
        dot ( rotation, float16_t4 ( tbn.z, -tbn.w, tbn.xy ) ),
        dot ( rotation, float16_t4 ( tbn.wz, -tbn.y, tbn.x ) )
    );
}

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

    GetTangentBitangent ( normalView,
        tangentView,
        RotateTBN ( compressedTBN.xyz, g_localView[ inputData._instanceIndex ] )
    );

    result._tangentView = (float32_t3)tangentView;
    result._bitangentView = (float32_t3)( cross ( normalView, tangentView ) * compressedTBN.w );
    result._normalView = (float32_t3)normalView;

    return result;
}
