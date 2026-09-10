#include "windows/frame_stream.hlsl"
#include "windows/index_stream.hlsl"
#include "windows/position_stream.hlsl"


struct InputData
{
    uint32_t        _vertexID:              SV_VertexID;
    uint32_t        _instanceID:            SV_InstanceID;
};

struct Outline
{
    float32_t3x4                            _model;
};

typedef vk::BufferPointer<Outline, 8U>      Outlines;

struct PushConstants
{
    Outlines                                _outlineStream;
    Frames                                  _frameStream;
    Positions                               _positionStream;
    uint64_t                                _indexStream;
    uint32_t                                _indexType;
};

[[vk::push_constant]]
PushConstants                               g_pushConstants;

//----------------------------------------------------------------------------------------------------------------------

linear float32_t4 VS ( in InputData inputData ): SV_Position
{
    Frame const frame = g_pushConstants._frameStream.Get ();

    uint64_t const outlineOffset = (uint64_t)( inputData._instanceID * sizeof ( Outline ) );
    Outline const outline = Outlines ( (uint64_t)g_pushConstants._outlineStream + outlineOffset ).Get ();

    uint32_t idx;

    switch ( g_pushConstants._indexType )
    {
        case VK_INDEX_TYPE_NONE_KHR:
        {
            idx = inputData._vertexID;
        }
        break;

        case VK_INDEX_TYPE_UINT16:
        {
            uint64_t const indexOffset = (uint64_t)( inputData._vertexID * sizeof ( uint16_t ) );
            idx = (uint32_t)Indices16 ( g_pushConstants._indexStream + indexOffset ).Get ();
        }
        break;

        case VK_INDEX_TYPE_UINT32:
        {
            uint64_t const indexOffset = (uint64_t)( inputData._vertexID * sizeof ( uint32_t ) );
            idx = Indices32 ( g_pushConstants._indexStream + indexOffset ).Get ();
        }
        break;

        default:
            // IMPOSSIBLE
        break;
    }

    uint64_t const positionOffset = (uint64_t)( idx * sizeof ( float32_t3 ) );
    float32_t3 const position = Positions ( (uint64_t)g_pushConstants._positionStream + positionOffset ).Get ();
    return mul ( frame._viewProj, float32_t4 ( mul ( outline._model, float32_t4 ( position, 1.0F ) ), 1.0F ) );
}
