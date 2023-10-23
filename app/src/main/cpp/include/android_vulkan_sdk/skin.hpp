#ifndef ANDROID_VULKAN_SKIN_HPP
#define ANDROID_VULKAN_SKIN_HPP


#include "primitive_types.hpp"


namespace android_vulkan {

#pragma pack ( push, 1 )

struct SkinHeader final
{
    AABB        _bounds;

    uint32_t    _skinVertexCount;
    uint64_t    _skinVertexDataOffset;

    uint32_t    _boneCount;
    uint64_t    _boneDataOffset;
};

struct SkinBone final
{
    uint32_t                        _index;
    android_vulkan::UTF8Offset      _name;
};

struct SkinInfluence final
{
    uint32_t                    _boneIndex;
    [[maybe_unused]] float      _boneWeight;
};

constexpr static size_t BONES_PER_VERTEX = 4U;

struct SkinVertex final
{
    SkinInfluence       _influences[ BONES_PER_VERTEX ];
};

#pragma pack ( pop )

} // namespace android_vulkan


#endif // ANDROID_VULKAN_SKIN_HPP
