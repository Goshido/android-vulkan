static const float4 g_vertices[ 4U ] =
{
    float4 ( -1.0f, -1.0f, 0.5f, 1.0f ),
    float4 ( 1.0f, -1.0f, 0.5f, 1.0f ),
    float4 ( -1.0f, 1.0f, 0.5f, 1.0f ),
    float4 ( 1.0f, 1.0f, 0.5f, 1.0f )
};

static const float2 g_coordiantes[ 4U ] =
{
    float2 ( -2.79505F, 1.0F ),
    float2 ( 1.39752F, 1.0F ),
    float2 ( -2.79505F, -1.0F ),
    float2 ( 1.39752F, -1.0F )
};

struct OutputData
{
    linear float4      _vertexH:        SV_Position;

    [[ vk::location ( 0 ) ]]
    linear float2      _coordinate:     COORDINATE;
};

//----------------------------------------------------------------------------------------------------------------------

OutputData VS ( in uint vertexID: SV_VertexID )
{
    OutputData result;
    result._vertexH = g_vertices[ vertexID ];
    result._coordinate = g_coordiantes[ vertexID ];

    return result;
}
