#include "pbr/full_screen_triangle.inc"


[[vk::binding ( BIND_TRANSFORM, SET_TRANSFORM )]]
cbuffer Transform:                              register ( b0 )
{
    float32_t2x2                g_transform;
};

static float32_t2 const g_vertices[ 3U ] =
{
    float32_t2 ( -1.0F, -3.0F ),
    float32_t2 ( 3.0F, 1.0F ),
    float32_t2 ( -1.0F, 1.0F )
};

static float32_t2 const g_uvs[ 3U ] =
{
    float32_t2 ( 0.0F, -1.0F ),
    float32_t2 ( 2.0F, 1.0F ),
    float32_t2 ( 0.0F, 1.0F )
};

struct OutputData
{
    linear float32_t4           _vertexH:       SV_Position;

    [[vk::location ( ATT_SLOT_UV )]]
    noperspective float32_t2    _uv:            UV;
};

//----------------------------------------------------------------------------------------------------------------------

OutputData VS ( in uint32_t vertexID: SV_VertexID )
{
    OutputData result;
    result._vertexH = float32_t4 ( mul ( g_transform, g_vertices[ vertexID ] ), 0.5F, 1.0F );
    result._uv = g_uvs[ vertexID ];
    return result;
}
