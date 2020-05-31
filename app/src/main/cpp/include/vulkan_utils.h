#ifndef VULKAN_UTILS_H
#define VULKAN_UTILS_H


#define AV_DX_ALIGNMENT_BEGIN _Pragma ( "pack ( push, 1 )" )
#define AV_DX_ALIGNMENT_END _Pragma ( "pack ( pop )" )

#define AV_VK_FLAG(x) ( static_cast<uint32_t> ( x ) )

// Note there is two types Vulkan handles:
// VK_DEFINE_HANDLE
// VK_DEFINE_NON_DISPATCHABLE_HANDLE
//
// VK_DEFINE_NON_DISPATCHABLE_HANDLE could be packed structure so in theory some Vulkan create API could return
// identical VK_DEFINE_NON_DISPATCHABLE_HANDLE for different objects. It is implementation dependent. Because of that
// user have to figure out unique ID for Vulkan objects. Good practice is to use source code file name or class name
// with property name to figure out what Vulkan resource is leaked.
// From another hand VK_DEFINE_HANDLE must be unique.
// See https://vulkan.lunarg.com/doc/view/1.1.108.0/mac/chunked_spec/chap2.html#fundamentals-objectmodel-overview


#ifndef ANDROID_VULKAN_DEBUG

#define AV_CHECK_VULKAN_LEAKS()

#define AV_REGISTER_BUFFER(where)
#define AV_UNREGISTER_BUFFER(where)

#define AV_REGISTER_COMMAND_POOL(where)
#define AV_UNREGISTER_COMMAND_POOL(where)

#define AV_REGISTER_DESCRIPTOR_POOL(where)
#define AV_UNREGISTER_DESCRIPTOR_POOL(where)

#define AV_REGISTER_DESCRIPTOR_SET_LAYOUT(where)
#define AV_UNREGISTER_DESCRIPTOR_SET_LAYOUT(where)

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

#define AV_REGISTER_RENDER_PASS(where)
#define AV_UNREGISTER_RENDER_PASS(where)

#define AV_REGISTER_SAMPLER(where)
#define AV_UNREGISTER_SAMPLER(where)

#define AV_REGISTER_SEMAPHORE(where)
#define AV_UNREGISTER_SEMAPHORE(where)

#define AV_REGISTER_SHADER_MODULE(where)
#define AV_UNREGISTER_SHADER_MODULE(where)

#define AV_REGISTER_SURFACE(where)
#define AV_UNREGISTER_SURFACE(where)

#define AV_REGISTER_SWAPCHAIN(where)
#define AV_UNREGISTER_SWAPCHAIN(where)

#else

#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <string>
#include <vulkan_wrapper.h>

GX_RESTORE_WARNING_STATE


#define AV_CHECK_VULKAN_LEAKS() android_vulkan::CheckVulkanLeaks ();

#define AV_REGISTER_BUFFER(where) android_vulkan::RegisterBuffer ( where );
#define AV_UNREGISTER_BUFFER(where) android_vulkan::UnregisterBuffer ( where );

#define AV_REGISTER_COMMAND_POOL(where) android_vulkan::RegisterCommandPool ( where );
#define AV_UNREGISTER_COMMAND_POOL(where) android_vulkan::UnregisterCommandPool ( where );

#define AV_REGISTER_DESCRIPTOR_POOL(where) android_vulkan::RegisterDescriptorPool ( where );
#define AV_UNREGISTER_DESCRIPTOR_POOL(where) android_vulkan::UnregisterDescriptorPool ( where );

#define AV_REGISTER_DESCRIPTOR_SET_LAYOUT(where) android_vulkan::RegisterDescriptorSetLayout ( where );
#define AV_UNREGISTER_DESCRIPTOR_SET_LAYOUT(where) android_vulkan::UnregisterDescriptorSetLayout ( where );

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

#define AV_REGISTER_RENDER_PASS(where) android_vulkan::RegisterRenderPass ( where );
#define AV_UNREGISTER_RENDER_PASS(where) android_vulkan::UnregisterRenderPass ( where );

#define AV_REGISTER_SAMPLER(where) android_vulkan::RegisterSampler ( where );
#define AV_UNREGISTER_SAMPLER(where) android_vulkan::UnregisterSampler ( where );

#define AV_REGISTER_SEMAPHORE(where) android_vulkan::RegisterSemaphore ( where );
#define AV_UNREGISTER_SEMAPHORE(where) android_vulkan::UnregisterSemaphore ( where );

#define AV_REGISTER_SHADER_MODULE(where) android_vulkan::RegisterShaderModule ( where );
#define AV_UNREGISTER_SHADER_MODULE(where) android_vulkan::UnregisterShaderModule ( where );

#define AV_REGISTER_SURFACE(where) android_vulkan::RegisterSurface ( where );
#define AV_UNREGISTER_SURFACE(where) android_vulkan::UnregisterSurface ( where );

#define AV_REGISTER_SWAPCHAIN(where) android_vulkan::RegisterSwapchain ( where );
#define AV_UNREGISTER_SWAPCHAIN(where) android_vulkan::UnregisterSwapchain ( where );

namespace android_vulkan {

void CheckVulkanLeaks ();

void RegisterBuffer ( std::string &&where );
void UnregisterBuffer ( std::string &&where );

void RegisterCommandPool ( std::string &&where );
void UnregisterCommandPool ( std::string &&where );

void RegisterDescriptorPool ( std::string &&where );
void UnregisterDescriptorPool ( std::string &&where );

void RegisterDescriptorSetLayout ( std::string &&where );
void UnregisterDescriptorSetLayout ( std::string &&where );

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

void RegisterRenderPass ( std::string &&where );
void UnregisterRenderPass ( std::string &&where );

void RegisterSampler ( std::string &&where );
void UnregisterSampler ( std::string &&where );

void RegisterSemaphore ( std::string &&where );
void UnregisterSemaphore ( std::string &&where );

void RegisterShaderModule ( std::string &&where );
void UnregisterShaderModule ( std::string &&where );

void RegisterSurface ( std::string &&where );
void UnregisterSurface ( std::string &&where );

void RegisterSwapchain ( std::string &&where );
void UnregisterSwapchain ( std::string &&where );

} // namespace android_vulkan

#endif


#endif // VULKAN_UTILS_H
