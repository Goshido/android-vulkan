#ifndef GIZMO_TILE_HLSL
#define GIZMO_TILE_HLSL


#define TILE_WIDTH              8U
#define TILE_WIDTH_SHIFT        3U
#define TILE_LOCAL_X_MASK       0x00000007U

#define TILE_HEIGHT             8U
#define TILE_HEIGHT_SHIFT       3U
#define TILE_LOCAL_Y_MASK       0x00000007U

#define TILE_LAYERS             8U

//----------------------------------------------------------------------------------------------------------------------

struct GizmoCounters
{
    uint32_t    _counters[ TILE_HEIGHT ];
};


#endif // GIZMO_TILE_HLSL
