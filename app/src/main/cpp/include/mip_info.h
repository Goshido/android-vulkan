#ifndef MIP_INFO_H
#define MIP_INFO_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <vulkan/vulkan.h>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

struct MipInfo final
{
    VkDeviceSize        _size;
    uint8_t const*      _data;
    VkExtent2D          _resolution;
};

} // namespace android_vulkan


#endif // MIP_INFO_H
