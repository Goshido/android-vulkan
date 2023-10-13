#ifndef ANDROID_VULKAN_JOINT_HPP
#define ANDROID_VULKAN_JOINT_HPP


#include <GXCommon/GXMath.hpp>


namespace android_vulkan {

struct Joint final
{
    GXVec3      _location;
    GXQuat      _orientation;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_JOINT_HPP
