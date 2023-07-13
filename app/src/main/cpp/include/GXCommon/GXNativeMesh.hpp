//version 1.4

#ifndef GX_NATIVE_MESH_HPP
#define GX_NATIVE_MESH_HPP


#include "GXMath.hpp"


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
    [[maybe_unused]] GXUInt         totalVertices;
    [[maybe_unused]] GXFloat*       vboData;

    [[maybe_unused]] GXMeshInfo ();
    [[maybe_unused]] GXVoid Cleanup ();
};

//------------------------------------------------------------------------------------

[[maybe_unused]] GXVoid GXCALL GXLoadNativeMesh ( GXMeshInfo &info, const GXWChar* fileName );


#endif //GX_NATIVE_MESH_HPP
