#include <logger.hpp>
#include <vulkan_loader.hpp>
#include <vulkan_utils.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <dlfcn.h>

GX_RESTORE_WARNING_STATE


// X macro technique:
// https://en.wikipedia.org/wiki/X_Macro
#define X(name) PFN_##name name = nullptr;

X ( vkGetInstanceProcAddr )
#include <vulkan_bootstrap.in>
#include <vulkan_device.in>
#include <vulkan_instance.in>

#undef X

namespace android_vulkan {

constexpr static char const VULKAN_LIBRARY[] = "libvulkan.so";

bool VulkanLoader::AcquireBootstrapFunctions () noexcept
{
    if ( _vulkan )
    {
        LogError ( "VulkanLoader::AcquireBootstrapFunctions - Already loaded! Please check business logic." );
        return false;
    }

    constexpr auto const flags = static_cast<int> ( AV_VK_FLAG ( RTLD_NOW ) | AV_VK_FLAG ( RTLD_LOCAL ) );
    _vulkan = dlopen ( VULKAN_LIBRARY, flags );

    if ( !_vulkan )
    {
        LogError ( "VulkanLoader::AcquireBootstrapFunctions - Can't load %s. Error: %s", VULKAN_LIBRARY, dlerror () );
        return false;
    }

    vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr> ( dlsym ( _vulkan, "vkGetInstanceProcAddr" ) );

    if ( !vkGetInstanceProcAddr )
    {
        LogError ( "VulkanLoader::AcquireInstanceFunctions - Can't find AcquireBootstrapFunctions. Error: %s",
            VULKAN_LIBRARY,
            dlerror ()
        );

        if ( !Unload () )
            LogError ( "VulkanLoader::AcquireBootstrapFunctions - Can't unload (branch one)." );

        return false;
    }

// X macro technique:
// https://en.wikipedia.org/wiki/X_Macro
#define X(name)                                                                                     \
    name = reinterpret_cast<PFN_##name> ( vkGetInstanceProcAddr ( VK_NULL_HANDLE, #name ) );        \
                                                                                                    \
    if ( !name )                                                                                    \
    {                                                                                               \
        LogError ( "VulkanLoader::AcquireBootstrapFunctions - Can't acquire " #name "." );          \
                                                                                                    \
        if ( !Unload () )                                                                           \
            LogError ( "VulkanLoader::AcquireBootstrapFunctions - Can't unload (branch two)." );    \
                                                                                                    \
        return false;                                                                               \
    }

#include <vulkan_bootstrap.in>

#undef X

    return true;
}

bool VulkanLoader::AcquireDeviceFunctions ( VkDevice device ) noexcept
{
    if ( !vkGetDeviceProcAddr )
    {
        LogError ( "VulkanLoader::AcquireDeviceFunctions - vkGetDeviceProcAddr is nullptr." );
        return false;
    }

// X macro technique:
// https://en.wikipedia.org/wiki/X_Macro
#define X(name)                                                                             \
    name = reinterpret_cast<PFN_##name> ( vkGetDeviceProcAddr ( device, #name ) );          \
                                                                                            \
    if ( !name )                                                                            \
    {                                                                                       \
        LogError ( "VulkanLoader::AcquireDeviceFunctions - Can't acquire " #name "." );     \
                                                                                            \
        if ( !Unload () )                                                                   \
            LogError ( "VulkanLoader::AcquireDeviceFunctions - Can't unload." );            \
                                                                                            \
        return false;                                                                       \
    }

#include <vulkan_device.in>

#undef X

    return true;
}

bool VulkanLoader::AcquireInstanceFunctions ( VkInstance instance ) noexcept
{
    if ( !vkGetInstanceProcAddr )
    {
        LogError ( "VulkanLoader::AcquireInstanceFunctions - vkGetDeviceProcAddr is nullptr." );
        return false;
    }

// X macro technique:
// https://en.wikipedia.org/wiki/X_Macro
#define X(name)                                                                                     \
    name = reinterpret_cast<PFN_##name> ( vkGetInstanceProcAddr ( instance, #name ) );              \
                                                                                                    \
    if ( !name )                                                                                    \
    {                                                                                               \
        LogError ( "VulkanLoader::AcquireInstanceFunctions - Can't acquire " #name "." );           \
                                                                                                    \
        if ( !Unload () )                                                                           \
            LogError ( "VulkanLoader::AcquireInstanceFunctions - Can't unload." );                  \
                                                                                                    \
        return false;                                                                               \
    }

#include <vulkan_instance.in>

#undef X

    return true;
}

bool VulkanLoader::Unload () noexcept
{
    if ( !_vulkan )
        return true;

// X macro technique:
// https://en.wikipedia.org/wiki/X_Macro
#define X(name) name = nullptr;

    X ( vkGetInstanceProcAddr )

#include <vulkan_bootstrap.in>
#include <vulkan_device.in>
#include <vulkan_instance.in>

#undef X

    bool result = true;

    if ( dlclose ( _vulkan ) )
    {
        LogError ( "VulkanLoader::Unload - Can't unload %s. Error: %s", VULKAN_LIBRARY, dlerror () );
        result = false;
    }

    _vulkan = nullptr;
    return result;
}

} // namespace android_vulkan
