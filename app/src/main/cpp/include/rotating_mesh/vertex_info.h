#ifndef ROTATING_MESH_VERTEX_INFO
#define ROTATING_MESH_VERTEX_INFO


namespace rotating_mesh {

#pragma pack ( push, 1 )

struct VertexInfo final
{
    float _vx;
    float _vy;
    float _vz;
    float _vw;

    float _tu;
    float _tv;

    constexpr VertexInfo ( float vx, float vy, float vz, float vw, float tu, float tv ):
        _vx ( vx ),
        _vy ( vy ),
        _vz ( vz ),
        _vw ( vw ),
        _tu ( tu ),
        _tv ( tv )
    {
        // NOTHING
    }
};

#pragma pack ( pop )

} // namespace rotating_mesh


#endif // ROTATING_MESH_VERTEX_INFO
