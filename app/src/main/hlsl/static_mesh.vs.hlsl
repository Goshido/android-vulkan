#include "platform/android/rotating_mesh/bindings.inc"
#include "tbn.hlsl"


[[vk::binding ( BIND_TRANSFORM, SET_ONCE )]]
cbuffer Transform:                              register ( b0 )
{
    float32_t4x4            _transform;
    float32_t4x4            _normalTransform;
    float32_t4              _localView;
};

struct InputData
{
    [[vk::location ( IN_SLOT_POSITION )]]
    float32_t3              _position:          POSITION;

    [[vk::location ( IN_SLOT_UV )]]
    float32_t2              _uv:                UV;

    [[vk::location ( IN_SLOT_TBN )]]
    float32_t4              _tbn:               TBN;
};

struct OutputData
{
    linear float32_t4       _vertexH:           SV_Position;

    [[vk::location ( ATT_SLOT_FRAGMENT_VIEW )]]
    linear float32_t3       _fragmentView:      FRAGMENT;

    [[vk::location ( ATT_SLOT_UV )]]
    linear float32_t2       _uv:                UV;

    [[vk::location ( ATT_SLOT_NORMAL_VIEW )]]
    linear float32_t3       _normalView:        NORMAL;

    [[vk::location ( ATT_SLOT_TANGENT_VIEW )]]
    linear float32_t3       _tangentView:       TANGENT;

    [[vk::location ( ATT_SLOT_BITANGENT_VIEW )]]
    linear float32_t3       _bitangentView:     BITANGENT;
};

//----------------------------------------------------------------------------------------------------------------------

OutputData VS ( in InputData inputData )
{
    float32_t4 const vertex = float32_t4 ( inputData._position, 1.0F );

    OutputData result;

    result._vertexH = mul ( _transform, vertex );
    result._fragmentView = ( mul ( _normalTransform, vertex ) ).xyz;
    result._uv = inputData._uv;

    float16_t4 const compressedTBN = mad ( (float16_t4)inputData._tbn, 2.0H, -1.0H );
    float16_t3 normalView;
    float16_t3 tangentView;

    GetNormalAndTangent ( normalView,
        tangentView,
        RotateTBN ( RecoverTBN ( compressedTBN.xyz ), (float16_t4)_localView )
    );

    result._tangentView = (float32_t3)tangentView;
    result._bitangentView = (float32_t3)( cross ( normalView, tangentView ) * compressedTBN.w );
    result._normalView = (float32_t3)normalView;

    return result;
}
