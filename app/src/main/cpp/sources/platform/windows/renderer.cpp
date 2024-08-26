#include <os_utils.hpp>
#include <renderer.hpp>
#include <trace.hpp>


namespace android_vulkan {

namespace {

constexpr char const SNAPSHOT_DIRECTORY[] = R"__(%APPDATA%\Goshido Inc\Editor\Vulkan Memory)__";

} // end of anonymous namespace

[[maybe_unused]] void Renderer::MakeVulkanMemorySnapshot () noexcept
{
    AV_TRACE ( "Vulkan memory snapshot" )
    std::filesystem::path const directory = editor::OSUtils::ResolvePath ( SNAPSHOT_DIRECTORY );
    std::filesystem::create_directories ( directory );
    _memoryAllocator.MakeSnapshot ( editor::OSUtils::ToString ( directory ).c_str () );
}

bool Renderer::DeployNativeSurface ( WindowHandle nativeWindow ) noexcept
{
    VkWin32SurfaceCreateInfoKHR const win32SurfaceCreateInfoKHR
    {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0U,
        .hinstance = GetModuleHandleA ( nullptr ),
        .hwnd = reinterpret_cast<HWND> ( nativeWindow )
    };

    return CheckVkResult ( vkCreateWin32SurfaceKHR ( _instance, &win32SurfaceCreateInfoKHR, nullptr, &_surface ),
        "Renderer::DeployNativeSurface",
        "Can't create Vulkan surface"
    );
}

std::span<char const* const> Renderer::GetInstanceExtensions () noexcept
{

#ifdef ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

    constexpr static char const* extensions[] =
    {
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    };

#else

    constexpr static char const* const extensions[] =
    {
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME
    };

#endif // ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

    return { extensions, std::size ( extensions ) };
}

} // namespace android_vulkan
