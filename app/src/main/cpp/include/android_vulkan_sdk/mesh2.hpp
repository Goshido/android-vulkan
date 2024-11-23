#ifndef ANDROID_VULKAN_MESH2_HPP
#define ANDROID_VULKAN_MESH2_HPP


#include "primitive_types.hpp"


namespace android_vulkan {

#pragma pack ( push, 1 )

using Mesh2Position [[maybe_unused]] = Vec3;

struct [[maybe_unused]] Mesh2Header final
{
    AABB            _bounds;

    uint32_t        _indexCount;
    uint64_t        _indexDataOffset;

    uint32_t        _vertexCount;
    uint64_t        _positionDataOffset;
    uint64_t        _vertexDataOffset;
};

struct [[maybe_unused]] Mesh2Vertex final
{
     Vec2           _uv;
     uint32_t       _tbn;
};

using Mesh2Index16 [[maybe_unused]] = uint16_t;
using Mesh2Index32 [[maybe_unused]] = uint32_t;

#pragma pack ( pop )

} // namespace android_vulkan


#endif // ANDROID_VULKAN_MESH2_HPP
