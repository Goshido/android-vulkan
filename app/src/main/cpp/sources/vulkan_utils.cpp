#ifdef ANDROID_VULKAN_DEBUG


#include <vulkan_utils.h>
#include <set>
#include <shared_mutex>
#include "logger.h"


namespace android_vulkan {

constexpr static const char* INDENT = "    ";

static std::shared_timed_mutex      g_Lock;
static std::set<std::string>        g_Buffers;
static std::set<std::string>        g_CommandPools;
static std::set<std::string>        g_DescriptorPools;
static std::set<std::string>        g_DescriptorSetLayouts;
static std::set<std::string>        g_Devices;
static std::set<std::string>        g_DeviceMemory;
static std::set<std::string>        g_Fences;
static std::set<std::string>        g_Framebuffers;
static std::set<std::string>        g_Images;
static std::set<std::string>        g_ImageViews;
static std::set<std::string>        g_Pipelines;
static std::set<std::string>        g_PipelineLayouts;
static std::set<std::string>        g_RenderPasses;
static std::set<std::string>        g_Samplers;
static std::set<std::string>        g_Semaphores;
static std::set<std::string>        g_ShaderModules;
static std::set<std::string>        g_Surfaces;
static std::set<std::string>        g_Swapchains;

static void CheckNonDispatchableObjectLeaks ( const char* objectType, std::set<std::string> &storage )
{
    if ( storage.empty () )
        return;

    LogError ( "AV_CHECK_VULKAN_LEAKS - %s objects were leaked: %zu", objectType, storage.size () );
    LogError ( ">>>" );

    for ( auto const& leak : storage )
        LogWarning ( "%s%s", INDENT, leak.c_str () );

    LogError ( "<<<" );

#ifdef ANDROID_VULKAN_LEAK_STRICT_MODE

    assert ( !"CheckNonDispatchableObjectLeaks triggered!" );

#endif

}

static void RegisterNonDispatchableObject ( const char* method,
    const char* objectType,
    std::string &&where,
    std::set<std::string> &storage
)
{
    std::unique_lock<std::shared_timed_mutex> lock ( g_Lock );
    const auto result = storage.insert ( where );

    if ( result.second )
        return;

    LogError ( "%s - There is some %s with identical ID: %s. Please check logic.", method, objectType, where.c_str () );

#ifdef ANDROID_VULKAN_LEAK_STRICT_MODE

    assert ( !"RegisterNonDispatchableObject triggered!" );

#endif

}

static void UnregisterNonDispatchableObject ( const char* method,
    const char* objectType,
    std::string &&where,
    std::set<std::string> &storage
)
{
    std::unique_lock<std::shared_timed_mutex> lock ( g_Lock );

    if ( storage.erase ( where ) )
        return;

    LogError ( "%s - Can't find %s with ID: %s. Please check logic.",
        method,
        objectType,
        where.c_str ()
    );

#ifdef ANDROID_VULKAN_LEAK_STRICT_MODE

    assert ( !"UnregisterNonDispatchableObject triggered!" );

#endif

}

void CheckVulkanLeaks ()
{
    std::shared_lock<std::shared_timed_mutex> lock ( g_Lock );

    CheckNonDispatchableObjectLeaks ( "Buffer", g_Buffers );
    CheckNonDispatchableObjectLeaks ( "Command pool", g_CommandPools );
    CheckNonDispatchableObjectLeaks ( "Descriptor pool", g_DescriptorPools );
    CheckNonDispatchableObjectLeaks ( "Descriptor set layout", g_DescriptorSetLayouts );
    CheckNonDispatchableObjectLeaks ( "Device", g_Devices );
    CheckNonDispatchableObjectLeaks ( "Device memory", g_DeviceMemory );
    CheckNonDispatchableObjectLeaks ( "Fence", g_Fences );
    CheckNonDispatchableObjectLeaks ( "Framebuffer", g_Framebuffers );
    CheckNonDispatchableObjectLeaks ( "Image", g_Images );
    CheckNonDispatchableObjectLeaks ( "Image view", g_ImageViews );
    CheckNonDispatchableObjectLeaks ( "Pipeline", g_Pipelines );
    CheckNonDispatchableObjectLeaks ( "Pipeline layout", g_PipelineLayouts );
    CheckNonDispatchableObjectLeaks ( "Render pass", g_RenderPasses );
    CheckNonDispatchableObjectLeaks ( "Sampler", g_Samplers );
    CheckNonDispatchableObjectLeaks ( "Semaphore", g_Semaphores );
    CheckNonDispatchableObjectLeaks ( "Shader module", g_ShaderModules );
    CheckNonDispatchableObjectLeaks ( "Surface", g_Surfaces );
    CheckNonDispatchableObjectLeaks ( "Swapchain", g_Swapchains );
}

void RegisterBuffer ( std::string &&where )
{
    RegisterNonDispatchableObject ( "AV_REGISTER_BUFFER",
        "buffer",
        std::move ( where ),
        g_Buffers
    );
}

void UnregisterBuffer ( std::string &&where )
{
    UnregisterNonDispatchableObject ( "AV_UNREGISTER_BUFFER",
        "buffer",
        std::move ( where ),
        g_Buffers
    );
}

void RegisterCommandPool ( std::string &&where )
{
    RegisterNonDispatchableObject ( "AV_REGISTER_COMMAND_POOL",
        "command pool",
        std::move ( where ),
        g_CommandPools
    );
}

void UnregisterCommandPool ( std::string &&where )
{
    UnregisterNonDispatchableObject ( "AV_UNREGISTER_COMMAND_POOL",
        "command pool",
        std::move ( where ),
        g_CommandPools
    );
}

void RegisterDescriptorPool ( std::string &&where )
{
    RegisterNonDispatchableObject ( "AV_REGISTER_DESCRIPTOR_POOL",
        "descriptor pool",
        std::move ( where ),
        g_DescriptorPools
    );
}

void UnregisterDescriptorPool ( std::string &&where )
{
    UnregisterNonDispatchableObject ( "AV_UNREGISTER_DESCRIPTOR_POOL",
        "descriptor pool",
        std::move ( where ),
        g_DescriptorPools
    );
}

void RegisterDescriptorSetLayout ( std::string &&where )
{
    RegisterNonDispatchableObject ( "AV_REGISTER_DESCRIPTOR_SET_LAYOUT",
        "descriptor set layout",
        std::move ( where ),
        g_DescriptorSetLayouts
    );
}

void UnregisterDescriptorSetLayout ( std::string &&where )
{
    UnregisterNonDispatchableObject ( "AV_UNREGISTER_DESCRIPTOR_SET_LAYOUT",
        "descriptor set layout",
        std::move ( where ),
        g_DescriptorSetLayouts
    );
}

void RegisterDevice ( std::string &&where )
{
    RegisterNonDispatchableObject ( "AV_REGISTER_DEVICE",
        "device",
        std::move ( where ),
        g_Devices
    );
}

void UnregisterDevice ( std::string &&where )
{
    UnregisterNonDispatchableObject ( "AV_UNREGISTER_DEVICE",
        "device",
        std::move ( where ),
        g_Devices
    );
}

void RegisterDeviceMemory ( std::string &&where )
{
    RegisterNonDispatchableObject ( "AV_REGISTER_DEVICE_MEMORY",
        "device memory",
        std::move ( where ),
        g_DeviceMemory
    );
}

void UnregisterDeviceMemory ( std::string &&where )
{
    UnregisterNonDispatchableObject ( "AV_UNREGISTER_DEVICE_MEMORY",
        "device memory",
        std::move ( where ),
        g_DeviceMemory
    );
}

void RegisterFence ( std::string &&where )
{
    RegisterNonDispatchableObject ( "AV_REGISTER_FENCE",
        "fence",
        std::move ( where ),
        g_Fences
    );
}

void UnregisterFence ( std::string &&where )
{
    UnregisterNonDispatchableObject ( "AV_UNREGISTER_FENCE",
        "fence",
        std::move ( where ),
        g_Fences
    );
}

void RegisterFramebuffer ( std::string &&where )
{
    RegisterNonDispatchableObject ( "AV_REGISTER_FRAMEBUFFER",
        "framebuffer",
        std::move ( where ),
        g_Framebuffers
    );
}

void UnregisterFramebuffer ( std::string &&where )
{
    UnregisterNonDispatchableObject ( "AV_UNREGISTER_FRAMEBUFFER",
        "framebuffer",
        std::move ( where ),
        g_Framebuffers
    );
}

void RegisterImage ( std::string &&where )
{
    RegisterNonDispatchableObject ( "AV_REGISTER_IMAGE",
        "image",
        std::move ( where ),
        g_Images
    );
}

void UnregisterImage ( std::string &&where )
{
    UnregisterNonDispatchableObject ( "AV_UNREGISTER_IMAGE",
        "image",
        std::move ( where ),
        g_Images
    );
}

void RegisterImageView ( std::string &&where )
{
    RegisterNonDispatchableObject ( "AV_REGISTER_IMAGE_VIEW",
        "image view",
        std::move ( where ),
        g_ImageViews
    );
}

void UnregisterImageView ( std::string &&where )
{
    UnregisterNonDispatchableObject ( "AV_UNREGISTER_IMAGE_VIEW",
        "image view",
        std::move ( where ),
        g_ImageViews
    );
}

void RegisterPipeline ( std::string &&where )
{
    RegisterNonDispatchableObject ( "AV_REGISTER_PIPELINE",
        "pipeline",
        std::move ( where ),
        g_Pipelines
    );
}

void UnregisterPipeline ( std::string &&where )
{
    UnregisterNonDispatchableObject ( "AV_UNREGISTER_PIPELINE",
        "pipeline",
        std::move ( where ),
        g_Pipelines
    );
}

void RegisterPipelineLayout ( std::string &&where )
{
    RegisterNonDispatchableObject ( "AV_REGISTER_PIPELINE_LAYOUT",
        "pipeline layout",
        std::move ( where ),
        g_PipelineLayouts
    );
}

void UnregisterPipelineLayout ( std::string &&where )
{
    UnregisterNonDispatchableObject ( "AV_UNREGISTER_PIPELINE_LAYOUT",
        "pipeline layout",
        std::move ( where ),
        g_PipelineLayouts
    );
}

void RegisterRenderPass ( std::string &&where )
{
    RegisterNonDispatchableObject ( "AV_REGISTER_RENDER_PASS",
        "render pass",
        std::move ( where ),
        g_RenderPasses
    );
}

void UnregisterRenderPass ( std::string &&where )
{
    UnregisterNonDispatchableObject ( "AV_UNREGISTER_RENDER_PASS",
        "render pass",
        std::move ( where ),
        g_RenderPasses
    );
}

void RegisterSampler ( std::string &&where )
{
    RegisterNonDispatchableObject ( "AV_REGISTER_SAMPLER",
        "sampler",
        std::move ( where ),
        g_Samplers
    );
}

void UnregisterSampler ( std::string &&where )
{
    UnregisterNonDispatchableObject ( "AV_UNREGISTER_SAMPLER",
        "sampler",
        std::move ( where ),
        g_Samplers
    );
}

void RegisterSemaphore ( std::string &&where )
{
    RegisterNonDispatchableObject ( "AV_REGISTER_SEMAPHORE",
        "semaphore",
        std::move ( where ),
        g_Semaphores
    );
}

void UnregisterSemaphore ( std::string &&where )
{
    UnregisterNonDispatchableObject ( "AV_UNREGISTER_SEMAPHORE",
        "semaphore",
        std::move ( where ),
        g_Semaphores
    );
}

void RegisterShaderModule ( std::string &&where )
{
    RegisterNonDispatchableObject ( "AV_REGISTER_SHADER_MODULE",
        "shader module",
        std::move ( where ),
        g_ShaderModules
    );
}

void UnregisterShaderModule ( std::string &&where )
{
    UnregisterNonDispatchableObject ( "AV_UNREGISTER_SHADER_MODULE",
        "shader module",
        std::move ( where ),
        g_ShaderModules
    );
}

void RegisterSurface ( std::string &&where )
{
    RegisterNonDispatchableObject ( "AV_REGISTER_SURFACE",
        "surface",
        std::move ( where ),
        g_Surfaces
    );
}

void UnregisterSurface ( std::string &&where )
{
    UnregisterNonDispatchableObject ( "AV_UNREGISTER_SURFACE",
        "surface",
        std::move ( where ),
        g_Surfaces
    );
}

void RegisterSwapchain ( std::string &&where )
{
    RegisterNonDispatchableObject ( "AV_REGISTER_SWAPCHAIN",
        "swapchain",
        std::move ( where ),
        g_Swapchains
    );
}

void UnregisterSwapchain ( std::string &&where )
{
    UnregisterNonDispatchableObject ( "AV_UNREGISTER_SWAPCHAIN",
        "swapchain",
        std::move ( where ),
        g_Swapchains
    );
}

} // namespace android_vulkan


#endif // ANDROID_VULKAN_DEBUG
