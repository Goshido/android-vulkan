#include "platform/windows/pbr/index_stream.inc"
#include "tbn.hlsl"
#include "windows/gbuffer_attribute_slots.hlsl"


struct InputData
{
    uint32_t                        _vertexID:          SV_VertexID;
    uint32_t                        _instanceID:        SV_InstanceID;
};

struct PushConstants
{
    uint64_t                        _transformStream;
    uint64_t                        _shadingStream;
    uint64_t                        _frameStream;
    uint64_t                        _positionStream;
    uint64_t                        _restStream;
    uint64_t                        _indexStream;
    uint16_t                        _indexType;
};

[[vk::push_constant]]
PushConstants                       g_pushConstants;

using Position = float32_t3;

struct Rest
{
    float16_t2                      _uv;
    uint32_t                        _tbn;
};

using IndexType = uint16_t;
using Index16 = uint16_t;
using Index32 = uint32_t;

struct Frame
{
    float32_t4x4                    _viewProj;
};

struct Transform
{
    float32_t3x4                    _model;
    Quat64                          _normal;
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

    [[vk::location ( ATT_SLOT_INSTANCE_ID )]]
    nointerpolation uint32_t        _instanceID:        INSTANCE_ID;
};

//----------------------------------------------------------------------------------------------------------------------

OutputData VS ( in InputData inputData )
{
    Frame const frame = vk::RawBufferLoad<Frame> ( g_pushConstants._frameStream, 4U );

    Transform const transform = vk::RawBufferLoad<Transform> (
        g_pushConstants._transformStream + inputData._instanceID * sizeof ( Transform ),
        8U
    );

    uint32_t idx;

    switch ( g_pushConstants._indexType )
    {
        case INDEX_TYPE_16:
            idx = (uint32_t)vk::RawBufferLoad<Index16> (
                g_pushConstants._indexStream + inputData._vertexID * sizeof ( Index16 ),
                2U
            );
        break;

        case INDEX_TYPE_32:
            idx = vk::RawBufferLoad<Index32> ( g_pushConstants._indexStream + inputData._vertexID * sizeof ( Index32 ),
                4U
            );
        break;

        case INDEX_TYPE_NONE:
            idx = inputData._vertexID;
        break;

        default:
            // IMPOSSIBLE
        break;
    }

    Position const position = vk::RawBufferLoad<Position> ( g_pushConstants._positionStream + idx * sizeof ( Position ),
        4U
    );

    Rest const rest = vk::RawBufferLoad<Rest> ( g_pushConstants._restStream + idx * sizeof ( Rest ), 4U );

    OutputData result;

    result._vertexH = mul ( frame._viewProj,
        float32_t4 ( mul ( transform._model, float32_t4 ( position, 1.0F ) ), 1.0F )
    );

    result._uv = (float32_t2)rest._uv;
    result._tangentView = (float32_t3)0.0F;

    float16_t4 const compressedTBN = mad ( (float16_t4)rest._tbn, 2.0H, -1.0H );
    float16_t3 normalView;
    float16_t3 tangentView;

    GetNormalAndTangent ( normalView,
        tangentView,
        RotateTBN ( RecoverTBN ( compressedTBN.xyz ), DecompressQuat64 ( transform._normal ) )
    );

    result._tangentView = (float32_t3)tangentView;
    result._bitangentView = (float32_t3)( cross ( normalView, tangentView ) * compressedTBN.w );
    result._normalView = (float32_t3)normalView;

    return result;
}
