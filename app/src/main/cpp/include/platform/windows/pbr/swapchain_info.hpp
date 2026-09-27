#ifndef PBR_SWAPCHAIN_INFO_HPP
#define PBR_SWAPCHAIN_INFO_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <vulkan/vulkan_core.h>

GX_RESTORE_WARNING_STATE


namespace pbr {

struct SwapchainInfo final
{
    VkImage         _image = VK_NULL_HANDLE;
    VkImageView     _view = VK_NULL_HANDLE;
    uint32_t        _idx = 0U;
};

} // namespace pbr


#endif // PBR_SWAPCHAIN_INFO_HPP
