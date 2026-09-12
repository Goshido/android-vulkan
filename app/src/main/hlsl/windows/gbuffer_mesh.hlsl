#ifndef GBUFFER_MESH_HLSL
#define GBUFFER_MESH_HLSL


#include "tbn.hlsl"
#include "windows/gbuffer_attributes.hlsl"
#include "windows/gbuffer_push_constants.hlsl"


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
    in eIndex indexType,
    in Positions positionStream,
    in Rests restStream
)
{
    Frame const frame = frameStream.Get ();

    uint64_t const transformOffset = (uint64_t)( inputData._instanceID * sizeof ( Transform ) );
    Transform const transform = Transforms ( (uint64_t)transformStream + transformOffset ).Get ();

    // FUCK - resolve face toggle
    uint32_t const idx = ResolveIndex ( indexStream, indexType, inputData._vertexID, false );
    uint64_t const positionOffset = (uint64_t)( idx * sizeof ( float32_t3 ) );
    float32_t3 const position = Positions ( (uint64_t)positionStream + positionOffset ).Get ();

    uint64_t const restOffset = (uint64_t)( idx * sizeof ( Rest ) );
    Rest const rest = Rests ( (uint64_t)restStream + restOffset ).Get ();

    Attributes result;

    result._vertexH = mul ( frame._viewProj,
        float32_t4 (
            transform._location + mul ( ToMatrix ( transform._rotation ), position * transform._scale ),
            1.0F
        )
    );

    result._uv = (float32_t2)rest._uv;

    float16_t3 normalLocal;
    float16_t3 tangentLocal;

    GetNormalAndTangent ( normalLocal, tangentLocal, ToQuat ( rest._tbn ) );
    float16_t3 bitangentLocal = cross ( normalLocal, tangentLocal ) * GetBitangentMirroring ( rest._tbn );

    // FUCK - do magic with scale

    float16_t3x3 const r = ToMatrix ( Rotate ( ToQuat ( transform._rotation ), ToQuat ( frame._toView ) ) );

    result._tangentView = (float32_t3)mul ( tangentLocal, r );
    result._bitangentView = (float32_t3)mul ( bitangentLocal, r );
    result._normalView = (float32_t3)mul ( normalLocal, r );

    result._instanceID = inputData._instanceID;
    return result;
}


#endif // GBUFFER_MESH_HLSL
