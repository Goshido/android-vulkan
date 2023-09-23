#ifndef ANDROID_VULKAN_SKELETON_HPP
#define ANDROID_VULKAN_SKELETON_HPP


#include "primitive_types.hpp"


namespace android_vulkan {

#pragma pack ( push, 1 )

constexpr static int32_t ROOT_BONE = -1;
using BoneParent = int32_t;

struct SkeletonHeader final
{
    uint32_t    _boneCount;
    uint64_t    _referenceTransformOffset;
    uint64_t    _inverseBindTransformOffset;
    uint64_t    _parentOffset;
    uint64_t    _nameInfoOffset;
};

#pragma pack ( pop )

} // namespace android_vulkan


#endif // ANDROID_VULKAN_SKELETON_HPP
