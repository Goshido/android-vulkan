#if defined ( ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS ) ||       \
    defined ( ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION )

#include <av_assert.hpp>
#include <logger.hpp>
#include <renderer.hpp>
#include <vulkan_api.hpp>
#include <vulkan_utils.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <set>
#include <shared_mutex>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

namespace {

class VulkanItem final
{
    private:
        size_t          _instances = 1U;
        std::string     _where;

    public:
        VulkanItem () noexcept;

        VulkanItem ( VulkanItem const & ) = default;
        VulkanItem &operator = ( VulkanItem const & ) = delete;

        VulkanItem ( VulkanItem && ) = default;
        VulkanItem &operator = ( VulkanItem && ) = default;

        explicit VulkanItem ( std::string &&where ) noexcept;

        ~VulkanItem () = default;

        void IncrementInstanceCount () noexcept;
        void DecrementInstanceCount () noexcept;
        void GetInfo ( std::string &info ) const noexcept;
        [[nodiscard]] bool IsLastInstance () const noexcept;

        [[nodiscard]] bool operator < ( VulkanItem const &other ) const noexcept;
};

VulkanItem::VulkanItem () noexcept:
    _where {}
{
    // NOTHING
}

VulkanItem::VulkanItem ( std::string &&where ) noexcept:
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

std::shared_timed_mutex     g_Lock;
std::set<VulkanItem>        g_DescriptorPools;
std::set<VulkanItem>        g_DeviceMemory;
std::set<VulkanItem>        g_Images;
std::set<VulkanItem>        g_ImageViews;
































PFN_vkCmdBeginDebugUtilsLabelEXT    vkCmdBeginDebugUtilsLabelEXT = nullptr;
PFN_vkCmdEndDebugUtilsLabelEXT      vkCmdEndDebugUtilsLabelEXT = nullptr;
PFN_vkSetDebugUtilsObjectNameEXT    vkSetDebugUtilsObjectNameEXT = nullptr;

} // end of anonymous namespace

static void RegisterNonDispatchableObject ( std::string &&where, std::set<VulkanItem> &storage )
{
    std::unique_lock<std::shared_timed_mutex> lock ( g_Lock );
    auto result = storage.insert ( VulkanItem ( std::move ( where ) ) );

    if ( result.second )
        return;

    auto &item = const_cast<VulkanItem &> ( *result.first );
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

        AV_ASSERT ( false )

#endif

    }

    if ( findResult->IsLastInstance () )
    {
        storage.erase ( findResult );
        return;
    }

    auto &item = const_cast<VulkanItem &> ( *findResult );
    item.DecrementInstanceCount ();
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
