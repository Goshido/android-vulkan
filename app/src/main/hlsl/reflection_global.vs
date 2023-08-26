static float32_t4 const g_vertices[ 4U ] =
{
    float32_t4 ( 1.0F, -1.0F, 0.5F, 1.0F ),
    float32_t4 ( 1.0F, 1.0F, 0.5F, 1.0F ),
    float32_t4 ( -1.0F, -1.0F, 0.5F, 1.0F ),
    float32_t4 ( -1.0F, 1.0F, 0.5F, 1.0F )
};

//----------------------------------------------------------------------------------------------------------------------

linear float32_t4 VS ( in uint32_t vertexID: SV_VertexID ): SV_Position
{
    return g_vertices[ vertexID ];
}
