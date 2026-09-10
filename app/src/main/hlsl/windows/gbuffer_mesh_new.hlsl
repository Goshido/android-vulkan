#ifndef GBUFFER_MESH_HLSL
#define GBUFFER_MESH_HLSL


#include "tbn.hlsl"
#include "windows/gbuffer_attributes.hlsl"
#include "windows/gbuffer_push_constants_new.hlsl"
#include "windows/index_stream_new.hlsl"


struct InputData
{
    uint32_t    _vertexID:      SV_VertexID;
    uint32_t    _instanceID:    SV_InstanceID;
};

//----------------------------------------------------------------------------------------------------------------------

Attributes Compute ( in InputData inputData,
    in Frames frameStream,
    in Transforms transformStream,
    in uint64_t indexStream,
    in uint32_t indexType,
    in Positions positionStream,
    in Rests restStream
)
{
    Frame const frame = frameStream.Get ();

    uint64_t const transformOffset = (uint64_t)( inputData._instanceID * sizeof ( Transform ) );
    Transform const transform = Transforms ( (uint64_t)transformStream + transformOffset ).Get ();

    uint32_t idx;

    switch ( indexType )
    {
        case VK_INDEX_TYPE_NONE_KHR:
        {
            idx = inputData._vertexID;
        }
        break;

        case VK_INDEX_TYPE_UINT16:
        {
            uint64_t const indexOffset = (uint64_t)( inputData._vertexID * sizeof ( uint16_t ) );
            idx = (uint32_t)Indices16 ( indexStream + indexOffset ).Get ();
        }
        break;

        case VK_INDEX_TYPE_UINT32:
        {
            uint64_t const indexOffset = (uint64_t)( inputData._vertexID * sizeof ( uint32_t ) );
            idx = Indices32 ( indexStream + indexOffset ).Get ();
        }
        break;

        default:
            // IMPOSSIBLE
        break;
    }

    uint64_t const positionOffset = (uint64_t)( idx * sizeof ( float32_t3 ) );
    float32_t3 const position = Positions ( (uint64_t)positionStream + positionOffset ).Get ();

    uint64_t const restOffset = (uint64_t)( idx * sizeof ( Rest ) );
    Rest const rest = Rests ( (uint64_t)restStream + restOffset ).Get ();

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
