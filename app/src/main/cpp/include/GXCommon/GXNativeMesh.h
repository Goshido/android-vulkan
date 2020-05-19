//version 1.2

#ifndef GX_NATIVE_MESH
#define GX_NATIVE_MESH


#include "GXMath.h"


#pragma pack ( push )
#pragma pack ( 1 )

struct GXNativeMeshHeader final
{
    GXUInt          totalVertices;
    GXUBigInt       vboOffset;      // VBO element struct: position (GXVec3), uv (GXVec2), normal (GXVec3), tangent (GXVec3), bitangent (GXVec3).
};

#pragma pack ( pop )

struct GXMeshInfo final
{
    GXUInt      totalVertices;
    GXFloat*    vboData;

    GXMeshInfo ();
    GXVoid Cleanup ();
};

//------------------------------------------------------------------------------------

GXVoid GXCALL GXLoadNativeMesh ( GXMeshInfo &info, const GXWChar* fileName );


#endif //GX_NATIVE_MESH
