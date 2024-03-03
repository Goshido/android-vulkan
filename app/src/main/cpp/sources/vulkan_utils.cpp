#if defined ( ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS ) ||       \
    defined ( ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION )


#include <av_assert.hpp>
#include <renderer.hpp>
#include <vulkan_api.hpp>
#include <vulkan_utils.hpp>


namespace android_vulkan {

namespace {

PFN_vkCmdBeginDebugUtilsLabelEXT    vkCmdBeginDebugUtilsLabelEXT = nullptr;
PFN_vkCmdEndDebugUtilsLabelEXT      vkCmdEndDebugUtilsLabelEXT = nullptr;
PFN_vkSetDebugUtilsObjectNameEXT    vkSetDebugUtilsObjectNameEXT = nullptr;

} // end of anonymous namespace

void InitVulkanDebugUtils ( VkInstance instance ) noexcept
{
    // [2024/02/28] It's important to acquire functions via VkInstance for RenderDoc only integration.
    // It means without VVL presence. Trying to use VkDevice way to get functions will fail.

    vkCmdBeginDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT> (
        vkGetInstanceProcAddr ( instance, "vkCmdBeginDebugUtilsLabelEXT" )
    );

    AV_ASSERT ( vkCmdBeginDebugUtilsLabelEXT )

    vkCmdEndDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT> (
        vkGetInstanceProcAddr ( instance, "vkCmdEndDebugUtilsLabelEXT" )
    );

    AV_ASSERT ( vkCmdEndDebugUtilsLabelEXT )

    vkSetDebugUtilsObjectNameEXT = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT> (
        vkGetInstanceProcAddr ( instance, "vkSetDebugUtilsObjectNameEXT" )
    );

    AV_ASSERT ( vkSetDebugUtilsObjectNameEXT )
}

void SetVulkanObjectName ( VkDevice device, uint64_t handle, VkObjectType type, char const *name ) noexcept
{
    VkDebugUtilsObjectNameInfoEXT const info {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .pNext = nullptr,
        .objectType = type,
        .objectHandle = handle,
        .pObjectName = name
    };

    [[maybe_unused]] bool const result = Renderer::CheckVkResult ( vkSetDebugUtilsObjectNameEXT ( device, &info ),
        "SetVulkanObjectName",
        "Can't set Vulkan object name"
    );

    AV_ASSERT ( result )
}

} // namespace android_vulkan


#endif // ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS || ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION
