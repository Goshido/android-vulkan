#include <precompiled_headers.hpp>

#if defined ( ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS ) ||       \
    defined ( ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION )

#include <av_assert.hpp>
#include <renderer.hpp>
#include <vulkan_api.hpp>
#include <vulkan_utils.hpp>


namespace android_vulkan {

namespace {

PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT = nullptr;
PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT = nullptr;
PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = nullptr;

constexpr GXVec4 GROUP_COLOR ( 115.0F / 255.0F, 185.0F / 255.0F, 0.0F, 1.0F );

} // end of anonymous namespace

void InitVulkanDebugUtils ( VkInstance instance ) noexcept
{
    // [2024/02/28] It's important to acquire functions via VkInstance for RenderDoc only integration.
    // It means without VVL presence. Trying to use VkDevice way to get functions will fail.

    vkCmdBeginDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT> (
        reinterpret_cast<void*> ( vkGetInstanceProcAddr ( instance, "vkCmdBeginDebugUtilsLabelEXT" ) )
    );

    AV_ASSERT ( vkCmdBeginDebugUtilsLabelEXT )

    vkCmdEndDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT> (
        reinterpret_cast<void*> ( vkGetInstanceProcAddr ( instance, "vkCmdEndDebugUtilsLabelEXT" ) )
    );

    AV_ASSERT ( vkCmdEndDebugUtilsLabelEXT )

    vkSetDebugUtilsObjectNameEXT = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT> (
        reinterpret_cast<void*> ( vkGetInstanceProcAddr ( instance, "vkSetDebugUtilsObjectNameEXT" ) )
    );

    AV_ASSERT ( vkSetDebugUtilsObjectNameEXT )
}

void SetVulkanObjectName ( VkDevice device, uint64_t handle, VkObjectType type, char const *name ) noexcept
{
    VkDebugUtilsObjectNameInfoEXT const info
    {
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

//----------------------------------------------------------------------------------------------------------------------

VulkanGroup::VulkanGroup ( VkCommandBuffer commandBuffer ) noexcept:
    _commandBuffer ( commandBuffer )
{
    // NOTHING
}

VulkanGroup::~VulkanGroup () noexcept
{
    vkCmdEndDebugUtilsLabelEXT ( _commandBuffer );
}

void VulkanGroup::IssueGroup ( char const* name ) noexcept
{
    VkDebugUtilsLabelEXT const info
    {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
        .pNext = nullptr,
        .pLabelName = name,
        .color = { GROUP_COLOR._data[ 0U ], GROUP_COLOR._data[ 1U ], GROUP_COLOR._data[ 2U ], GROUP_COLOR._data[ 3U ] }
    };

    vkCmdBeginDebugUtilsLabelEXT ( _commandBuffer, &info );
}

} // namespace android_vulkan


#endif // ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS || ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION
