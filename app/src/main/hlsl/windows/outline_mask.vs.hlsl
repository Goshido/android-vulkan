#include "windows/frame_stream.hlsl"
#include "windows/vertex_index.hlsl"


struct InputData
{
    uint32_t        _vertexID:      SV_VertexID;
    uint32_t        _instanceID:    SV_InstanceID;
};

struct Outline
{
    float32_t3x4    _model;
};

struct PushConstants
{
    uint64_t        _outlineStream;
    uint64_t        _frameStream;
    uint64_t        _positionStream;
    uint64_t        _indexStream;
    uint32_t        _indexType;
};

[[vk::push_constant]]
PushConstants       g_pushConstants;

//----------------------------------------------------------------------------------------------------------------------

linear float32_t4 VS ( in InputData inputData ): SV_Position
{
    Frame const frame = vk::RawBufferLoad<Frame> ( g_pushConstants._frameStream, 4U );

    Outline const outline = vk::RawBufferLoad<Outline> (
        g_pushConstants._outlineStream + inputData._instanceID * sizeof ( Outline ),
        8U
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

    return mul ( frame._viewProj,
        float32_t4 ( mul ( outline._model, float32_t4 ( position, 1.0F ) ), 1.0F )
    );
}
