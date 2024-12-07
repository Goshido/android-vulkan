static float32_t2 const g_outsideCVV[ 3U ] =
{
    float32_t2 ( 6.0F, 4.0F ),
    float32_t2 ( 10.0F, 8.0F ),
    float32_t2 ( 6.0F, 8.0F )
};

//----------------------------------------------------------------------------------------------------------------------

float32_t4 VS ( in uint32_t vertexID: SV_VertexID ): SV_Position
{
    return float32_t4 ( g_outsideCVV[ vertexID ], -1.0F, 1.0F );
}
