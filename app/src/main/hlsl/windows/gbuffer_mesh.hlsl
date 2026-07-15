#ifndef GBUFFER_MESH_HLSL
#define GBUFFER_MESH_HLSL


#include "tbn.hlsl"
#include "windows/gbuffer_attributes.hlsl"
#include "windows/gbuffer_push_constants.hlsl"
#include "windows/gbuffer_streams.hlsl"
#include "windows/vertex_index.hlsl"


struct InputData
{
    uint32_t    _vertexID:      SV_VertexID;
    uint32_t    _instanceID:    SV_InstanceID;
};

//----------------------------------------------------------------------------------------------------------------------

Attributes Compute ( in InputData inputData,
    in uint64_t frameStream,
    in uint64_t transformStream,
    in uint64_t indexStream,
    in uint32_t indexType,
    in uint64_t positionStream,
    in uint64_t restStream
)
{
    Frame const frame = vk::RawBufferLoad<Frame> ( frameStream, 4U );

    Transform const transform = vk::RawBufferLoad<Transform> (
        transformStream + inputData._instanceID * sizeof ( Transform ),
        8U
    );

    uint32_t idx;

    switch ( indexType )
    {
        case VK_INDEX_TYPE_NONE_KHR:
            idx = inputData._vertexID;
        break;

        case VK_INDEX_TYPE_UINT16:
            idx = (uint32_t)vk::RawBufferLoad<uint16_t> ( indexStream + inputData._vertexID * sizeof ( uint16_t ), 2U );
        break;

        case VK_INDEX_TYPE_UINT32:
            idx = vk::RawBufferLoad<uint32_t> ( indexStream + inputData._vertexID * sizeof ( uint32_t ), 4U );
        break;

        default:
            // IMPOSSIBLE
        break;
    }

    float32_t3 const position = vk::RawBufferLoad<float32_t3> ( positionStream + idx * sizeof ( float32_t3 ), 4U );
    Rest const rest = vk::RawBufferLoad<Rest> ( restStream + idx * sizeof ( Rest ), 4U );

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


#endif // GBUFFER_MESH_HLSL
