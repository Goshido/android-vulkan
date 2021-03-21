static const float4 g_vertices[ 4u ] =
{
    float4 ( 1.0f, -1.0f, 0.5f, 1.0f ),
    float4 ( 1.0f, 1.0f, 0.5f, 1.0f ),
    float4 ( -1.0f, -1.0f, 0.5f, 1.0f ),
    float4 ( -1.0f, 1.0f, 0.5f, 1.0f )
};

//----------------------------------------------------------------------------------------------------------------------

linear float4 VS ( in uint vertexID: SV_VertexID ): SV_Position
{
    return g_vertices[ vertexID ];
}
