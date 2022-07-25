#include <vulkan_utils.h>

#ifdef ANDROID_VULKAN_DEBUG

#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>
#include <set>
#include <shared_mutex>

GX_RESTORE_WARNING_STATE

#include "logger.h"


namespace android_vulkan {

constexpr static char const* INDENT = "    ";

class VulkanItem final
{
    private:
        size_t          _instances;
        std::string     _where;

    public:
        VulkanItem () noexcept;

        VulkanItem ( VulkanItem const & ) = default;
        VulkanItem& operator = ( VulkanItem const & ) = delete;

        VulkanItem ( VulkanItem && ) = default;
        VulkanItem& operator = ( VulkanItem && ) = default;

        explicit VulkanItem ( std::string &&where ) noexcept;

        ~VulkanItem () = default;

        void IncrementInstanceCount () noexcept;
        void DecrementInstanceCount () noexcept;
        void GetInfo ( std::string &info ) const noexcept;
        [[nodiscard]] bool IsLastInstance () const noexcept;

        bool operator < ( VulkanItem const &other ) const noexcept;
};

VulkanItem::VulkanItem () noexcept:
    _instances ( 1U ),
    _where {}
{
    // NOTHING
}

VulkanItem::VulkanItem ( std::string &&where ) noexcept:
    _instances ( 1U ),
    _where ( std::move ( where ) )
{
    // NOTHING
}

void VulkanItem::IncrementInstanceCount () noexcept
{
    ++_instances;
}

void VulkanItem::DecrementInstanceCount () noexcept
{
    --_instances;
}

void VulkanItem::GetInfo ( std::string &info ) const noexcept
{
    info = _where + " (instances: " + std::to_string ( static_cast<long long int> ( _instances ) ) + ")";
}

bool VulkanItem::IsLastInstance () const noexcept
{
    return _instances == 1U;
}

bool VulkanItem::operator < ( const VulkanItem &other ) const noexcept
{
    return _where < other._where;
}

//----------------------------------------------------------------------------------------------------------------------

static std::shared_timed_mutex      g_Lock;
static std::set<VulkanItem>         g_Buffers;
static std::set<VulkanItem>         g_CommandPools;
static std::set<VulkanItem>         g_DescriptorPools;
static std::set<VulkanItem>         g_DescriptorSetLayouts;
static std::set<VulkanItem>         g_Devices;
static std::set<VulkanItem>         g_DeviceMemory;
static std::set<VulkanItem>         g_Fences;
static std::set<VulkanItem>         g_Framebuffers;
static std::set<VulkanItem>         g_Images;
static std::set<VulkanItem>         g_ImageViews;
static std::set<VulkanItem>         g_Pipelines;
static std::set<VulkanItem>         g_PipelineLayouts;
static std::set<VulkanItem>         g_RenderPasses;
static std::set<VulkanItem>         g_Samplers;
static std::set<VulkanItem>         g_Semaphores;
static std::set<VulkanItem>         g_ShaderModules;
static std::set<VulkanItem>         g_Surfaces;
static std::set<VulkanItem>         g_Swapchains;

static bool CheckNonDispatchableObjectLeaks ( char const* objectType, std::set<VulkanItem> &storage )
{
    if ( storage.empty () )
        return true;

    LogError ( "AV_CHECK_VULKAN_LEAKS - %s objects were leaked: %zu", objectType, storage.size () );
    LogError ( ">>>" );
    std::string info;

    for ( auto const& leak : storage )
    {
        leak.GetInfo ( info );
        LogError ( "%s%s", INDENT, info.c_str () );
    }

    LogError ( "<<<" );
    return false;
}

static void RegisterNonDispatchableObject ( std::string &&where, std::set<VulkanItem> &storage )
{
    std::unique_lock<std::shared_timed_mutex> lock ( g_Lock );
    auto result = storage.insert ( VulkanItem ( std::move ( where ) ) );

    if ( result.second )
        return;

    auto& item = const_cast<VulkanItem&> ( *result.first );
    item.IncrementInstanceCount ();
}

static void UnregisterNonDispatchableObject ( char const* method,
    char const* objectType,
    std::string &&where,
    std::set<VulkanItem> &storage
)
{
    std::unique_lock<std::shared_timed_mutex> lock ( g_Lock );
    char const* str = where.c_str ();
    auto const findResult = storage.find ( VulkanItem ( std::move ( where ) ) );

    if ( findResult == storage.cend () )
    {
        LogError ( "%s - Can't find %s with ID: %s. Please check logic.",
            method,
            objectType,
            str
        );

#ifdef ANDROID_VULKAN_STRICT_MODE

        assert ( !"UnregisterNonDispatchableObject triggered!" );

#endif

    }

    if ( findResult->IsLastInstance () )
    {
        storage.erase ( findResult );
        return;
    }

    auto& item = const_cast<VulkanItem&> ( *findResult );
    item.DecrementInstanceCount ();
}

void CheckVulkanLeaks ()
{
    std::shared_lock<std::shared_timed_mutex> lock ( g_Lock );

    bool result = CheckNonDispatchableObjectLeaks ( "Buffer", g_Buffers );
    result &= CheckNonDispatchableObjectLeaks ( "Command pool", g_CommandPools );
    result &= CheckNonDispatchableObjectLeaks ( "Descriptor pool", g_DescriptorPools );
    result &= CheckNonDispatchableObjectLeaks ( "Descriptor set layout", g_DescriptorSetLayouts );
    result &= CheckNonDispatchableObjectLeaks ( "Device", g_Devices );
    result &= CheckNonDispatchableObjectLeaks ( "Device memory", g_DeviceMemory );
    result &= CheckNonDispatchableObjectLeaks ( "Fence", g_Fences );
    result &= CheckNonDispatchableObjectLeaks ( "Framebuffer", g_Framebuffers );
    result &= CheckNonDispatchableObjectLeaks ( "Image", g_Images );
    result &= CheckNonDispatchableObjectLeaks ( "Image view", g_ImageViews );
    result &= CheckNonDispatchableObjectLeaks ( "Pipeline", g_Pipelines );
    result &= CheckNonDispatchableObjectLeaks ( "Pipeline layout", g_PipelineLayouts );
    result &= CheckNonDispatchableObjectLeaks ( "Render pass", g_RenderPasses );
    result &= CheckNonDispatchableObjectLeaks ( "Sampler", g_Samplers );
    result &= CheckNonDispatchableObjectLeaks ( "Semaphore", g_Semaphores );
    result &= CheckNonDispatchableObjectLeaks ( "Shader module", g_ShaderModules );
    result &= CheckNonDispatchableObjectLeaks ( "Surface", g_Surfaces );
    result &= CheckNonDispatchableObjectLeaks ( "Swapchain", g_Swapchains );

#ifdef ANDROID_VULKAN_STRICT_MODE

    if ( !result )
        assert ( !"CheckVulkanLeaks triggered!" );

#endif // ANDROID_VULKAN_STRICT_MODE

}

void RegisterBuffer ( std::string &&where )
{
    RegisterNonDispatchableObject ( std::move ( where ), g_Buffers );
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
    RegisterNonDispatchableObject ( std::move ( where ), g_CommandPools );
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
    RegisterNonDispatchableObject ( std::move ( where ), g_DescriptorPools );
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
    RegisterNonDispatchableObject ( std::move ( where ), g_DescriptorSetLayouts );
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
    RegisterNonDispatchableObject ( std::move ( where ), g_Devices );
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
    RegisterNonDispatchableObject ( std::move ( where ), g_DeviceMemory );
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
    RegisterNonDispatchableObject ( std::move ( where ), g_Fences );
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
    RegisterNonDispatchableObject ( std::move ( where ), g_Framebuffers );
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
    RegisterNonDispatchableObject ( std::move ( where ), g_Images );
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
    RegisterNonDispatchableObject ( std::move ( where ), g_ImageViews );
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
    RegisterNonDispatchableObject ( std::move ( where ), g_Pipelines );
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
    RegisterNonDispatchableObject ( std::move ( where ), g_PipelineLayouts );
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
    RegisterNonDispatchableObject ( std::move ( where ), g_RenderPasses );
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
    RegisterNonDispatchableObject ( std::move ( where ), g_Samplers );
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
    RegisterNonDispatchableObject ( std::move ( where ), g_Semaphores );
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
    RegisterNonDispatchableObject ( std::move ( where ), g_ShaderModules );
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
    RegisterNonDispatchableObject ( std::move ( where ), g_Surfaces );
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
    RegisterNonDispatchableObject ( std::move ( where ), g_Swapchains );
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
