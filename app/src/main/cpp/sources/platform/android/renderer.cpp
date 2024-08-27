#include <core.hpp>
#include <logger.hpp>
#include <renderer.hpp>
#include <trace.hpp>
#include <vulkan_utils.hpp>


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

bool Renderer::SelectTargetHardware ( std::string_view const &/*userGPU*/,
    VkPhysicalDevice &targetPhysicalDevice,
    uint32_t &targetQueueFamilyIndex
) const noexcept
{
    // Find physical device with graphic and compute queues.

    constexpr auto target = static_cast<VkFlags> (
        AV_VK_FLAG ( VK_QUEUE_COMPUTE_BIT ) | AV_VK_FLAG ( VK_QUEUE_GRAPHICS_BIT )
    );

    for ( auto const &device : _physicalDeviceInfo )
    {
        auto const &queueFamilyInfo = device.second._queueFamilyInfo;
        size_t const count = queueFamilyInfo.size ();

        for ( size_t i = 0U; i < count; ++i )
        {
            VkFlags const queueFamilyFlags = queueFamilyInfo[ i ].first;

            if ( ( queueFamilyFlags & target ) != target )
                continue;

            targetPhysicalDevice = device.first;
            targetQueueFamilyIndex = static_cast<uint32_t> ( i );

            return true;
        }
    }

    LogError ( "Renderer::SelectTargetHardware - Can't find target hardware!" );
    return false;
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

std::span<size_t const> Renderer::GetRequiredFeatures () noexcept
{
    constexpr static size_t const features[] =
    {
        offsetof ( VkPhysicalDeviceFeatures, textureCompressionASTC_LDR )
    };

    return { features, std::size ( features ) };
}

std::span<std::pair<VkFormat, char const* const> const> Renderer::GetRequiredFormats () noexcept
{
    constexpr static std::pair<VkFormat, char const* const> const formats[] =
    {
        { VK_FORMAT_A2R10G10B10_UNORM_PACK32, "VK_FORMAT_A2R10G10B10_UNORM_PACK32" },
        { VK_FORMAT_ASTC_6x6_SRGB_BLOCK, "VK_FORMAT_ASTC_6x6_SRGB_BLOCK" },
        { VK_FORMAT_ASTC_6x6_UNORM_BLOCK, "VK_FORMAT_ASTC_6x6_UNORM_BLOCK" },
        { VK_FORMAT_D16_UNORM, "VK_FORMAT_D16_UNORM" },
        { VK_FORMAT_D24_UNORM_S8_UINT, "VK_FORMAT_D24_UNORM_S8_UINT" },
        { VK_FORMAT_D32_SFLOAT, "VK_FORMAT_D32_SFLOAT" },
        { VK_FORMAT_R16_SFLOAT, "VK_FORMAT_R16_SFLOAT" },
        { VK_FORMAT_R16G16B16A16_SFLOAT, "VK_FORMAT_R16G16B16A16_SFLOAT" },
        { VK_FORMAT_R8_SRGB, "VK_FORMAT_R8_SRGB" },
        { VK_FORMAT_R8_UNORM, "VK_FORMAT_R8_UNORM" },
        { VK_FORMAT_R8G8B8A8_SRGB, "VK_FORMAT_R8G8B8A8_SRGB" },
        { VK_FORMAT_R8G8B8A8_UNORM, "VK_FORMAT_R8G8B8A8_UNORM" },
        { VK_FORMAT_S8_UINT, "VK_FORMAT_S8_UINT" },
        { VK_FORMAT_X8_D24_UNORM_PACK32, "VK_FORMAT_X8_D24_UNORM_PACK32" }
    };

    return { formats, std::size ( formats ) };
}

} // namespace android_vulkan
