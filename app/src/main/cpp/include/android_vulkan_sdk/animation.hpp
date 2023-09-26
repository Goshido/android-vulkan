#ifndef ANDROID_VULKAN_ANIMATION_HPP
#define ANDROID_VULKAN_ANIMATION_HPP


#include "primitive_types.hpp"


namespace android_vulkan {

#pragma pack ( push, 1 )

struct AnimationHeader final
{
    float       _fps;
    uint32_t    _frameCount;
    uint64_t    _animationDataOffset;
    uint64_t    _boneNameInfoOffset;
};

#pragma pack ( pop )

} // namespace android_vulkan


#endif // ANDROID_VULKAN_ANIMATION_HPP
