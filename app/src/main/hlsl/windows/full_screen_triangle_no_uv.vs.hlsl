static float32_t2 const g_vertices[ 3U ] =
{
    float32_t2 ( -1.0F, -3.0F ),
    float32_t2 ( 3.0F, 1.0F ),
    float32_t2 ( -1.0F, 1.0F )
};

//----------------------------------------------------------------------------------------------------------------------

linear float32_t4 VS ( in uint32_t vertexID: SV_VertexID ): SV_Position
{
    return float32_t4 ( g_vertices[ vertexID ], 0.5F, 1.0F );
}
