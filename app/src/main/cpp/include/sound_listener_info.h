#ifndef ANDROID_VULKAN_SOUND_LISTENER_INFO_HPP
#define ANDROID_VULKAN_SOUND_LISTENER_INFO_HPP


#include <GXCommon/GXMath.h>


namespace android_vulkan {

struct SoundListenerInfo final
{
    GXVec3      _leftDirection { -1.0F, 0.0F, 0.0F };
    GXVec3      _rightDirection { 1.0F, 0.0F, 0.0F };
    GXVec3      _location { 0.0F, 0.0F, 0.0F };
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_SOUND_LISTENER_INFO_HPP
