#include "windows/frame_stream.hlsl"
#include "windows/index_stream.hlsl"
#include "windows/position_stream.hlsl"
#include "windows/transform_stream.hlsl"


struct InputData
{
    uint32_t        _vertexID:              SV_VertexID;
    uint32_t        _instanceID:            SV_InstanceID;
};

struct PushConstants
{
    Transforms                              _transformStream;
    Frames                                  _frameStream;
    Positions                               _positionStream;
    uint64_t                                _indexStream;
    eIndex                                  _indexType;
};

[[vk::push_constant]]
PushConstants                               g_pushConstants;

//----------------------------------------------------------------------------------------------------------------------

linear float32_t4 VS ( in InputData inputData ): SV_Position
{
    Frame const frame = g_pushConstants._frameStream.Get ();

    uint64_t const transformOffset = (uint64_t)( inputData._instanceID * sizeof ( Transform ) );
    Transform const transform = Transforms ( (uint64_t)g_pushConstants._transformStream + transformOffset ).Get ();

    // FUCK - resolve face toggle

    uint32_t const idx = ResolveIndex ( g_pushConstants._indexStream,
        g_pushConstants._indexType,
        inputData._vertexID,
        false
    );

    uint64_t const positionOffset = (uint64_t)( idx * sizeof ( float32_t3 ) );
    float32_t3 const position = Positions ( (uint64_t)g_pushConstants._positionStream + positionOffset ).Get ();

    return mul ( frame._viewProj,
        float32_t4 (
            transform._location + mul ( ToMatrix ( transform._rotation ), position * transform._scale ),
            1.0F
        )
    );
}
