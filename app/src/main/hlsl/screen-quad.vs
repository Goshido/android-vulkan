struct PresentTransform
{
    matrix                  _transform;
};

[[vk::push_constant]]
const PresentTransform      g_presentTransform;

static const float4 g_vertices[ 4u ] =
{
    float4 ( 1.0f, -1.0f, 0.5f, 1.0f ),
    float4 ( 1.0f, 1.0f, 0.5f, 1.0f ),
    float4 ( -1.0f, -1.0f, 0.5f, 1.0f ),
    float4 ( -1.0f, 1.0f, 0.5f, 1.0f )
};

static const half2 g_uvs[ 4u ] =
{
    half2 ( 1.0h, 0.0h ),
    half2 ( 1.0h, 1.0h ),
    half2 ( 0.0h, 0.0h ),
    half2 ( 0.0h, 1.0h )
};

struct OutputData
{
    linear float4           _vertexH:       SV_Position;

    [[vk::location ( 0 )]]
    noperspective half2     _coordinate:    COORDINATE;
};

//----------------------------------------------------------------------------------------------------------------------

OutputData VS ( in uint vertexID: SV_VertexID )
{
    OutputData result;
    result._vertexH = mul ( g_presentTransform._transform, g_vertices[ vertexID ] );
    result._coordinate = g_uvs[ vertexID ];

    return result;
}
