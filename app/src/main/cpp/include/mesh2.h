#ifndef ANDROID_VULKAN_MESH2_H
#define ANDROID_VULKAN_MESH2_H


#include <primitive_types.h>


namespace android_vulkan {

#pragma pack ( push, 1 )

struct Mesh2Header final
{
    AABB        _bounds;

    uint64_t    _indexCount;
    uint64_t    _indexDataOffset;

    uint32_t    _vertexCount;
    uint64_t    _vertexDataOffset;
};

struct Mesh2Vertex final
{
     Vec3       _vertex;
     Vec2       _uv;
     Vec3       _normal;
     Vec3       _tangent;
     Vec3       _bitangent;
};

using Mesh2Index = uint32_t;

#pragma pack ( pop )

} // namespace android_vulkan


#endif // ANDROID_VULKAN_MESH2_H
