#include <logger.hpp>
#include <vulkan_loader.hpp>
#include <vulkan_utils.hpp>


// X macro technique:
// https://en.wikipedia.org/wiki/X_Macro
#define X(name) PFN_##name name = nullptr;

#include <vulkan_bootstrap.in>
#include <vulkan_device.in>
#include <vulkan_instance.in>

#undef X

namespace android_vulkan {

bool VulkanLoader::AcquireBootstrapFunctions () noexcept
{
    if ( !OpenLibrary () ) [[unlikely]]
    {
        LogError ( "VulkanLoader::AcquireBootstrapFunctions - Already loaded! Please check business logic." );
        return false;
    }

// X macro technique:
// https://en.wikipedia.org/wiki/X_Macro
#define X(name)                                                                                     \
    name = reinterpret_cast<PFN_##name> (                                                           \
        reinterpret_cast<void*> ( vkGetInstanceProcAddr ( VK_NULL_HANDLE, #name ) )                 \
    );                                                                                              \
                                                                                                    \
    if ( !name ) [[unlikely]]                                                                       \
    {                                                                                               \
        LogError ( "VulkanLoader::AcquireBootstrapFunctions - Can't acquire " #name "." );          \
                                                                                                    \
        if ( !Unload () ) [[unlikely]]                                                              \
            LogError ( "VulkanLoader::AcquireBootstrapFunctions - Can't unload." );                 \
                                                                                                    \
        return false;                                                                               \
    }

#include <vulkan_bootstrap.in>

#undef X

    return true;
}

bool VulkanLoader::AcquireDeviceFunctions ( VkDevice device ) noexcept
{
    if ( !vkGetDeviceProcAddr ) [[unlikely]]
    {
        LogError ( "VulkanLoader::AcquireDeviceFunctions - vkGetDeviceProcAddr is nullptr." );
        return false;
    }

// X macro technique:
// https://en.wikipedia.org/wiki/X_Macro
#define X(name)                                                                                                     \
    name = reinterpret_cast<PFN_##name> ( reinterpret_cast<void*> ( vkGetDeviceProcAddr ( device, #name ) ) );      \
                                                                                                                    \
    if ( !name ) [[unlikely]]                                                                                       \
    {                                                                                                               \
        LogError ( "VulkanLoader::AcquireDeviceFunctions - Can't acquire " #name "." );                             \
                                                                                                                    \
        if ( !Unload () ) [[unlikely]]                                                                              \
            LogError ( "VulkanLoader::AcquireDeviceFunctions - Can't unload." );                                    \
                                                                                                                    \
        return false;                                                                                               \
    }

#include <vulkan_device.in>

#undef X

    return true;
}

bool VulkanLoader::AcquireInstanceFunctions ( VkInstance instance ) noexcept
{
    if ( !vkGetInstanceProcAddr ) [[unlikely]]
    {
        LogError ( "VulkanLoader::AcquireInstanceFunctions - vkGetDeviceProcAddr is nullptr." );
        return false;
    }

// X macro technique:
// https://en.wikipedia.org/wiki/X_Macro
#define X(name)                                                                                 \
    name = reinterpret_cast<PFN_##name> (                                                       \
        reinterpret_cast<void*> ( vkGetInstanceProcAddr ( instance, #name ) )                   \
    );                                                                                          \
                                                                                                \
    if ( !name ) [[unlikely]]                                                                   \
    {                                                                                           \
        LogError ( "VulkanLoader::AcquireInstanceFunctions - Can't acquire " #name "." );       \
                                                                                                \
        if ( !Unload () ) [[unlikely]]                                                          \
            LogError ( "VulkanLoader::AcquireInstanceFunctions - Can't unload." );              \
                                                                                                \
        return false;                                                                           \
    }

#include <vulkan_instance.in>

#undef X

    return true;
}

bool VulkanLoader::Unload () noexcept
{
    if ( !_vulkan ) [[unlikely]]
        return true;

// X macro technique:
// https://en.wikipedia.org/wiki/X_Macro
#define X(name) name = nullptr;

#include <vulkan_bootstrap.in>
#include <vulkan_device.in>
#include <vulkan_instance.in>

#undef X

    return CloseLibrary ();
}

} // namespace android_vulkan
