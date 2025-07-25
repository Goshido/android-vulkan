#include <precompiled_headers.hpp>
#include <bitwise.hpp>
#include <core.hpp>
#include <logger.hpp>
#include <renderer.hpp>
#include <trace.hpp>
#include <vulkan_utils.hpp>


namespace android_vulkan {

namespace {

constexpr uint32_t MAJOR = 1U;
constexpr uint32_t MINOR = 1U;
constexpr uint32_t PATCH = 0U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

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

bool Renderer::SelectTargetHardware ( std::string_view const &/*userGPU*/ ) noexcept
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

            VkPhysicalDevice physicalDevice = device.first;
            _physicalDevice = physicalDevice;
            vkGetPhysicalDeviceMemoryProperties ( physicalDevice, &_physicalDeviceMemoryProperties );
            _queueFamilyIndex = static_cast<uint32_t> ( i );
            return true;
        }
    }

    LogError ( "Renderer::SelectTargetHardware - Can't find target hardware!" );
    return false;
}

std::span<char const* const> Renderer::GetDeviceExtensions () noexcept
{
    constexpr static char const* extensions[] =
    {
        VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME,
        VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
        VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME,
        VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME,
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    return { extensions, std::size ( extensions ) };
}

std::span<char const* const> Renderer::GetInstanceExtensions () noexcept
{

    constexpr static char const* extensions[] =
    {

#ifdef AV_ENABLE_VVL

        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,

#endif // AV_ENABLE_VVL

        VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME
    };

    return extensions;
}

std::span<size_t const> Renderer::GetRequiredFeatures () noexcept
{
    constexpr static size_t const features[] =
    {
        offsetof ( VkPhysicalDeviceFeatures, textureCompressionASTC_LDR )
    };

    return features;
}

bool Renderer::CheckRequiredDeviceExtensions ( std::vector<std::string> const &deviceExtensions ) noexcept
{
    std::set<std::string> allExtensions;
    allExtensions.insert ( deviceExtensions.cbegin (), deviceExtensions.cend () );

    LogInfo ( ">>> Checking required device extensions..." );

    // Note bitwise '&' is intentional. All checks must be done to view whole picture.

    return AV_BITWISE ( CheckExtensionScalarBlockLayout ( allExtensions ) ) &
        AV_BITWISE ( CheckExtensionShaderFloat16Int8 ( allExtensions ) ) &
        AV_BITWISE ( CheckExtensionCommon ( allExtensions, VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME ) ) &
        AV_BITWISE ( CheckExtensionCommon ( allExtensions, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME ) ) &
        AV_BITWISE ( CheckExtensionCommon ( allExtensions, VK_KHR_SWAPCHAIN_EXTENSION_NAME ) );
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

    return formats;
}

VkPhysicalDeviceFeatures2 Renderer::GetRequiredPhysicalDeviceFeatures () noexcept
{
    constexpr static VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures separateDSLayoutFeatures
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES,
        .pNext = nullptr,
        .separateDepthStencilLayouts = VK_TRUE
    };

    constexpr static VkPhysicalDeviceMultiviewFeatures multiviewFeatures
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES,
        .pNext = const_cast<VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures*> ( &separateDSLayoutFeatures ),
        .multiview = VK_TRUE,
        .multiviewGeometryShader = VK_FALSE,
        .multiviewTessellationShader = VK_FALSE
    };

    constexpr static VkPhysicalDeviceFloat16Int8FeaturesKHR float16Int8Features
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT16_INT8_FEATURES_KHR,
        .pNext = const_cast<VkPhysicalDeviceMultiviewFeatures*> ( &multiviewFeatures ),
        .shaderFloat16 = VK_TRUE,
        .shaderInt8 = VK_FALSE
    };

    constexpr static VkPhysicalDeviceScalarBlockLayoutFeaturesEXT scalarBlockFeatures
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES_EXT,
        .pNext = const_cast<VkPhysicalDeviceFloat16Int8FeaturesKHR*> ( &float16Int8Features ),
        .scalarBlockLayout = VK_TRUE
    };

    return
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = const_cast<VkPhysicalDeviceScalarBlockLayoutFeaturesEXT*> ( &scalarBlockFeatures ),

        .features
        {
            .robustBufferAccess = VK_FALSE,
            .fullDrawIndexUint32 = VK_TRUE,
            .imageCubeArray = VK_FALSE,
            .independentBlend = VK_FALSE,
            .geometryShader = VK_FALSE,
            .tessellationShader = VK_FALSE,
            .sampleRateShading = VK_FALSE,
            .dualSrcBlend = VK_FALSE,
            .logicOp = VK_FALSE,
            .multiDrawIndirect = VK_FALSE,
            .drawIndirectFirstInstance = VK_FALSE,
            .depthClamp = VK_FALSE,
            .depthBiasClamp = VK_FALSE,
            .fillModeNonSolid = VK_FALSE,
            .depthBounds = VK_FALSE,
            .wideLines = VK_FALSE,
            .largePoints = VK_FALSE,
            .alphaToOne = VK_FALSE,
            .multiViewport = VK_FALSE,
            .samplerAnisotropy = VK_FALSE,
            .textureCompressionETC2 = VK_FALSE,
            .textureCompressionASTC_LDR = VK_TRUE,
            .textureCompressionBC = VK_FALSE,
            .occlusionQueryPrecise = VK_FALSE,
            .pipelineStatisticsQuery = VK_FALSE,
            .vertexPipelineStoresAndAtomics = VK_FALSE,
            .fragmentStoresAndAtomics = VK_FALSE,
            .shaderTessellationAndGeometryPointSize = VK_FALSE,
            .shaderImageGatherExtended = VK_FALSE,
            .shaderStorageImageExtendedFormats = VK_FALSE,
            .shaderStorageImageMultisample = VK_FALSE,
            .shaderStorageImageReadWithoutFormat = VK_FALSE,
            .shaderStorageImageWriteWithoutFormat = VK_FALSE,
            .shaderUniformBufferArrayDynamicIndexing = VK_FALSE,
            .shaderSampledImageArrayDynamicIndexing = VK_FALSE,
            .shaderStorageBufferArrayDynamicIndexing = VK_FALSE,
            .shaderStorageImageArrayDynamicIndexing = VK_FALSE,
            .shaderClipDistance = VK_FALSE,
            .shaderCullDistance = VK_FALSE,
            .shaderFloat64 = VK_FALSE,
            .shaderInt64 = VK_FALSE,
            .shaderInt16 = VK_TRUE,
            .shaderResourceResidency = VK_FALSE,
            .shaderResourceMinLod = VK_FALSE,
            .sparseBinding = VK_FALSE,
            .sparseResidencyBuffer = VK_FALSE,
            .sparseResidencyImage2D = VK_FALSE,
            .sparseResidencyImage3D = VK_FALSE,
            .sparseResidency2Samples = VK_FALSE,
            .sparseResidency4Samples = VK_FALSE,
            .sparseResidency8Samples = VK_FALSE,
            .sparseResidency16Samples = VK_FALSE,
            .sparseResidencyAliased = VK_FALSE,
            .variableMultisampleRate = VK_FALSE,
            .inheritedQueries = VK_FALSE
        }
    };
}

Renderer::Version Renderer::GetRequiredVulkanVersion () noexcept
{
    return
    {
        ._major = MAJOR,
        ._minor = MINOR,
        ._patch = PATCH
    };
}

} // namespace android_vulkan
