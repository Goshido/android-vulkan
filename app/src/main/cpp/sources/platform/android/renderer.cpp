#include <core.hpp>
#include <renderer.hpp>
#include <trace.hpp>


namespace android_vulkan {

[[maybe_unused]] void Renderer::MakeVulkanMemorySnapshot () noexcept
{
    AV_TRACE ( "Vulkan memory snapshot" )
    _memoryAllocator.MakeSnapshot ( Core::GetCacheDirectory ().c_str () );
}

bool Renderer::DeployNativeSurface ( WindowHandle nativeWindow ) noexcept
{
    VkAndroidSurfaceCreateInfoKHR const androidSurfaceCreateInfoKHR
    {
        .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0U,
        .window = reinterpret_cast<ANativeWindow*> ( nativeWindow )
    };

    return CheckVkResult ( vkCreateAndroidSurfaceKHR ( _instance, &androidSurfaceCreateInfoKHR, nullptr, &_surface ),
        "Renderer::DeployNativeSurface",
        "Can't create Vulkan surface"
    );
}

std::span<char const* const> Renderer::GetInstanceExtensions () noexcept
{

#ifdef ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

    constexpr static char const* extensions[] =
    {
        VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    };

#else

    constexpr static char const* const extensions[] =
    {
        VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME
    };

#endif // ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

    return { extensions, std::size ( extensions ) };
}

} // namespace android_vulkan
