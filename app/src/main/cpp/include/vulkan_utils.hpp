#ifndef VULKAN_UTILS_HPP
#define VULKAN_UTILS_HPP


// Sanity check
#if defined ( ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS ) && \
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

#if !defined ( ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS ) &&      \
    !defined ( ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION )

#define AV_REGISTER_COMMAND_POOL(where)
#define AV_UNREGISTER_COMMAND_POOL(where)

#define AV_REGISTER_DESCRIPTOR_POOL(where)
#define AV_UNREGISTER_DESCRIPTOR_POOL(where)

#define AV_REGISTER_DEVICE(where)
#define AV_UNREGISTER_DEVICE(where)

#define AV_REGISTER_DEVICE_MEMORY(where)
#define AV_UNREGISTER_DEVICE_MEMORY(where)

#define AV_REGISTER_FENCE(where)
#define AV_UNREGISTER_FENCE(where)

#define AV_REGISTER_FRAMEBUFFER(where)
#define AV_UNREGISTER_FRAMEBUFFER(where)

#define AV_REGISTER_IMAGE(where)
#define AV_UNREGISTER_IMAGE(where)

#define AV_REGISTER_IMAGE_VIEW(where)
#define AV_UNREGISTER_IMAGE_VIEW(where)

#define AV_REGISTER_PIPELINE(where)
#define AV_UNREGISTER_PIPELINE(where)

#define AV_REGISTER_PIPELINE_LAYOUT(where)
#define AV_UNREGISTER_PIPELINE_LAYOUT(where)

#define AV_REGISTER_SEMAPHORE(where)
#define AV_UNREGISTER_SEMAPHORE(where)

#define AV_REGISTER_SHADER_MODULE(where)
#define AV_UNREGISTER_SHADER_MODULE(where)









#define AV_SET_VULKAN_OBJECT_NAME(device, handle, type, ...)

#else

#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <string>
#include <vulkan/vulkan.h>

GX_RESTORE_WARNING_STATE


#define AV_REGISTER_COMMAND_POOL(where) android_vulkan::RegisterCommandPool ( where );
#define AV_UNREGISTER_COMMAND_POOL(where) android_vulkan::UnregisterCommandPool ( where );

#define AV_REGISTER_DESCRIPTOR_POOL(where) android_vulkan::RegisterDescriptorPool ( where );
#define AV_UNREGISTER_DESCRIPTOR_POOL(where) android_vulkan::UnregisterDescriptorPool ( where );

#define AV_REGISTER_DEVICE(where) android_vulkan::RegisterDevice ( where );
#define AV_UNREGISTER_DEVICE(where) android_vulkan::UnregisterDevice ( where );

#define AV_REGISTER_DEVICE_MEMORY(where) android_vulkan::RegisterDeviceMemory ( where );
#define AV_UNREGISTER_DEVICE_MEMORY(where) android_vulkan::UnregisterDeviceMemory ( where );

#define AV_REGISTER_FENCE(where) android_vulkan::RegisterFence ( where );
#define AV_UNREGISTER_FENCE(where) android_vulkan::UnregisterFence ( where );

#define AV_REGISTER_FRAMEBUFFER(where) android_vulkan::RegisterFramebuffer ( where );
#define AV_UNREGISTER_FRAMEBUFFER(where) android_vulkan::UnregisterFramebuffer ( where );

#define AV_REGISTER_IMAGE(where) android_vulkan::RegisterImage ( where );
#define AV_UNREGISTER_IMAGE(where) android_vulkan::UnregisterImage ( where );

#define AV_REGISTER_IMAGE_VIEW(where) android_vulkan::RegisterImageView ( where );
#define AV_UNREGISTER_IMAGE_VIEW(where) android_vulkan::UnregisterImageView ( where );

#define AV_REGISTER_PIPELINE(where) android_vulkan::RegisterPipeline ( where );
#define AV_UNREGISTER_PIPELINE(where) android_vulkan::UnregisterPipeline ( where );

#define AV_REGISTER_PIPELINE_LAYOUT(where) android_vulkan::RegisterPipelineLayout ( where );
#define AV_UNREGISTER_PIPELINE_LAYOUT(where) android_vulkan::UnregisterPipelineLayout ( where );

#define AV_REGISTER_SEMAPHORE(where) android_vulkan::RegisterSemaphore ( where );
#define AV_UNREGISTER_SEMAPHORE(where) android_vulkan::UnregisterSemaphore ( where );

#define AV_REGISTER_SHADER_MODULE(where) android_vulkan::RegisterShaderModule ( where );
#define AV_UNREGISTER_SHADER_MODULE(where) android_vulkan::UnregisterShaderModule ( where );







#define AV_SET_VULKAN_OBJECT_NAME(device, handle, type, ...)                                                        \
{                                                                                                                   \
    char _FuCk_NaMe_[ 256U ];                                                                                       \
    std::snprintf ( _FuCk_NaMe_, std::size ( _FuCk_NaMe_ ), __VA_ARGS__ );                                          \
    android_vulkan::SetVulkanObjectName ( device, reinterpret_cast<uint64_t> ( handle ), type, _FuCk_NaMe_ );       \
}

namespace android_vulkan {

void RegisterCommandPool ( std::string &&where );
void UnregisterCommandPool ( std::string &&where );

void RegisterDescriptorPool ( std::string &&where );
void UnregisterDescriptorPool ( std::string &&where );

void RegisterDevice ( std::string &&where );
void UnregisterDevice ( std::string &&where );

void RegisterDeviceMemory ( std::string &&where );
void UnregisterDeviceMemory ( std::string &&where );

void RegisterFence ( std::string &&where );
void UnregisterFence ( std::string &&where );

void RegisterFramebuffer ( std::string &&where );
void UnregisterFramebuffer ( std::string &&where );

void RegisterImage ( std::string &&where );
void UnregisterImage ( std::string &&where );

void RegisterImageView ( std::string &&where );
void UnregisterImageView ( std::string &&where );

void RegisterPipeline ( std::string &&where );
void UnregisterPipeline ( std::string &&where );

void RegisterPipelineLayout ( std::string &&where );
void UnregisterPipelineLayout ( std::string &&where );

void RegisterSemaphore ( std::string &&where );
void UnregisterSemaphore ( std::string &&where );

void RegisterShaderModule ( std::string &&where );
void UnregisterShaderModule ( std::string &&where );












// Initialization is based on article
// https://www.saschawillems.de/blog/2016/05/28/tutorial-on-using-vulkans-vk_ext_debug_marker-with-renderdoc/
void InitVulkanDebugUtils ( VkInstance instance ) noexcept;

void SetVulkanObjectName ( VkDevice device, uint64_t handle, VkObjectType type, char const *name ) noexcept;

} // namespace android_vulkan

#endif // ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS || ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION


#endif // VULKAN_UTILS_HPP
