#ifndef ANDROID_VULKAN_LOADER_HPP
#define ANDROID_VULKAN_LOADER_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <vulkan/vulkan.h>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

class VulkanLoader final
{
    private:
        void*       _vulkan = nullptr;

    public:
        VulkanLoader () = default;

        VulkanLoader ( VulkanLoader const & ) = delete;
        VulkanLoader &operator = ( VulkanLoader const & ) = delete;

        VulkanLoader ( VulkanLoader && ) = delete;
        VulkanLoader &operator = ( VulkanLoader && ) = delete;

        ~VulkanLoader () = default;

        [[nodiscard]] bool AcquireBootstrapFunctions () noexcept;
        [[nodiscard]] bool AcquireDeviceFunctions ( VkDevice device ) noexcept;
        [[nodiscard]] bool AcquireInstanceFunctions ( VkInstance instance ) noexcept;
        [[nodiscard]] bool Unload () noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_LOADER_HPP
