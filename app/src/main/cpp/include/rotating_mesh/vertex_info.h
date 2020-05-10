#ifndef ROTATING_MESH_VERTEX_INFO
#define ROTATING_MESH_VERTEX_INFO


#include <GXCommon/GXMath.h>


namespace rotating_mesh {

#pragma pack ( push, 1 )

struct VertexInfo final
{
    GXVec4      _vertex;
    GXVec2      _uv;

    constexpr VertexInfo ( float vx, float vy, float vz, float vw, float tu, float tv ):
        _vertex ( vx, vy, vz, vw ),
        _uv ( tu, tv )
    {
        // NOTHING
    }

    constexpr VertexInfo ( const GXVec4 &vertex, const GXVec2 &uv ):
        _vertex ( vertex._data[ 0U ], vertex._data[ 1U ], vertex._data[ 2U ], vertex._data[ 3U ] ),
        _uv ( uv._data[ 0U ], uv._data[ 1U ] )
    {
        // NOTHING
    }
};

#pragma pack ( pop )

} // namespace rotating_mesh


#endif // ROTATING_MESH_VERTEX_INFO
