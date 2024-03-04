#ifndef VULKAN_UTILS_HPP
#define VULKAN_UTILS_HPP


// Sanity check
#if defined ( ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS ) &&                                                      \
    defined ( ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION )

#error ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS and ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION \
macro are mutually exclusive. Please check build configuration in the CMakeLists.txt file.

#endif // ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS && ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION

#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <cstdint>

GX_RESTORE_WARNING_STATE

#define AV_DX_ALIGNMENT_BEGIN _Pragma ( "pack ( push, 1 )" )
#define AV_DX_ALIGNMENT_END _Pragma ( "pack ( pop )" )

#define AV_VK_FLAG(x) ( static_cast<uint32_t> ( x ) )

#if !defined ( ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS ) &&                                                     \
    !defined ( ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION )

#define AV_SET_VULKAN_OBJECT_NAME(device, handle, type, ...)
#define AV_VULKAN_GROUP(commandBuffer, ...)

#else

#include <vulkan/vulkan_core.h>


#define AV_SET_VULKAN_OBJECT_NAME(device, handle, type, ...)                                                           \
{                                                                                                                      \
    char _FuCk_NaMe_[ 256U ];                                                                                          \
    std::snprintf ( _FuCk_NaMe_, std::size ( _FuCk_NaMe_ ), __VA_ARGS__ );                                             \
    android_vulkan::SetVulkanObjectName ( device, reinterpret_cast<uint64_t> ( handle ), type, _FuCk_NaMe_ );          \
}

#define AV_VULKAN_GROUP(commandBuffer, ...)                                                                            \
android_vulkan::VulkanGroup _FuCk_VuLkAn_GrOuP_ ( commandBuffer );                                                     \
{                                                                                                                      \
    char _FuCk_NaMe_[ 256U ];                                                                                          \
    std::snprintf ( _FuCk_NaMe_, std::size ( _FuCk_NaMe_ ), __VA_ARGS__ );                                             \
    _FuCk_VuLkAn_GrOuP_.IssueGroup ( _FuCk_NaMe_ );                                                                    \
}

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

#endif // ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS || ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION


#endif // VULKAN_UTILS_HPP
