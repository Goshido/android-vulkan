#ifndef VULKAN_UTILS_HPP
#define VULKAN_UTILS_HPP


// Sanity check
#if defined ( AV_ENABLE_VVL ) && ( defined ( AV_ENABLE_RENDERDOC ) || defined ( AV_ENABLE_NSIGHT ) )

#error AV_ENABLE_VVL and ( AV_ENABLE_RENDERDOC or AV_ENABLE_NSIGHT ) \
macro are mutually exclusive. Please check build configuration in the CMakeLists.txt file.

#endif // AV_ENABLE_VVL && ( AV_ENABLE_RENDERDOC || AV_ENABLE_NSIGHT )

#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <cstdint>

GX_RESTORE_WARNING_STATE

#define AV_DX_ALIGNMENT_BEGIN _Pragma ( "pack ( push, 1 )" )
#define AV_DX_ALIGNMENT_END _Pragma ( "pack ( pop )" )

#define AV_VK_FLAG(x) ( static_cast<uint32_t> ( x ) )

#if !defined ( AV_ENABLE_VVL ) && !defined ( AV_ENABLE_RENDERDOC ) && !defined ( AV_ENABLE_NSIGHT )

#define AV_SET_VULKAN_OBJECT_NAME(device, handle, type, ...)
#define AV_VULKAN_GROUP(commandBuffer, ...)

#else

#include <vulkan/vulkan_core.h>

#define AV_VULKAN_GEN(x, y) x##____##y
#define AV_VULKAN_GEN2(x, y) AV_VULKAN_GEN ( x, y )

#define AV_SET_VULKAN_OBJECT_NAME_IMPL(buf, device, handle, type, ...)                                                 \
{                                                                                                                      \
    char buf[ 256U ];                                                                                                  \
    std::snprintf ( buf, std::size ( buf ), __VA_ARGS__ );                                                             \
    android_vulkan::SetVulkanObjectName ( device, reinterpret_cast<uint64_t> ( handle ), type, buf );                  \
}

#define AV_SET_VULKAN_OBJECT_NAME(device, handle, type, ...)                                                           \
    AV_SET_VULKAN_OBJECT_NAME_IMPL ( AV_VULKAN_GEN2 ( buf, __LINE__ ), device, handle, type, __VA_ARGS__ )

#define AV_VULKAN_GROUP_IMPL(buf, group, commandBuffer, ...)                                                           \
    android_vulkan::VulkanGroup group ( commandBuffer );                                                               \
    char buf[ 256U ];                                                                                                  \
    std::snprintf ( buf, std::size ( buf ), __VA_ARGS__ );                                                             \
    group.IssueGroup ( buf );

#define AV_VULKAN_GROUP(commandBuffer, ...)                                                                            \
    AV_VULKAN_GROUP_IMPL ( AV_VULKAN_GEN2 ( buf, __LINE__ ),                                                           \
        AV_VULKAN_GEN2 ( group, __LINE__ ),                                                                            \
        commandBuffer,                                                                                                 \
        __VA_ARGS__                                                                                                    \
    )

namespace android_vulkan {

// Initialization is based on article
// https://www.saschawillems.de/blog/2016/05/28/tutorial-on-using-vulkans-vk_ext_debug_marker-with-renderdoc/
void InitVulkanDebugUtils ( VkInstance instance ) noexcept;

void SetVulkanObjectName ( VkDevice device, uint64_t handle, VkObjectType type, char const *name ) noexcept;

//----------------------------------------------------------------------------------------------------------------------

class VulkanGroup final
{
    private:
        VkCommandBuffer     _commandBuffer = VK_NULL_HANDLE;

    public:
        VulkanGroup () = delete;

        VulkanGroup ( VulkanGroup const & ) = delete;
        VulkanGroup &operator = ( VulkanGroup const & ) = delete;

        VulkanGroup ( VulkanGroup && ) = delete;
        VulkanGroup &operator = ( VulkanGroup && ) = delete;

        explicit VulkanGroup ( VkCommandBuffer commandBuffer ) noexcept;

        ~VulkanGroup () noexcept;

        void IssueGroup ( char const* name ) noexcept;
};

} // namespace android_vulkan

#endif // !AV_ENABLE_VVL && !AV_ENABLE_RENDERDOC && !AV_ENABLE_NSIGHT


#endif // VULKAN_UTILS_HPP
