#include "tbn.hlsl"
#include "tbn32.hlsl"
#include "tbn64.hlsl"
#include "windows/gbuffer_attributes.hlsl"
#include "windows/gbuffer_push_constants.hlsl"
#include "windows/index_stream.hlsl"


struct InputData
{
    uint32_t        _vertexID:      SV_VertexID;
    uint32_t        _instanceID:    SV_InstanceID;
};

struct Rest
{
    float16_t2      _uv;
    TBN32           _tbn;
};

struct Frame
{
    float32_t4x4    _viewProj;
};

struct Transform
{
    float32_t3x4    _model;
    TBN64           _normal;
};

//----------------------------------------------------------------------------------------------------------------------

Attributes VS ( in InputData inputData )
{
    Frame const frame = vk::RawBufferLoad<Frame> ( g_pushConstants._frameStream, 4U );

    Transform const transform = vk::RawBufferLoad<Transform> (
        g_pushConstants._transformStream + inputData._instanceID * sizeof ( Transform ),
        8U
    );

    Rest const rest = vk::RawBufferLoad<Rest> (
        g_pushConstants._restStream + inputData._instanceID * sizeof ( Rest ),
        4U
    );

    uint32_t idx;

    switch ( g_pushConstants._indexType )
    {
        case VK_INDEX_TYPE_NONE_KHR:
            idx = inputData._vertexID;
        break;

        case VK_INDEX_TYPE_UINT16:
            idx = (uint32_t)vk::RawBufferLoad<uint16_t> (
                g_pushConstants._indexStream + inputData._vertexID * sizeof ( uint16_t ),
                2U
            );
        break;

        case VK_INDEX_TYPE_UINT32:
            idx = vk::RawBufferLoad<uint32_t> (
                g_pushConstants._indexStream + inputData._vertexID * sizeof ( uint32_t ),
                4U
            );
        break;

        default:
            // IMPOSSIBLE
        break;
    }

    float32_t3 const position = vk::RawBufferLoad<float32_t3> (
        g_pushConstants._positionStream + idx * sizeof ( float32_t3 ),
        4U
    );

    Attributes result;

    result._vertexH = mul ( frame._viewProj,
        float32_t4 ( mul ( transform._model, float32_t4 ( position, 1.0F ) ), 1.0F )
    );

    result._uv = (float32_t2)rest._uv;

    float16_t3 normalView;
    float16_t3 tangentView;
    GetNormalAndTangent ( normalView, tangentView, Rotate ( ToQuat ( rest._tbn ), ToQuat ( transform._normal ) ) );

    result._tangentView = (float32_t3)tangentView;
    result._bitangentView = (float32_t3)( cross ( normalView, tangentView ) * GetBitangentMirroring ( rest._tbn ) );
    result._normalView = (float32_t3)normalView;

    result._instanceID = inputData._instanceID;
    return result;
}
