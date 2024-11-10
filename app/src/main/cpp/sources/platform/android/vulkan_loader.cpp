#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <vulkan_loader.hpp>
#include <vulkan_utils.hpp>


extern PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;

namespace android_vulkan {

namespace {

constexpr char const VULKAN_LIBRARY[] = "libvulkan.so";

} // end of anonymous namespace

bool VulkanLoader::OpenLibrary () noexcept
{
    if ( _vulkan ) [[unlikely]]
    {
        LogError ( "VulkanLoader::OpenLibrary - Already loaded! Please check business logic." );
        return false;
    }

    constexpr auto const flags = static_cast<int> ( AV_VK_FLAG ( RTLD_NOW ) | AV_VK_FLAG ( RTLD_LOCAL ) );
    _vulkan = dlopen ( VULKAN_LIBRARY, flags );

    if ( !_vulkan ) [[unlikely]]
    {
        LogError ( "VulkanLoader::OpenLibrary - Can't load %s. Error: %s", VULKAN_LIBRARY, dlerror () );
        return false;
    }

    vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr> ( dlsym ( _vulkan, "vkGetInstanceProcAddr" ) );

    if ( vkGetInstanceProcAddr ) [[likely]]
        return true;

    LogError ( "VulkanLoader::OpenLibrary - Can't find AcquireBootstrapFunctions. Error: %s",
        VULKAN_LIBRARY,
        dlerror ()
    );

    if ( !Unload () ) [[unlikely]]
        LogError ( "VulkanLoader::OpenLibrary - Can't unload." );

    return false;
}

bool VulkanLoader::CloseLibrary () noexcept
{
    if ( dlclose ( _vulkan ) == 0 ) [[likely]]
    {
        _vulkan = nullptr;
        return true;
    }

    LogError ( "VulkanLoader::CloseLibrary - Can't unload %s. Error: %s", VULKAN_LIBRARY, dlerror () );
    return false;
}

} // namespace android_vulkan
