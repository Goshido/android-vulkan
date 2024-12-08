#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <vulkan_loader.hpp>
#include <vulkan_utils.hpp>


extern PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;

namespace android_vulkan {

namespace {

constexpr char const VULKAN_LIBRARY[] = "vulkan-1.dll";

} // end of anonymous namespace

bool VulkanLoader::OpenLibrary () noexcept
{
    if ( _vulkan ) [[unlikely]]
    {
        LogError ( "VulkanLoader::OpenLibrary - Already loaded! Please check business logic." );
        return false;
    }

    HMODULE const loader = LoadLibraryA ( VULKAN_LIBRARY );
    _vulkan = static_cast<void*> ( loader );

    if ( !_vulkan ) [[unlikely]]
    {
        LogError ( "VulkanLoader::OpenLibrary - Can't load %s. Error: 0x%08X", VULKAN_LIBRARY,
            GetLastError ()
        );

        return false;
    }

    vkGetInstanceProcAddr = static_cast<PFN_vkGetInstanceProcAddr> (
        static_cast<void*> ( GetProcAddress ( loader, "vkGetInstanceProcAddr" ) )
    );

    if ( vkGetInstanceProcAddr ) [[likely]]
        return true;

    LogError ( "VulkanLoader::OpenLibrary - Can't find AcquireBootstrapFunctions. Error: 0x%08X",
        VULKAN_LIBRARY,
        GetLastError ()
    );

    if ( !Unload () ) [[unlikely]]
        LogError ( "VulkanLoader::OpenLibrary - Can't unload." );

    return false;
}

bool VulkanLoader::CloseLibrary () noexcept
{
    if ( FreeLibrary ( static_cast<HMODULE> ( _vulkan ) ) != 0 ) [[likely]]
    {
        _vulkan = nullptr;
        return true;
    }

    LogError ( "VulkanLoader::CloseLibrary - Can't unload %s. Error: 0x%08X", VULKAN_LIBRARY, GetLastError () );
    return false;
}

} // namespace android_vulkan
