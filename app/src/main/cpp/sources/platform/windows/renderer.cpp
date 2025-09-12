#include <precompiled_headers.hpp>
#include <bitwise.hpp>
#include <os_utils.hpp>
#include <logger.hpp>
#include <renderer.hpp>
#include <trace.hpp>
#include <vulkan_utils.hpp>


namespace android_vulkan {

namespace {

constexpr char const SNAPSHOT_DIRECTORY[] = R"__(%APPDATA%\Goshido Inc\Editor\Vulkan Memory)__";

constexpr Renderer::VulkanVersion VERSION
{
    ._major = 1U,
    ._minor = 4U,
    ._patch = 0U,
};

constexpr char const INDENT_2[] = "        ";

//----------------------------------------------------------------------------------------------------------------------

[[nodiscard]] bool CheckExtensionDescriptorBuffer ( VkPhysicalDevice physicalDevice,
    std::set<std::string> const &allExtensions
) noexcept
{
    if ( !Renderer::CheckExtensionCommon ( allExtensions, VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME ) ) [[unlikely]]
        return false;

    VkPhysicalDeviceDescriptorBufferFeaturesEXT hardwareSupport
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT,
        .pNext = nullptr,
        .descriptorBuffer = VK_FALSE,
        .descriptorBufferCaptureReplay = VK_FALSE,
        .descriptorBufferImageLayoutIgnored = VK_FALSE,
        .descriptorBufferPushDescriptors = VK_FALSE
    };

    VkPhysicalDeviceFeatures2 probe
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &hardwareSupport,
        .features {}
    };

    vkGetPhysicalDeviceFeatures2 ( physicalDevice, &probe );

    if ( hardwareSupport.descriptorBuffer ) [[likely]]
    {
        LogInfo ( "%sOK: descriptorBuffer", INDENT_2 );
        return true;
    }

    LogError ( "%sFAIL: descriptorBuffer", INDENT_2 );
    return false;
}

[[nodiscard]] bool CheckExtensionMutableDescriptorType ( VkPhysicalDevice physicalDevice,
    std::set<std::string> const &allExtensions
) noexcept
{
    if ( !Renderer::CheckExtensionCommon ( allExtensions, VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME ) ) [[unlikely]]
        return false;

    VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT hardwareSupport
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT,
        .pNext = nullptr,
        .mutableDescriptorType = VK_FALSE
    };

    VkPhysicalDeviceFeatures2 probe
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &hardwareSupport,
        .features {}
    };

    vkGetPhysicalDeviceFeatures2 ( physicalDevice, &probe );

    if ( hardwareSupport.mutableDescriptorType ) [[likely]]
    {
        LogInfo ( "%sOK: mutableDescriptorType", INDENT_2 );
        return true;
    }

    LogError ( "%sFAIL: mutableDescriptorType", INDENT_2 );
    return false;
}

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

void Renderer::MakeVulkanMemorySnapshot () noexcept
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

bool Renderer::SelectTargetHardware ( std::string_view const &userGPU ) noexcept
{
    // Find physical device with graphic and compute queues. Priority:
    // 1. user specified GPU
    // 2. first discrete GPU
    // 3. first integrated GPU

    struct Candidate final
    {
        VkPhysicalDevice                    _device = VK_NULL_HANDLE;
        VulkanPhysicalDeviceInfo const*     _info = nullptr;
    };

    std::optional<Candidate> user {};
    std::optional<Candidate> discrete {};
    std::optional<Candidate> integrated {};

    for ( auto const &device : _physicalDeviceInfo )
    {
        VulkanPhysicalDeviceInfo const &info = device.second;

        if ( info._deviceName == userGPU )
        {
            user = std::optional<Candidate> (
                Candidate
                {
                    ._device = device.first,
                    ._info = &info
                }
            );

            break;
        }

        if ( !discrete && info._deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU )
        {
            discrete = std::optional<Candidate> (
                Candidate
                {
                    ._device = device.first,
                    ._info = &info
                }
            );
        }

        if ( !integrated && info._deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU )
        {
            integrated = std::optional<Candidate> (
                Candidate
                {
                    ._device = device.first,
                    ._info = &info
                }
            );
        }
    }

    using QueueFamilyInfo = std::vector<std::pair<VkFlags, uint32_t>>;
    QueueFamilyInfo const* queueFamilyInfo;

    auto const grab = [ this ] ( std::optional<Candidate> const &candidate ) noexcept -> QueueFamilyInfo const*
    {
        Candidate const &c = *candidate;
        VkPhysicalDevice physicalDevice = c._device;
        _physicalDevice = physicalDevice;
        vkGetPhysicalDeviceMemoryProperties ( physicalDevice, &_physicalDeviceMemoryProperties );
        VulkanPhysicalDeviceInfo const &info = *c._info;
        _deviceName = info._deviceName;
        return &info._queueFamilyInfo;
    };

    if ( user )
    {
        queueFamilyInfo = grab ( user );
    }
    else if ( discrete )
    {
        queueFamilyInfo = grab ( discrete );
    }
    else if ( integrated )
    {
        queueFamilyInfo = grab ( discrete );
    }
    else
    {
        LogError ( "Renderer::SelectTargetHardware - Can't find target hardware!" );
        return false;
    }

    constexpr auto target = static_cast<VkFlags> (
        AV_VK_FLAG ( VK_QUEUE_COMPUTE_BIT ) | AV_VK_FLAG ( VK_QUEUE_GRAPHICS_BIT )
    );

    std::vector<std::pair<VkFlags, uint32_t>> const &info = *queueFamilyInfo;
    size_t const count = info.size ();

    for ( size_t i = 0U; i < count; ++i )
    {
        if ( ( info[ i ].first & target ) == target )
        {
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

#ifdef AV_ENABLE_NSIGHT

        VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,

#endif // AV_ENABLE_NSIGHT

        VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME,
        VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME,
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    return extensions;
}

std::span<char const* const> Renderer::GetInstanceExtensions () noexcept
{
    constexpr static char const* extensions[] =
    {

#if defined ( AV_ENABLE_VVL ) || defined ( AV_ENABLE_NSIGHT )

        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,

#endif // AV_ENABLE_VVL || AV_ENABLE_NSIGHT

        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    };

    return extensions;
}

bool Renderer::CheckRequiredFeatures ( std::vector<std::string> const &deviceExtensions ) noexcept
{
    LogInfo ( ">>> Checking required device features..." );

    VkPhysicalDeviceVulkan11Features features11
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
        .pNext = nullptr,
        .storageBuffer16BitAccess = VK_FALSE,
        .uniformAndStorageBuffer16BitAccess = VK_FALSE,
        .storagePushConstant16 = VK_FALSE,
        .storageInputOutput16 = VK_FALSE,
        .multiview = VK_FALSE,
        .multiviewGeometryShader = VK_FALSE,
        .multiviewTessellationShader = VK_FALSE,
        .variablePointersStorageBuffer = VK_FALSE,
        .variablePointers = VK_FALSE,
        .protectedMemory = VK_FALSE,
        .samplerYcbcrConversion = VK_FALSE,
        .shaderDrawParameters = VK_FALSE
    };

    VkPhysicalDeviceVulkan12Features features12
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .pNext = const_cast<VkPhysicalDeviceVulkan11Features*> ( &features11 ),
        .samplerMirrorClampToEdge = VK_FALSE,
        .drawIndirectCount = VK_FALSE,
        .storageBuffer8BitAccess = VK_FALSE,
        .uniformAndStorageBuffer8BitAccess = VK_FALSE,
        .storagePushConstant8 = VK_FALSE,
        .shaderBufferInt64Atomics = VK_FALSE,
        .shaderSharedInt64Atomics = VK_FALSE,
        .shaderFloat16 = VK_FALSE,
        .shaderInt8 = VK_FALSE,
        .descriptorIndexing = VK_FALSE,
        .shaderInputAttachmentArrayDynamicIndexing = VK_FALSE,
        .shaderUniformTexelBufferArrayDynamicIndexing = VK_FALSE,
        .shaderStorageTexelBufferArrayDynamicIndexing = VK_FALSE,
        .shaderUniformBufferArrayNonUniformIndexing = VK_FALSE,
        .shaderSampledImageArrayNonUniformIndexing = VK_FALSE,
        .shaderStorageBufferArrayNonUniformIndexing = VK_FALSE,
        .shaderStorageImageArrayNonUniformIndexing = VK_FALSE,
        .shaderInputAttachmentArrayNonUniformIndexing = VK_FALSE,
        .shaderUniformTexelBufferArrayNonUniformIndexing = VK_FALSE,
        .shaderStorageTexelBufferArrayNonUniformIndexing = VK_FALSE,
        .descriptorBindingUniformBufferUpdateAfterBind = VK_FALSE,
        .descriptorBindingSampledImageUpdateAfterBind = VK_FALSE,
        .descriptorBindingStorageImageUpdateAfterBind = VK_FALSE,
        .descriptorBindingStorageBufferUpdateAfterBind = VK_FALSE,
        .descriptorBindingUniformTexelBufferUpdateAfterBind = VK_FALSE,
        .descriptorBindingStorageTexelBufferUpdateAfterBind = VK_FALSE,
        .descriptorBindingUpdateUnusedWhilePending = VK_FALSE,
        .descriptorBindingPartiallyBound = VK_FALSE,
        .descriptorBindingVariableDescriptorCount = VK_FALSE,
        .runtimeDescriptorArray = VK_FALSE,
        .samplerFilterMinmax = VK_FALSE,
        .scalarBlockLayout = VK_FALSE,
        .imagelessFramebuffer = VK_FALSE,
        .uniformBufferStandardLayout = VK_FALSE,
        .shaderSubgroupExtendedTypes = VK_FALSE,
        .separateDepthStencilLayouts = VK_FALSE,
        .hostQueryReset = VK_FALSE,
        .timelineSemaphore = VK_FALSE,
        .bufferDeviceAddress = VK_FALSE,
        .bufferDeviceAddressCaptureReplay = VK_FALSE,
        .bufferDeviceAddressMultiDevice = VK_FALSE,
        .vulkanMemoryModel = VK_FALSE,
        .vulkanMemoryModelDeviceScope = VK_FALSE,
        .vulkanMemoryModelAvailabilityVisibilityChains = VK_FALSE,
        .shaderOutputViewportIndex = VK_FALSE,
        .shaderOutputLayer = VK_FALSE,
        .subgroupBroadcastDynamicId = VK_FALSE
    };

    VkPhysicalDeviceVulkan13Features features13
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &features12,
        .robustImageAccess = VK_FALSE,
        .inlineUniformBlock = VK_FALSE,
        .descriptorBindingInlineUniformBlockUpdateAfterBind = VK_FALSE,
        .pipelineCreationCacheControl = VK_FALSE,
        .privateData = VK_FALSE,
        .shaderDemoteToHelperInvocation = VK_FALSE,
        .shaderTerminateInvocation = VK_FALSE,
        .subgroupSizeControl = VK_FALSE,
        .computeFullSubgroups = VK_FALSE,
        .synchronization2 = VK_FALSE,
        .textureCompressionASTC_HDR = VK_FALSE,
        .shaderZeroInitializeWorkgroupMemory = VK_FALSE,
        .dynamicRendering = VK_FALSE,
        .shaderIntegerDotProduct = VK_FALSE,
        .maintenance4 = VK_FALSE
    };

    VkPhysicalDeviceVulkan14Features features14
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
        .pNext = &features13,
        .globalPriorityQuery = VK_FALSE,
        .shaderSubgroupRotate = VK_FALSE,
        .shaderSubgroupRotateClustered = VK_FALSE,
        .shaderFloatControls2 = VK_FALSE,
        .shaderExpectAssume = VK_FALSE,
        .rectangularLines = VK_FALSE,
        .bresenhamLines = VK_FALSE,
        .smoothLines = VK_FALSE,
        .stippledRectangularLines = VK_FALSE,
        .stippledBresenhamLines = VK_FALSE,
        .stippledSmoothLines = VK_FALSE,
        .vertexAttributeInstanceRateDivisor = VK_FALSE,
        .vertexAttributeInstanceRateZeroDivisor = VK_FALSE,
        .indexTypeUint8 = VK_FALSE,
        .dynamicRenderingLocalRead = VK_FALSE,
        .maintenance5 = VK_FALSE,
        .maintenance6 = VK_FALSE,
        .pipelineProtectedAccess = VK_FALSE,
        .pipelineRobustness = VK_FALSE,
        .hostImageCopy = VK_FALSE,
        .pushDescriptor = VK_FALSE
    };

    VkPhysicalDeviceFeatures2 probe
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &features14,
        .features {}
    };

    vkGetPhysicalDeviceFeatures2 ( _physicalDevice, &probe );
    VkPhysicalDeviceFeatures const &features = probe.features;

    std::set<std::string> allExtensions;
    allExtensions.insert ( deviceExtensions.cbegin (), deviceExtensions.cend () );

    // Note bitwise '&' is intentional. All checks must be done to view whole picture.

    return AV_BITWISE ( CheckExtensionDescriptorBuffer ( _physicalDevice, allExtensions ) ) &
        AV_BITWISE ( CheckExtensionMutableDescriptorType ( _physicalDevice, allExtensions ) ) &
        AV_BITWISE ( CheckExtensionCommon ( allExtensions, VK_KHR_SWAPCHAIN_EXTENSION_NAME ) ) &

#ifdef AV_ENABLE_NSIGHT

        AV_BITWISE ( CheckExtensionCommon ( allExtensions, VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME ) ) &

#endif // AV_ENABLE_NSIGHT

        // FUCK - remove after vertex pulling
        AV_BITWISE ( CheckFeature ( features.fullDrawIndexUint32, "fullDrawIndexUint32" ) ) &

        AV_BITWISE ( CheckFeature ( features.samplerAnisotropy, "samplerAnisotropy" ) ) &
        AV_BITWISE ( CheckFeature ( features.shaderInt16, "shaderInt16" ) ) &
        AV_BITWISE ( CheckFeature ( features.shaderInt64, "shaderInt64" ) ) &

        AV_BITWISE ( CheckFeature ( features.shaderSampledImageArrayDynamicIndexing,
            "shaderSampledImageArrayDynamicIndexing" ) ) &

        AV_BITWISE ( CheckFeature ( features.shaderStorageBufferArrayDynamicIndexing,
            "shaderStorageBufferArrayDynamicIndexing" ) ) &

        AV_BITWISE ( CheckFeature ( features.shaderStorageImageArrayDynamicIndexing,
            "shaderStorageImageArrayDynamicIndexing" ) ) &

        AV_BITWISE ( CheckFeature ( features.textureCompressionBC, "textureCompressionBC" ) ) &

        AV_BITWISE ( CheckFeature ( features11.multiview, "multiview" ) ) &

        AV_BITWISE ( CheckFeature ( features12.bufferDeviceAddress, "bufferDeviceAddress" ) ) &
        AV_BITWISE ( CheckFeature ( features12.descriptorBindingPartiallyBound, "descriptorBindingPartiallyBound" ) ) &
        AV_BITWISE ( CheckFeature ( features12.descriptorIndexing, "descriptorIndexing" ) ) &
        AV_BITWISE ( CheckFeature ( features12.runtimeDescriptorArray, "runtimeDescriptorArray" ) ) &
        AV_BITWISE ( CheckFeature ( features12.scalarBlockLayout, "scalarBlockLayout" ) ) &
        AV_BITWISE ( CheckFeature ( features12.separateDepthStencilLayouts, "separateDepthStencilLayouts" ) ) &
        AV_BITWISE ( CheckFeature ( features12.shaderFloat16, "shaderFloat16" ) ) &

        AV_BITWISE ( CheckFeature ( features12.shaderSampledImageArrayNonUniformIndexing,
            "shaderSampledImageArrayNonUniformIndexing" ) ) &

        AV_BITWISE ( CheckFeature ( features12.shaderStorageBufferArrayNonUniformIndexing,
            "shaderStorageBufferArrayNonUniformIndexing" ) ) &

        AV_BITWISE ( CheckFeature ( features12.shaderStorageImageArrayNonUniformIndexing,
            "shaderStorageImageArrayNonUniformIndexing" ) ) &

        AV_BITWISE ( CheckFeature ( features13.dynamicRendering, "dynamicRendering" ) ) &
        AV_BITWISE ( CheckFeature ( features13.dynamicRendering, "synchronization2" ) ) &

        AV_BITWISE ( CheckFeature ( features14.maintenance5, "maintenance5" ) );
}

void Renderer::GetPlatformFeatureProperties () noexcept
{
    VkPhysicalDeviceDescriptorBufferPropertiesEXT props
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT,
        .pNext = nullptr,
        .combinedImageSamplerDescriptorSingleArray = VK_FALSE,
        .bufferlessPushDescriptors = VK_FALSE,
        .allowSamplerImageViewPostSubmitCreation = VK_FALSE,
        .descriptorBufferOffsetAlignment = 0U,
        .maxDescriptorBufferBindings = 0U,
        .maxResourceDescriptorBufferBindings = 0U,
        .maxSamplerDescriptorBufferBindings = 0U,
        .maxEmbeddedImmutableSamplerBindings = 0U,
        .maxEmbeddedImmutableSamplers = 0U,
        .bufferCaptureReplayDescriptorDataSize = 0U,
        .imageCaptureReplayDescriptorDataSize = 0U,
        .imageViewCaptureReplayDescriptorDataSize = 0U,
        .samplerCaptureReplayDescriptorDataSize = 0U,
        .accelerationStructureCaptureReplayDescriptorDataSize = 0U,
        .samplerDescriptorSize = 0U,
        .combinedImageSamplerDescriptorSize = 0U,
        .sampledImageDescriptorSize = 0U,
        .storageImageDescriptorSize = 0U,
        .uniformTexelBufferDescriptorSize = 0U,
        .robustUniformTexelBufferDescriptorSize = 0U,
        .storageTexelBufferDescriptorSize = 0U,
        .robustStorageTexelBufferDescriptorSize = 0U,
        .uniformBufferDescriptorSize = 0U,
        .robustUniformBufferDescriptorSize = 0U,
        .storageBufferDescriptorSize = 0U,
        .robustStorageBufferDescriptorSize = 0U,
        .inputAttachmentDescriptorSize = 0U,
        .accelerationStructureDescriptorSize = 0U,
        .maxSamplerDescriptorBufferRange = 0U,
        .maxResourceDescriptorBufferRange = 0U,
        .samplerDescriptorBufferAddressSpaceSize = 0U,
        .resourceDescriptorBufferAddressSpaceSize = 0U,
        .descriptorBufferAddressSpaceSize = 0U
    };

    VkPhysicalDeviceProperties2 probe
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
        .pNext = &props,
        .properties {}
    };

    vkGetPhysicalDeviceProperties2 ( _physicalDevice, &probe );
    _descriptorBufferOffsetAlignment = static_cast<size_t> ( props.descriptorBufferOffsetAlignment );
    _samplerDescriptorSize = props.samplerDescriptorSize;
    _sampledImageDescriptorSize = props.sampledImageDescriptorSize;
    _storageBufferDescriptorSize = props.storageBufferDescriptorSize;
    _storageImageDescriptorSize = props.storageImageDescriptorSize;
}

std::span<std::pair<VkFormat, char const* const> const> Renderer::GetRequiredFormats () noexcept
{
    constexpr static std::pair<VkFormat, char const* const> const formats[] =
    {
        { VK_FORMAT_A2R10G10B10_UNORM_PACK32, "VK_FORMAT_A2R10G10B10_UNORM_PACK32" },
        { VK_FORMAT_BC1_RGB_SRGB_BLOCK, "VK_FORMAT_BC1_RGB_SRGB_BLOCK" },
        { VK_FORMAT_BC1_RGB_UNORM_BLOCK, "VK_FORMAT_BC1_RGB_UNORM_BLOCK" },
        { VK_FORMAT_BC1_RGBA_SRGB_BLOCK, "VK_FORMAT_BC1_RGBA_SRGB_BLOCK" },
        { VK_FORMAT_BC1_RGBA_UNORM_BLOCK, "VK_FORMAT_BC1_RGBA_UNORM_BLOCK" },
        { VK_FORMAT_BC2_SRGB_BLOCK, "VK_FORMAT_BC2_SRGB_BLOCK" },
        { VK_FORMAT_BC2_UNORM_BLOCK, "VK_FORMAT_BC2_UNORM_BLOCK" },
        { VK_FORMAT_BC3_SRGB_BLOCK, "VK_FORMAT_BC3_SRGB_BLOCK" },
        { VK_FORMAT_BC3_UNORM_BLOCK, "VK_FORMAT_BC3_UNORM_BLOCK" },
        { VK_FORMAT_BC4_SNORM_BLOCK, "VK_FORMAT_BC4_SNORM_BLOCK" },
        { VK_FORMAT_BC4_UNORM_BLOCK, "VK_FORMAT_BC4_UNORM_BLOCK" },
        { VK_FORMAT_BC5_SNORM_BLOCK, "VK_FORMAT_BC5_SNORM_BLOCK" },
        { VK_FORMAT_BC5_UNORM_BLOCK, "VK_FORMAT_BC5_UNORM_BLOCK" },
        { VK_FORMAT_BC6H_SFLOAT_BLOCK, "VK_FORMAT_BC6H_SFLOAT_BLOCK" },
        { VK_FORMAT_BC6H_UFLOAT_BLOCK, "VK_FORMAT_BC6H_UFLOAT_BLOCK" },
        { VK_FORMAT_BC7_SRGB_BLOCK, "VK_FORMAT_BC7_SRGB_BLOCK" },
        { VK_FORMAT_BC7_UNORM_BLOCK, "VK_FORMAT_BC7_UNORM_BLOCK" },
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
    constexpr static VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT mutableDescriptorTypeFeatures
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT,
        .pNext = nullptr,
        .mutableDescriptorType = VK_TRUE
    };

    constexpr static VkPhysicalDeviceDescriptorBufferFeaturesEXT descriptorBufferFeatures
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT,
        .pNext = const_cast<VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT*> ( &mutableDescriptorTypeFeatures ),
        .descriptorBuffer = VK_TRUE,
        .descriptorBufferCaptureReplay = VK_FALSE,
        .descriptorBufferImageLayoutIgnored = VK_FALSE,
        .descriptorBufferPushDescriptors = VK_FALSE
    };

    constexpr static VkPhysicalDeviceVulkan11Features features11
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
        .pNext = const_cast<VkPhysicalDeviceDescriptorBufferFeaturesEXT*> ( &descriptorBufferFeatures ),
        .storageBuffer16BitAccess = VK_FALSE,
        .uniformAndStorageBuffer16BitAccess = VK_FALSE,
        .storagePushConstant16 = VK_FALSE,
        .storageInputOutput16 = VK_FALSE,
        .multiview = VK_TRUE,
        .multiviewGeometryShader = VK_FALSE,
        .multiviewTessellationShader = VK_FALSE,
        .variablePointersStorageBuffer = VK_FALSE,
        .variablePointers = VK_FALSE,
        .protectedMemory = VK_FALSE,
        .samplerYcbcrConversion = VK_FALSE,
        .shaderDrawParameters = VK_FALSE
    };

    constexpr static VkPhysicalDeviceVulkan12Features features12
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .pNext = const_cast<VkPhysicalDeviceVulkan11Features*> ( &features11 ),
        .samplerMirrorClampToEdge = VK_FALSE,
        .drawIndirectCount = VK_FALSE,
        .storageBuffer8BitAccess = VK_FALSE,
        .uniformAndStorageBuffer8BitAccess = VK_FALSE,
        .storagePushConstant8 = VK_FALSE,
        .shaderBufferInt64Atomics = VK_FALSE,
        .shaderSharedInt64Atomics = VK_FALSE,
        .shaderFloat16 = VK_TRUE,
        .shaderInt8 = VK_FALSE,
        .descriptorIndexing = VK_TRUE,
        .shaderInputAttachmentArrayDynamicIndexing = VK_FALSE,
        .shaderUniformTexelBufferArrayDynamicIndexing = VK_FALSE,
        .shaderStorageTexelBufferArrayDynamicIndexing = VK_FALSE,
        .shaderUniformBufferArrayNonUniformIndexing = VK_FALSE,
        .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
        .shaderStorageBufferArrayNonUniformIndexing = VK_TRUE,
        .shaderStorageImageArrayNonUniformIndexing = VK_TRUE,
        .shaderInputAttachmentArrayNonUniformIndexing = VK_FALSE,
        .shaderUniformTexelBufferArrayNonUniformIndexing = VK_FALSE,
        .shaderStorageTexelBufferArrayNonUniformIndexing = VK_FALSE,
        .descriptorBindingUniformBufferUpdateAfterBind = VK_FALSE,
        .descriptorBindingSampledImageUpdateAfterBind = VK_FALSE,
        .descriptorBindingStorageImageUpdateAfterBind = VK_FALSE,
        .descriptorBindingStorageBufferUpdateAfterBind = VK_FALSE,
        .descriptorBindingUniformTexelBufferUpdateAfterBind = VK_FALSE,
        .descriptorBindingStorageTexelBufferUpdateAfterBind = VK_FALSE,
        .descriptorBindingUpdateUnusedWhilePending = VK_FALSE,
        .descriptorBindingPartiallyBound = VK_TRUE,
        .descriptorBindingVariableDescriptorCount = VK_FALSE,
        .runtimeDescriptorArray = VK_TRUE,
        .samplerFilterMinmax = VK_FALSE,
        .scalarBlockLayout = VK_TRUE,
        .imagelessFramebuffer = VK_FALSE,
        .uniformBufferStandardLayout = VK_FALSE,
        .shaderSubgroupExtendedTypes = VK_FALSE,
        .separateDepthStencilLayouts = VK_TRUE,
        .hostQueryReset = VK_FALSE,
        .timelineSemaphore = VK_FALSE,
        .bufferDeviceAddress = VK_TRUE,
        .bufferDeviceAddressCaptureReplay = VK_FALSE,
        .bufferDeviceAddressMultiDevice = VK_FALSE,
        .vulkanMemoryModel = VK_FALSE,
        .vulkanMemoryModelDeviceScope = VK_FALSE,
        .vulkanMemoryModelAvailabilityVisibilityChains = VK_FALSE,
        .shaderOutputViewportIndex = VK_FALSE,
        .shaderOutputLayer = VK_FALSE,
        .subgroupBroadcastDynamicId = VK_FALSE
    };

    constexpr static VkPhysicalDeviceVulkan13Features features13
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = const_cast<VkPhysicalDeviceVulkan12Features*> ( &features12 ),
        .robustImageAccess = VK_FALSE,
        .inlineUniformBlock = VK_FALSE,
        .descriptorBindingInlineUniformBlockUpdateAfterBind = VK_FALSE,
        .pipelineCreationCacheControl = VK_FALSE,
        .privateData = VK_FALSE,
        .shaderDemoteToHelperInvocation = VK_FALSE,
        .shaderTerminateInvocation = VK_FALSE,
        .subgroupSizeControl = VK_FALSE,
        .computeFullSubgroups = VK_FALSE,
        .synchronization2 = VK_TRUE,
        .textureCompressionASTC_HDR = VK_FALSE,
        .shaderZeroInitializeWorkgroupMemory = VK_FALSE,
        .dynamicRendering = VK_TRUE,
        .shaderIntegerDotProduct = VK_FALSE,
        .maintenance4 = VK_FALSE
    };

    constexpr static VkPhysicalDeviceVulkan14Features features14
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
        .pNext = const_cast<VkPhysicalDeviceVulkan13Features*> ( &features13 ),
        .globalPriorityQuery = VK_FALSE,
        .shaderSubgroupRotate = VK_FALSE,
        .shaderSubgroupRotateClustered = VK_FALSE,
        .shaderFloatControls2 = VK_FALSE,
        .shaderExpectAssume = VK_FALSE,
        .rectangularLines = VK_FALSE,
        .bresenhamLines = VK_FALSE,
        .smoothLines = VK_FALSE,
        .stippledRectangularLines = VK_FALSE,
        .stippledBresenhamLines = VK_FALSE,
        .stippledSmoothLines = VK_FALSE,
        .vertexAttributeInstanceRateDivisor = VK_FALSE,
        .vertexAttributeInstanceRateZeroDivisor = VK_FALSE,
        .indexTypeUint8 = VK_FALSE,
        .dynamicRenderingLocalRead = VK_FALSE,

        // It's needed to remove VkShaderModule.
        .maintenance5 = VK_TRUE,

        .maintenance6 = VK_FALSE,
        .pipelineProtectedAccess = VK_FALSE,
        .pipelineRobustness = VK_FALSE,
        .hostImageCopy = VK_FALSE,
        .pushDescriptor = VK_FALSE
    };

    return
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = const_cast<VkPhysicalDeviceVulkan14Features*> ( &features14 ),

        .features
        {
            .robustBufferAccess = VK_FALSE,

            // FUCK - remove after vertex pulling
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
            .samplerAnisotropy = VK_TRUE,
            .textureCompressionETC2 = VK_FALSE,
            .textureCompressionASTC_LDR = VK_FALSE,
            .textureCompressionBC = VK_TRUE,
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
            .shaderSampledImageArrayDynamicIndexing = VK_TRUE,
            .shaderStorageBufferArrayDynamicIndexing = VK_TRUE,
            .shaderStorageImageArrayDynamicIndexing = VK_TRUE,
            .shaderClipDistance = VK_FALSE,
            .shaderCullDistance = VK_FALSE,
            .shaderFloat64 = VK_FALSE,
            .shaderInt64 = VK_TRUE,
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

Renderer::VulkanVersion Renderer::GetRequiredVulkanVersion () noexcept
{
    return VERSION;
}

} // namespace android_vulkan
