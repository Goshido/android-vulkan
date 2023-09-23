#ifndef ANDROID_VULKAN_BONE_JOINT_HPP
#define ANDROID_VULKAN_BONE_JOINT_HPP


#include "primitive_types.hpp"


namespace android_vulkan {

#pragma pack ( push, 1 )

struct BoneJoint final
{
    Vec3    _location;
    Quat    _orientation;
};

#pragma pack ( pop )

} // namespace android_vulkan


#endif // ANDROID_VULKAN_BONE_JOINT_HPP
