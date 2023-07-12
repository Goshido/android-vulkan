#ifndef ANDROID_VULKAN_MESH2_HPP
#define ANDROID_VULKAN_MESH2_HPP


#include <primitive_types.h>


namespace android_vulkan {

#pragma pack ( push, 1 )

struct Mesh2Header final
{
    AABB                            _bounds;

    uint64_t                        _indexCount;
    uint64_t                        _indexDataOffset;

    uint32_t                        _vertexCount;
    [[maybe_unused]] uint64_t       _vertexDataOffset;
};

struct Mesh2Vertex final
{
     [[maybe_unused]] Vec3      _vertex;
     [[maybe_unused]] Vec2      _uv;
     Vec3                       _normal;
     [[maybe_unused]] Vec3      _tangent;
     [[maybe_unused]] Vec3      _bitangent;
};

using Mesh2Index [[maybe_unused]] = uint32_t;

#pragma pack ( pop )

} // namespace android_vulkan


#endif // ANDROID_VULKAN_MESH2_HPP
