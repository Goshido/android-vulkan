static const float32_t4 g_vertices[ 4U ] =
{
    float32_t4 ( 1.0F, -1.0F, 0.5F, 1.0F ),
    float32_t4 ( 1.0F, 1.0F, 0.5F, 1.0F ),
    float32_t4 ( -1.0F, -1.0F, 0.5F, 1.0F ),
    float32_t4 ( -1.0F, 1.0F, 0.5F, 1.0F )
};

static const float32_t2 g_coordiantes[ 4U ] =
{
    float32_t2 ( -2.79505F, 1.0F ),
    float32_t2 ( 1.39752F, 1.0F ),
    float32_t2 ( -2.79505F, -1.0F ),
    float32_t2 ( 1.39752F, -1.0F )
};

struct OutputData
{
    linear float32_t4       _vertexH:       SV_Position;

    [[vk::location ( 0 )]]
    linear float32_t2       _coordinate:    COORDINATE;
};

//----------------------------------------------------------------------------------------------------------------------

OutputData VS ( in uint32_t vertexID: SV_VertexID )
{
    OutputData result;
    result._vertexH = g_vertices[ vertexID ];
    result._coordinate = g_coordiantes[ vertexID ];

    return result;
}
