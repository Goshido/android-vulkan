struct PresentTransform
{
    float32_t4x4            _transform;
};

[[vk::push_constant]]
const PresentTransform      g_presentTransform;

static const float32_t4 g_vertices[ 4U ] =
{
    float32_t4 ( 1.0F, -1.0F, 0.5F, 1.0F ),
    float32_t4 ( 1.0F, 1.0F, 0.5F, 1.0F ),
    float32_t4 ( -1.0F, -1.0F, 0.5F, 1.0F ),
    float32_t4 ( -1.0F, 1.0F, 0.5F, 1.0F )
};

static const float32_t2 g_uvs[ 4U ] =
{
    float32_t2 ( 1.0F, 0.0F ),
    float32_t2 ( 1.0F, 1.0F ),
    float32_t2 ( 0.0F, 0.0F ),
    float32_t2 ( 0.0F, 1.0F )
};

struct OutputData
{
    linear float32_t4           _vertexH:       SV_Position;

    [[vk::location ( 0 )]]
    noperspective float32_t2    _coordinate:    COORDINATE;
};

//----------------------------------------------------------------------------------------------------------------------

OutputData VS ( in uint32_t vertexID: SV_VertexID )
{
    OutputData result;
    result._vertexH = mul ( g_presentTransform._transform, g_vertices[ vertexID ] );
    result._coordinate = g_uvs[ vertexID ];

    return result;
}
