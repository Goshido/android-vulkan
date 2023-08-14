#include <av_assert.hpp>
#include <renderer.hpp>
#include <bitwise.hpp>
#include <file.hpp>
#include <vulkan_utils.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <cinttypes>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <unordered_set>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

constexpr static char const* APPLICATION_NAME = "android vulkan";
constexpr static char const* ENGINE_NAME = "renderer";
constexpr static char const* INDENT_1 = "    ";
constexpr static char const* INDENT_2 = "        ";
constexpr static char const* INDENT_3 = "            ";
constexpr static size_t const INITIAL_EXTENSION_STORAGE_SIZE = 64U;

constexpr static uint32_t MAJOR = 1U;
constexpr static uint32_t MINOR = 1U;
constexpr static uint32_t PATCH = 131U;

// Note vulkan_core.h is a little bit dirty from clang-tidy point of view.
// So suppress this third-party mess via "NOLINT" control comment.
constexpr static uint32_t const TARGET_VULKAN_VERSION = VK_MAKE_VERSION ( MAJOR, MINOR, PATCH ); // NOLINT

constexpr static char const* UNKNOWN_RESULT = "UNKNOWN";

//----------------------------------------------------------------------------------------------------------------------

constexpr static std::pair<uint32_t, char const*> const g_vkCompositeAlphaFlagBitsKHRMapper[] =
{
    { static_cast<uint32_t> ( VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR ), "VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR" },
    { static_cast<uint32_t> ( VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR ), "VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR" },

    {
        static_cast<uint32_t> ( VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR ),
        "VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR"
    },

    {
        static_cast<uint32_t> ( VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR ),
        "VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR"
    }
};

constexpr static size_t const g_vkCompositeAlphaFlagBitsKHRMapperItems = std::size (
    g_vkCompositeAlphaFlagBitsKHRMapper
);

constexpr static std::pair<uint32_t, char const*> const g_vkFormatFeatureFlagBitsMapper[] =
{
    { static_cast<uint32_t> ( VK_FORMAT_FEATURE_BLIT_DST_BIT ), "VK_FORMAT_FEATURE_BLIT_DST_BIT" },
    { static_cast<uint32_t> ( VK_FORMAT_FEATURE_BLIT_SRC_BIT ), "VK_FORMAT_FEATURE_BLIT_SRC_BIT" },
    { static_cast<uint32_t> ( VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT ), "VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT" },

    {
        static_cast<uint32_t> ( VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT ),
        "VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT"
    },

    {
        static_cast<uint32_t> ( VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT ),
        "VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT"
    },

    {
        static_cast<uint32_t> ( VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT ),
        "VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT"
    },

    { static_cast<uint32_t> ( VK_FORMAT_FEATURE_DISJOINT_BIT ), "VK_FORMAT_FEATURE_DISJOINT_BIT" },

    {
        static_cast<uint32_t> ( VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT ),
        "VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT"
    },

    {
        static_cast<uint32_t> ( VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT ),
        "VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT"
    },

    { static_cast<uint32_t> ( VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT ), "VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT" },

    {
        static_cast<uint32_t> ( VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_IMG ),
        "VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_IMG"
    },

    {
        static_cast<uint32_t> ( VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT ),
        "VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT"
    },

    {
        static_cast<uint32_t> ( VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT_EXT ),
        "VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT_EXT"
    },

    {
        static_cast<uint32_t> ( VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT ),
        "VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT"
    },

    {
        static_cast<uint32_t> (
            VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT
        ),

        "VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT"
    },

    {
        static_cast<uint32_t> ( VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT ),
        "VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT"
    },

    {
        static_cast<uint32_t> ( VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT ),
        "VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT"
    },

    {
        static_cast<uint32_t> ( VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT ),
        "VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT"
    },

    { static_cast<uint32_t> ( VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT ), "VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT" },

    {
        static_cast<uint32_t> ( VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT ),
        "VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT"
    },

    {
        static_cast<uint32_t> ( VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT ),
        "VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT"
    },

    { static_cast<uint32_t> ( VK_FORMAT_FEATURE_TRANSFER_DST_BIT ), "VK_FORMAT_FEATURE_TRANSFER_DST_BIT" },
    { static_cast<uint32_t> ( VK_FORMAT_FEATURE_TRANSFER_SRC_BIT ), "VK_FORMAT_FEATURE_TRANSFER_SRC_BIT" },

    {
        static_cast<uint32_t> ( VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT ),
        "VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT"
    },

    { static_cast<uint32_t> ( VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT ), "VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT" }
};

constexpr static size_t const g_vkFormatFeatureFlagBitsMapperItems = std::size ( g_vkFormatFeatureFlagBitsMapper );

constexpr static std::pair<uint32_t, char const*> const g_vkMemoryHeapFlagBitsMapper[] =
{
    { static_cast<uint32_t> ( VK_MEMORY_HEAP_DEVICE_LOCAL_BIT ), "VK_MEMORY_HEAP_DEVICE_LOCAL_BIT" },
    { static_cast<uint32_t> ( VK_MEMORY_HEAP_MULTI_INSTANCE_BIT ), "VK_MEMORY_HEAP_MULTI_INSTANCE_BIT" }
};

constexpr static size_t const g_vkMemoryHeapFlagBitsMapperItems = std::size ( g_vkMemoryHeapFlagBitsMapper );

constexpr static std::pair<uint32_t, char const*> const g_vkMemoryPropertyFlagBitsMapper[] =
{
    { static_cast<uint32_t> ( VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ), "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT" },
    { static_cast<uint32_t> ( VK_MEMORY_PROPERTY_HOST_CACHED_BIT ), "VK_MEMORY_PROPERTY_HOST_CACHED_BIT" },
    { static_cast<uint32_t> ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ), "VK_MEMORY_PROPERTY_HOST_COHERENT_BIT" },
    { static_cast<uint32_t> ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ), "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT" },
    { static_cast<uint32_t> ( VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT ), "VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT" },
    { static_cast<uint32_t> ( VK_MEMORY_PROPERTY_PROTECTED_BIT ), "VK_MEMORY_PROPERTY_PROTECTED_BIT" },

    {
        static_cast<uint32_t> ( VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD ),
        "VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD"
    },

    {
        static_cast<uint32_t> ( VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD ),
        "VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD"
    }
};

constexpr static size_t const g_vkMemoryPropertyFlagBitsMapperItems = std::size ( g_vkMemoryPropertyFlagBitsMapper );

constexpr static std::pair<uint32_t, char const*> const g_vkImageUsageFlagBitsMapper[] =
{
    { static_cast<uint32_t> ( VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT ), "VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT" },
    { static_cast<uint32_t> ( VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT ), "VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT" },
    { static_cast<uint32_t> ( VK_IMAGE_USAGE_SAMPLED_BIT ), "VK_IMAGE_USAGE_SAMPLED_BIT" },
    { static_cast<uint32_t> ( VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV ), "VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV" },
    { static_cast<uint32_t> ( VK_IMAGE_USAGE_STORAGE_BIT ), "VK_IMAGE_USAGE_STORAGE_BIT" },
    { static_cast<uint32_t> ( VK_IMAGE_USAGE_TRANSFER_DST_BIT ), "VK_IMAGE_USAGE_TRANSFER_DST_BIT" },
    { static_cast<uint32_t> ( VK_IMAGE_USAGE_TRANSFER_SRC_BIT ), "VK_IMAGE_USAGE_TRANSFER_SRC_BIT" },
    { static_cast<uint32_t> ( VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT ), "VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT" },

    {
        static_cast<uint32_t> ( VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT ),
        "VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT"
    },

    {
        static_cast<uint32_t> ( VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT ),
        "VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT"
    }
};

constexpr static size_t const g_vkImageUsageFlagBitsMapperItems = std::size ( g_vkImageUsageFlagBitsMapper );

constexpr static std::pair<uint32_t, char const*> const g_vkSampleCountFlagMapper[] =
{
    { static_cast<uint32_t> ( VK_SAMPLE_COUNT_1_BIT ), "VK_SAMPLE_COUNT_1_BIT" },
    { static_cast<uint32_t> ( VK_SAMPLE_COUNT_2_BIT ), "VK_SAMPLE_COUNT_2_BIT" },
    { static_cast<uint32_t> ( VK_SAMPLE_COUNT_4_BIT ), "VK_SAMPLE_COUNT_4_BIT" },
    { static_cast<uint32_t> ( VK_SAMPLE_COUNT_8_BIT ), "VK_SAMPLE_COUNT_8_BIT" },
    { static_cast<uint32_t> ( VK_SAMPLE_COUNT_16_BIT ), "VK_SAMPLE_COUNT_16_BIT" },
    { static_cast<uint32_t> ( VK_SAMPLE_COUNT_32_BIT ), "VK_SAMPLE_COUNT_32_BIT" },
    { static_cast<uint32_t> ( VK_SAMPLE_COUNT_64_BIT ), "VK_SAMPLE_COUNT_64_BIT" }
};

constexpr static size_t const g_vkSampleCountFlagMapperItems = std::size ( g_vkSampleCountFlagMapper );

constexpr static std::pair<uint32_t, char const*> const g_vkQueueFlagMapper[] =
{
    { static_cast<uint32_t> ( VK_QUEUE_COMPUTE_BIT ), "VK_QUEUE_COMPUTE_BIT" },
    { static_cast<uint32_t> ( VK_QUEUE_GRAPHICS_BIT ), "VK_QUEUE_GRAPHICS_BIT" },
    { static_cast<uint32_t> ( VK_QUEUE_PROTECTED_BIT ), "VK_QUEUE_PROTECTED_BIT" },
    { static_cast<uint32_t> ( VK_QUEUE_SPARSE_BINDING_BIT ), "VK_QUEUE_SPARSE_BINDING_BIT" },
    { static_cast<uint32_t> ( VK_QUEUE_TRANSFER_BIT ), "VK_QUEUE_TRANSFER_BIT" }
};

constexpr static size_t const g_vkQueueFlagMapperItems = std::size ( g_vkQueueFlagMapper );

constexpr static std::pair<uint32_t, char const*> const g_vkSurfaceTransformFlagBitsKHRMapper[] =
{
    { VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR, "VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR" },
    { VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, "VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR" },
    { VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR, "VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR" },
    { VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR, "VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR" },
    { VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR, "VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR" },
    { VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR, "VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR" },

    {
        VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR,
        "VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR"
    },

    {
        VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR,
        "VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR"
    },

    {
        VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR,
        "VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR"
    }
};

constexpr static size_t const g_vkSurfaceTransformFlagBitsKHRMapperItems = std::size (
    g_vkSurfaceTransformFlagBitsKHRMapper
);

// Kung-Fu: used in Renderer::PrintPhysicalDeviceGroupInfo to print physical device features.
static std::unordered_map<size_t, char const*> const g_vkFeatureMap =
{
    { offsetof ( VkPhysicalDeviceFeatures, alphaToOne ), "alphaToOne" },
    { offsetof ( VkPhysicalDeviceFeatures, depthBiasClamp ), "depthBiasClamp" },
    { offsetof ( VkPhysicalDeviceFeatures, depthBounds ), "depthBounds" },
    { offsetof ( VkPhysicalDeviceFeatures, depthClamp ), "depthClamp" },
    { offsetof ( VkPhysicalDeviceFeatures, drawIndirectFirstInstance ), "drawIndirectFirstInstance" },
    { offsetof ( VkPhysicalDeviceFeatures, dualSrcBlend ), "dualSrcBlend" },
    { offsetof ( VkPhysicalDeviceFeatures, fillModeNonSolid ), "fillModeNonSolid" },
    { offsetof ( VkPhysicalDeviceFeatures, fragmentStoresAndAtomics ), "fragmentStoresAndAtomics" },
    { offsetof ( VkPhysicalDeviceFeatures, fullDrawIndexUint32 ), "fullDrawIndexUint32" },
    { offsetof ( VkPhysicalDeviceFeatures, geometryShader ), "geometryShader" },
    { offsetof ( VkPhysicalDeviceFeatures, imageCubeArray ), "imageCubeArray" },
    { offsetof ( VkPhysicalDeviceFeatures, independentBlend ), "independentBlend" },
    { offsetof ( VkPhysicalDeviceFeatures, inheritedQueries ), "inheritedQueries" },
    { offsetof ( VkPhysicalDeviceFeatures, largePoints ), "largePoints" },
    { offsetof ( VkPhysicalDeviceFeatures, logicOp ), "logicOp" },
    { offsetof ( VkPhysicalDeviceFeatures, multiDrawIndirect ), "multiDrawIndirect" },
    { offsetof ( VkPhysicalDeviceFeatures, multiViewport ), "multiViewport" },
    { offsetof ( VkPhysicalDeviceFeatures, occlusionQueryPrecise ), "occlusionQueryPrecise" },
    { offsetof ( VkPhysicalDeviceFeatures, pipelineStatisticsQuery ), "pipelineStatisticsQuery" },
    { offsetof ( VkPhysicalDeviceFeatures, robustBufferAccess ), "robustBufferAccess" },
    { offsetof ( VkPhysicalDeviceFeatures, sampleRateShading ), "sampleRateShading" },
    { offsetof ( VkPhysicalDeviceFeatures, samplerAnisotropy ), "samplerAnisotropy" },
    { offsetof ( VkPhysicalDeviceFeatures, shaderClipDistance ), "shaderClipDistance" },
    { offsetof ( VkPhysicalDeviceFeatures, shaderCullDistance ), "shaderCullDistance" },
    { offsetof ( VkPhysicalDeviceFeatures, shaderFloat64 ), "shaderFloat64" },
    { offsetof ( VkPhysicalDeviceFeatures, shaderImageGatherExtended ), "shaderImageGatherExtended" },
    { offsetof ( VkPhysicalDeviceFeatures, shaderInt16 ), "shaderInt16" },
    { offsetof ( VkPhysicalDeviceFeatures, shaderInt64 ), "shaderInt64" },
    { offsetof ( VkPhysicalDeviceFeatures, shaderResourceMinLod ), "shaderResourceMinLod" },
    { offsetof ( VkPhysicalDeviceFeatures, shaderResourceResidency ), "shaderResourceResidency" },
    { offsetof ( VkPhysicalDeviceFeatures, shaderStorageImageMultisample ), "shaderStorageImageMultisample" },
    { offsetof ( VkPhysicalDeviceFeatures, sparseBinding ), "sparseBinding" },
    { offsetof ( VkPhysicalDeviceFeatures, sparseResidency16Samples ), "sparseResidency16Samples" },
    { offsetof ( VkPhysicalDeviceFeatures, sparseResidency2Samples ), "sparseResidency2Samples" },
    { offsetof ( VkPhysicalDeviceFeatures, sparseResidency4Samples ), "sparseResidency4Samples" },
    { offsetof ( VkPhysicalDeviceFeatures, sparseResidency8Samples ), "sparseResidency8Samples" },
    { offsetof ( VkPhysicalDeviceFeatures, sparseResidencyAliased ), "sparseResidencyAliased" },
    { offsetof ( VkPhysicalDeviceFeatures, sparseResidencyBuffer ), "sparseResidencyBuffer" },
    { offsetof ( VkPhysicalDeviceFeatures, sparseResidencyImage2D ), "sparseResidencyImage2D" },
    { offsetof ( VkPhysicalDeviceFeatures, sparseResidencyImage3D ), "sparseResidencyImage3D" },
    { offsetof ( VkPhysicalDeviceFeatures, tessellationShader ), "tessellationShader" },
    { offsetof ( VkPhysicalDeviceFeatures, textureCompressionASTC_LDR ), "textureCompressionASTC_LDR" },
    { offsetof ( VkPhysicalDeviceFeatures, textureCompressionBC ), "textureCompressionBC" },
    { offsetof ( VkPhysicalDeviceFeatures, textureCompressionETC2 ), "textureCompressionETC2" },
    { offsetof ( VkPhysicalDeviceFeatures, variableMultisampleRate ), "variableMultisampleRate" },
    { offsetof ( VkPhysicalDeviceFeatures, vertexPipelineStoresAndAtomics ), "vertexPipelineStoresAndAtomics" },
    { offsetof ( VkPhysicalDeviceFeatures, wideLines ), "wideLines" },

    {
        offsetof ( VkPhysicalDeviceFeatures, shaderTessellationAndGeometryPointSize ),
        "shaderTessellationAndGeometryPointSize"
    },

    {
        offsetof ( VkPhysicalDeviceFeatures, shaderStorageImageExtendedFormats ),
        "shaderStorageImageExtendedFormats"
    },

    {
        offsetof ( VkPhysicalDeviceFeatures, shaderStorageImageReadWithoutFormat ),
        "shaderStorageImageReadWithoutFormat"
    },

    {
        offsetof ( VkPhysicalDeviceFeatures, shaderStorageImageWriteWithoutFormat ),
        "shaderStorageImageWriteWithoutFormat"
    },

    {
        offsetof ( VkPhysicalDeviceFeatures, shaderUniformBufferArrayDynamicIndexing ),
        "shaderUniformBufferArrayDynamicIndexing"
    },

    {
        offsetof ( VkPhysicalDeviceFeatures, shaderSampledImageArrayDynamicIndexing ),
        "shaderSampledImageArrayDynamicIndexing"
    },

    {
        offsetof ( VkPhysicalDeviceFeatures, shaderStorageBufferArrayDynamicIndexing ),
        "shaderStorageBufferArrayDynamicIndexing"
    },

    {
        offsetof ( VkPhysicalDeviceFeatures, shaderStorageImageArrayDynamicIndexing ),
        "shaderStorageImageArrayDynamicIndexing"
    }
};

//----------------------------------------------------------------------------------------------------------------------

VulkanPhysicalDeviceInfo::VulkanPhysicalDeviceInfo () noexcept:
    _extensionStorage ( INITIAL_EXTENSION_STORAGE_SIZE ),
    _extensions {},
    _features {},
    _queueFamilyInfo {},
    _surfaceCapabilities {}
{
    // NOTHING
}

//----------------------------------------------------------------------------------------------------------------------

#ifdef ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

// MessageID list of the ignored messages.
static std::unordered_set<uint32_t> const g_validationFilter =
{
    // The following warning was triggered: VKDBGUTILWARN003.
    // Please refer to the Adreno Game Developer Guide for more information:
    // https://developer.qualcomm.com/docs/adreno-gpu/developer-guide/index.html
    // [2023/07/24] Caused by Vulkan Adreno Layer which presents when VK_EXT_debug_utils is enabled.
    // https://developer.qualcomm.com/sites/default/files/docs/adreno-gpu/snapdragon-game-toolkit/gdg/components/vk_adreno_layer.html
    // Qualcomm description: Renderpass is not qualified for multipass due to a given subpass.
    // It happens at UI composition renderpass which can not be multipass by design.
    0x00000000U,

    // Attempting to enable extension VK_EXT_debug_utils, but this extension is intended to support use
    // by applications when debugging and it is strongly recommended that it be otherwise avoided.
    // [2022/07/26] Yeah. I'm pretty aware about that. Thank you.
    0x822806FAU
};

constexpr static std::pair<uint32_t, char const*> const g_vkDebugUtilsMessageSeverityFlagBitsEXTMapper[] =
{
    { VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT, "Verbose" },
    { VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT, "Info" },
    { VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT, "Warning" },
    { VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, "Error" }
};

constexpr static size_t const g_vkDebugUtilsMessageSeverityFlagBitsEXTMapperItems = std::size (
    g_vkDebugUtilsMessageSeverityFlagBitsEXTMapper
);

constexpr static std::pair<uint32_t, char const*> const g_vkDebugUtilsMessageTypeFlagBitsEXTMapper[] =
{
    { VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT, "General" },
    { VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT, "Validation" },
    { VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT, "Performance" }
};

constexpr static size_t const g_vkDebugUtilsMessageTypeFlagBitsEXTMapperItems = std::size (
    g_vkDebugUtilsMessageTypeFlagBitsEXTMapper
);

#endif // ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

std::map<VkColorSpaceKHR, char const*> const Renderer::_vulkanColorSpaceMap =
{
    { VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT, "VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT" },
    { VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT, "VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT" },
    { VK_COLOR_SPACE_BT2020_LINEAR_EXT, "VK_COLOR_SPACE_BT2020_LINEAR_EXT" },
    { VK_COLOR_SPACE_BT709_LINEAR_EXT, "VK_COLOR_SPACE_BT709_LINEAR_EXT" },
    { VK_COLOR_SPACE_BT709_NONLINEAR_EXT, "VK_COLOR_SPACE_BT709_NONLINEAR_EXT" },
    { VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT, "VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT" },
    { VK_COLOR_SPACE_DISPLAY_NATIVE_AMD, "VK_COLOR_SPACE_DISPLAY_NATIVE_AMD" },
    { VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT, "VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT" },
    { VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT, "VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT" },
    { VK_COLOR_SPACE_DOLBYVISION_EXT, "VK_COLOR_SPACE_DOLBYVISION_EXT" },
    { VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT, "VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT" },
    { VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT, "VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT" },
    { VK_COLOR_SPACE_HDR10_HLG_EXT, "VK_COLOR_SPACE_HDR10_HLG_EXT" },
    { VK_COLOR_SPACE_HDR10_ST2084_EXT, "VK_COLOR_SPACE_HDR10_ST2084_EXT" },
    { VK_COLOR_SPACE_PASS_THROUGH_EXT, "VK_COLOR_SPACE_PASS_THROUGH_EXT" },
    { VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, "VK_COLOR_SPACE_SRGB_NONLINEAR_KHR" }
};

std::map<VkCompositeAlphaFlagBitsKHR, char const*> const Renderer::_vulkanCompositeAlphaMap =
{
    { VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR, "VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR" },
    { VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, "VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR" },
    { VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR, "VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR" },
    { VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR, "VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR" }
};

std::map<VkFormat, char const*> const Renderer::_vulkanFormatMap =
{
    { VK_FORMAT_A1R5G5B5_UNORM_PACK16, "VK_FORMAT_A1R5G5B5_UNORM_PACK16" },
    { VK_FORMAT_A2B10G10R10_SINT_PACK32, "VK_FORMAT_A2B10G10R10_SINT_PACK32" },
    { VK_FORMAT_A2B10G10R10_SNORM_PACK32, "VK_FORMAT_A2B10G10R10_SNORM_PACK32" },
    { VK_FORMAT_A2B10G10R10_SSCALED_PACK32, "VK_FORMAT_A2B10G10R10_SSCALED_PACK32" },
    { VK_FORMAT_A2B10G10R10_UINT_PACK32, "VK_FORMAT_A2B10G10R10_UINT_PACK32" },
    { VK_FORMAT_A2B10G10R10_UNORM_PACK32, "VK_FORMAT_A2B10G10R10_UNORM_PACK32" },
    { VK_FORMAT_A2B10G10R10_USCALED_PACK32, "VK_FORMAT_A2B10G10R10_USCALED_PACK32" },
    { VK_FORMAT_A2R10G10B10_SINT_PACK32, "VK_FORMAT_A2R10G10B10_SINT_PACK32" },
    { VK_FORMAT_A2R10G10B10_SNORM_PACK32, "VK_FORMAT_A2R10G10B10_SNORM_PACK32" },
    { VK_FORMAT_A2R10G10B10_SSCALED_PACK32, "VK_FORMAT_A2R10G10B10_SSCALED_PACK32" },
    { VK_FORMAT_A2R10G10B10_UINT_PACK32, "VK_FORMAT_A2R10G10B10_UINT_PACK32" },
    { VK_FORMAT_A2R10G10B10_UNORM_PACK32, "VK_FORMAT_A2R10G10B10_UNORM_PACK32" },
    { VK_FORMAT_A2R10G10B10_USCALED_PACK32, "VK_FORMAT_A2R10G10B10_USCALED_PACK32" },
    { VK_FORMAT_A8B8G8R8_SINT_PACK32, "VK_FORMAT_A8B8G8R8_SINT_PACK32" },
    { VK_FORMAT_A8B8G8R8_SNORM_PACK32, "VK_FORMAT_A8B8G8R8_SNORM_PACK32" },
    { VK_FORMAT_A8B8G8R8_SRGB_PACK32, "VK_FORMAT_A8B8G8R8_SRGB_PACK32" },
    { VK_FORMAT_A8B8G8R8_SSCALED_PACK32, "VK_FORMAT_A8B8G8R8_SSCALED_PACK32" },
    { VK_FORMAT_A8B8G8R8_UINT_PACK32, "VK_FORMAT_A8B8G8R8_UINT_PACK32" },
    { VK_FORMAT_A8B8G8R8_UNORM_PACK32, "VK_FORMAT_A8B8G8R8_UNORM_PACK32" },
    { VK_FORMAT_A8B8G8R8_USCALED_PACK32, "VK_FORMAT_A8B8G8R8_USCALED_PACK32" },
    { VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT, "VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT" },
    { VK_FORMAT_ASTC_10x10_SRGB_BLOCK, "VK_FORMAT_ASTC_10x10_SRGB_BLOCK" },
    { VK_FORMAT_ASTC_10x10_UNORM_BLOCK, "VK_FORMAT_ASTC_10x10_UNORM_BLOCK" },
    { VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT, "VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT" },
    { VK_FORMAT_ASTC_10x5_SRGB_BLOCK, "VK_FORMAT_ASTC_10x5_SRGB_BLOCK" },
    { VK_FORMAT_ASTC_10x5_UNORM_BLOCK, "VK_FORMAT_ASTC_10x5_UNORM_BLOCK" },
    { VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT, "VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT" },
    { VK_FORMAT_ASTC_10x6_SRGB_BLOCK, "VK_FORMAT_ASTC_10x6_SRGB_BLOCK" },
    { VK_FORMAT_ASTC_10x6_UNORM_BLOCK, "VK_FORMAT_ASTC_10x6_UNORM_BLOCK" },
    { VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT, "VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT" },
    { VK_FORMAT_ASTC_10x8_SRGB_BLOCK, "VK_FORMAT_ASTC_10x8_SRGB_BLOCK" },
    { VK_FORMAT_ASTC_10x8_UNORM_BLOCK, "VK_FORMAT_ASTC_10x8_UNORM_BLOCK" },
    { VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT, "VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT" },
    { VK_FORMAT_ASTC_12x10_SRGB_BLOCK, "VK_FORMAT_ASTC_12x10_SRGB_BLOCK" },
    { VK_FORMAT_ASTC_12x10_UNORM_BLOCK, "VK_FORMAT_ASTC_12x10_UNORM_BLOCK" },
    { VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT, "VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT" },
    { VK_FORMAT_ASTC_12x12_SRGB_BLOCK, "VK_FORMAT_ASTC_12x12_SRGB_BLOCK" },
    { VK_FORMAT_ASTC_12x12_UNORM_BLOCK, "VK_FORMAT_ASTC_12x12_UNORM_BLOCK" },
    { VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT, "VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT" },
    { VK_FORMAT_ASTC_4x4_SRGB_BLOCK, "VK_FORMAT_ASTC_4x4_SRGB_BLOCK" },
    { VK_FORMAT_ASTC_4x4_UNORM_BLOCK, "VK_FORMAT_ASTC_4x4_UNORM_BLOCK" },
    { VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT, "VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT" },
    { VK_FORMAT_ASTC_5x4_SRGB_BLOCK, "VK_FORMAT_ASTC_5x4_SRGB_BLOCK" },
    { VK_FORMAT_ASTC_5x4_UNORM_BLOCK, "VK_FORMAT_ASTC_5x4_UNORM_BLOCK" },
    { VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT, "VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT" },
    { VK_FORMAT_ASTC_5x5_SRGB_BLOCK, "VK_FORMAT_ASTC_5x5_SRGB_BLOCK" },
    { VK_FORMAT_ASTC_5x5_UNORM_BLOCK, "VK_FORMAT_ASTC_5x5_UNORM_BLOCK" },
    { VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT, "VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT" },
    { VK_FORMAT_ASTC_6x5_SRGB_BLOCK, "VK_FORMAT_ASTC_6x5_SRGB_BLOCK" },
    { VK_FORMAT_ASTC_6x5_UNORM_BLOCK, "VK_FORMAT_ASTC_6x5_UNORM_BLOCK" },
    { VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT, "VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT" },
    { VK_FORMAT_ASTC_6x6_SRGB_BLOCK, "VK_FORMAT_ASTC_6x6_SRGB_BLOCK" },
    { VK_FORMAT_ASTC_6x6_UNORM_BLOCK, "VK_FORMAT_ASTC_6x6_UNORM_BLOCK" },
    { VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT, "VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT" },
    { VK_FORMAT_ASTC_8x5_SRGB_BLOCK, "VK_FORMAT_ASTC_8x5_SRGB_BLOCK" },
    { VK_FORMAT_ASTC_8x5_UNORM_BLOCK, "VK_FORMAT_ASTC_8x5_UNORM_BLOCK" },
    { VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT, "VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT" },
    { VK_FORMAT_ASTC_8x6_SRGB_BLOCK, "VK_FORMAT_ASTC_8x6_SRGB_BLOCK" },
    { VK_FORMAT_ASTC_8x6_UNORM_BLOCK, "VK_FORMAT_ASTC_8x6_UNORM_BLOCK" },
    { VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT, "VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT" },
    { VK_FORMAT_ASTC_8x8_SRGB_BLOCK, "VK_FORMAT_ASTC_8x8_SRGB_BLOCK" },
    { VK_FORMAT_ASTC_8x8_UNORM_BLOCK, "VK_FORMAT_ASTC_8x8_UNORM_BLOCK" },
    { VK_FORMAT_B10G11R11_UFLOAT_PACK32, "VK_FORMAT_B10G11R11_UFLOAT_PACK32" },
    { VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16, "VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16" },
    { VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16, "VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16" },
    { VK_FORMAT_B16G16R16G16_422_UNORM, "VK_FORMAT_B16G16R16G16_422_UNORM" },
    { VK_FORMAT_B4G4R4A4_UNORM_PACK16, "VK_FORMAT_B4G4R4A4_UNORM_PACK16" },
    { VK_FORMAT_B5G5R5A1_UNORM_PACK16, "VK_FORMAT_B5G5R5A1_UNORM_PACK16" },
    { VK_FORMAT_B5G6R5_UNORM_PACK16, "VK_FORMAT_B5G6R5_UNORM_PACK16" },
    { VK_FORMAT_B8G8R8A8_SINT, "VK_FORMAT_B8G8R8A8_SINT" },
    { VK_FORMAT_B8G8R8A8_SNORM, "VK_FORMAT_B8G8R8A8_SNORM" },
    { VK_FORMAT_B8G8R8A8_SRGB, "VK_FORMAT_B8G8R8A8_SRGB" },
    { VK_FORMAT_B8G8R8A8_SSCALED, "VK_FORMAT_B8G8R8A8_SSCALED" },
    { VK_FORMAT_B8G8R8A8_UINT, "VK_FORMAT_B8G8R8A8_UINT" },
    { VK_FORMAT_B8G8R8A8_UNORM, "VK_FORMAT_B8G8R8A8_UNORM" },
    { VK_FORMAT_B8G8R8A8_USCALED, "VK_FORMAT_B8G8R8A8_USCALED" },
    { VK_FORMAT_B8G8R8G8_422_UNORM, "VK_FORMAT_B8G8R8G8_422_UNORM" },
    { VK_FORMAT_B8G8R8_SINT, "VK_FORMAT_B8G8R8_SINT" },
    { VK_FORMAT_B8G8R8_SNORM, "VK_FORMAT_B8G8R8_SNORM" },
    { VK_FORMAT_B8G8R8_SRGB, "VK_FORMAT_B8G8R8_SRGB" },
    { VK_FORMAT_B8G8R8_SSCALED, "VK_FORMAT_B8G8R8_SSCALED" },
    { VK_FORMAT_B8G8R8_UINT, "VK_FORMAT_B8G8R8_UINT" },
    { VK_FORMAT_B8G8R8_UNORM, "VK_FORMAT_B8G8R8_UNORM" },
    { VK_FORMAT_B8G8R8_USCALED, "VK_FORMAT_B8G8R8_USCALED" },
    { VK_FORMAT_BC1_RGBA_SRGB_BLOCK, "VK_FORMAT_BC1_RGBA_SRGB_BLOCK" },
    { VK_FORMAT_BC1_RGBA_UNORM_BLOCK, "VK_FORMAT_BC1_RGBA_UNORM_BLOCK" },
    { VK_FORMAT_BC1_RGB_SRGB_BLOCK, "VK_FORMAT_BC1_RGB_SRGB_BLOCK" },
    { VK_FORMAT_BC1_RGB_UNORM_BLOCK, "VK_FORMAT_BC1_RGB_UNORM_BLOCK" },
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
    { VK_FORMAT_D16_UNORM_S8_UINT, "VK_FORMAT_D16_UNORM_S8_UINT" },
    { VK_FORMAT_D24_UNORM_S8_UINT, "VK_FORMAT_D24_UNORM_S8_UINT" },
    { VK_FORMAT_D32_SFLOAT, "VK_FORMAT_D32_SFLOAT" },
    { VK_FORMAT_D32_SFLOAT_S8_UINT, "VK_FORMAT_D32_SFLOAT_S8_UINT" },
    { VK_FORMAT_E5B9G9R9_UFLOAT_PACK32, "VK_FORMAT_E5B9G9R9_UFLOAT_PACK32" },
    { VK_FORMAT_EAC_R11G11_SNORM_BLOCK, "VK_FORMAT_EAC_R11G11_SNORM_BLOCK" },
    { VK_FORMAT_EAC_R11G11_UNORM_BLOCK, "VK_FORMAT_EAC_R11G11_UNORM_BLOCK" },
    { VK_FORMAT_EAC_R11_SNORM_BLOCK, "VK_FORMAT_EAC_R11_SNORM_BLOCK" },
    { VK_FORMAT_EAC_R11_UNORM_BLOCK, "VK_FORMAT_EAC_R11_UNORM_BLOCK" },
    { VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK, "VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK" },
    { VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK, "VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK" },
    { VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK, "VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK" },
    { VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK, "VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK" },
    { VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK, "VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK" },
    { VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK, "VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK" },
    { VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16, "VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16" },
    { VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16, "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16" },
    { VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16, "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16" },
    { VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16, "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16" },
    { VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16, "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16" },
    { VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16, "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16" },
    { VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16, "VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16" },
    { VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16, "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16" },
    { VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16, "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16" },
    { VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16, "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16" },
    { VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16, "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16" },
    { VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16, "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16" },
    { VK_FORMAT_G16B16G16R16_422_UNORM, "VK_FORMAT_G16B16G16R16_422_UNORM" },
    { VK_FORMAT_G16_B16R16_2PLANE_420_UNORM, "VK_FORMAT_G16_B16R16_2PLANE_420_UNORM" },
    { VK_FORMAT_G16_B16R16_2PLANE_422_UNORM, "VK_FORMAT_G16_B16R16_2PLANE_422_UNORM" },
    { VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM, "VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM" },
    { VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM, "VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM" },
    { VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM, "VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM" },
    { VK_FORMAT_G8B8G8R8_422_UNORM, "VK_FORMAT_G8B8G8R8_422_UNORM" },
    { VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, "VK_FORMAT_G8_B8R8_2PLANE_420_UNORM" },
    { VK_FORMAT_G8_B8R8_2PLANE_422_UNORM, "VK_FORMAT_G8_B8R8_2PLANE_422_UNORM" },
    { VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM, "VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM" },
    { VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM, "VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM" },
    { VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM, "VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM" },
    { VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG, "VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG" },
    { VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG, "VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG" },
    { VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG, "VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG" },
    { VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG, "VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG" },
    { VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG, "VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG" },
    { VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG, "VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG" },
    { VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG, "VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG" },
    { VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG, "VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG" },
    { VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16, "VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16" },
    { VK_FORMAT_R10X6G10X6_UNORM_2PACK16, "VK_FORMAT_R10X6G10X6_UNORM_2PACK16" },
    { VK_FORMAT_R10X6_UNORM_PACK16, "VK_FORMAT_R10X6_UNORM_PACK16" },
    { VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16, "VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16" },
    { VK_FORMAT_R12X4G12X4_UNORM_2PACK16, "VK_FORMAT_R12X4G12X4_UNORM_2PACK16" },
    { VK_FORMAT_R12X4_UNORM_PACK16, "VK_FORMAT_R12X4_UNORM_PACK16" },
    { VK_FORMAT_R16G16B16A16_SFLOAT, "VK_FORMAT_R16G16B16A16_SFLOAT" },
    { VK_FORMAT_R16G16B16A16_SINT, "VK_FORMAT_R16G16B16A16_SINT" },
    { VK_FORMAT_R16G16B16A16_SNORM, "VK_FORMAT_R16G16B16A16_SNORM" },
    { VK_FORMAT_R16G16B16A16_SSCALED, "VK_FORMAT_R16G16B16A16_SSCALED" },
    { VK_FORMAT_R16G16B16A16_UINT, "VK_FORMAT_R16G16B16A16_UINT" },
    { VK_FORMAT_R16G16B16A16_UNORM, "VK_FORMAT_R16G16B16A16_UNORM" },
    { VK_FORMAT_R16G16B16A16_USCALED, "VK_FORMAT_R16G16B16A16_USCALED" },
    { VK_FORMAT_R16G16B16_SFLOAT, "VK_FORMAT_R16G16B16_SFLOAT" },
    { VK_FORMAT_R16G16B16_SINT, "VK_FORMAT_R16G16B16_SINT" },
    { VK_FORMAT_R16G16B16_SNORM, "VK_FORMAT_R16G16B16_SNORM" },
    { VK_FORMAT_R16G16B16_SSCALED, "VK_FORMAT_R16G16B16_SSCALED" },
    { VK_FORMAT_R16G16B16_UINT, "VK_FORMAT_R16G16B16_UINT" },
    { VK_FORMAT_R16G16B16_UNORM, "VK_FORMAT_R16G16B16_UNORM" },
    { VK_FORMAT_R16G16B16_USCALED, "VK_FORMAT_R16G16B16_USCALED" },
    { VK_FORMAT_R16G16_SFLOAT, "VK_FORMAT_R16G16_SFLOAT" },
    { VK_FORMAT_R16G16_SINT, "VK_FORMAT_R16G16_SINT" },
    { VK_FORMAT_R16G16_SNORM, "VK_FORMAT_R16G16_SNORM" },
    { VK_FORMAT_R16G16_SSCALED, "VK_FORMAT_R16G16_SSCALED" },
    { VK_FORMAT_R16G16_UINT, "VK_FORMAT_R16G16_UINT" },
    { VK_FORMAT_R16G16_UNORM, "VK_FORMAT_R16G16_UNORM" },
    { VK_FORMAT_R16G16_USCALED, "VK_FORMAT_R16G16_USCALED" },
    { VK_FORMAT_R16_SFLOAT, "VK_FORMAT_R16_SFLOAT" },
    { VK_FORMAT_R16_SINT, "VK_FORMAT_R16_SINT" },
    { VK_FORMAT_R16_SNORM, "VK_FORMAT_R16_SNORM" },
    { VK_FORMAT_R16_SSCALED, "VK_FORMAT_R16_SSCALED" },
    { VK_FORMAT_R16_UINT, "VK_FORMAT_R16_UINT" },
    { VK_FORMAT_R16_UNORM, "VK_FORMAT_R16_UNORM" },
    { VK_FORMAT_R16_USCALED, "VK_FORMAT_R16_USCALED" },
    { VK_FORMAT_R32G32B32A32_SFLOAT, "VK_FORMAT_R32G32B32A32_SFLOAT" },
    { VK_FORMAT_R32G32B32A32_SINT, "VK_FORMAT_R32G32B32A32_SINT" },
    { VK_FORMAT_R32G32B32A32_UINT, "VK_FORMAT_R32G32B32A32_UINT" },
    { VK_FORMAT_R32G32B32_SFLOAT, "VK_FORMAT_R32G32B32_SFLOAT" },
    { VK_FORMAT_R32G32B32_SINT, "VK_FORMAT_R32G32B32_SINT" },
    { VK_FORMAT_R32G32B32_UINT, "VK_FORMAT_R32G32B32_UINT" },
    { VK_FORMAT_R32G32_SFLOAT, "VK_FORMAT_R32G32_SFLOAT" },
    { VK_FORMAT_R32G32_SINT, "VK_FORMAT_R32G32_SINT" },
    { VK_FORMAT_R32G32_UINT, "VK_FORMAT_R32G32_UINT" },
    { VK_FORMAT_R32_SFLOAT, "VK_FORMAT_R32_SFLOAT" },
    { VK_FORMAT_R32_SINT, "VK_FORMAT_R32_SINT" },
    { VK_FORMAT_R32_UINT, "VK_FORMAT_R32_UINT" },
    { VK_FORMAT_R4G4B4A4_UNORM_PACK16, "VK_FORMAT_R4G4B4A4_UNORM_PACK16" },
    { VK_FORMAT_R4G4_UNORM_PACK8, "VK_FORMAT_R4G4_UNORM_PACK8" },
    { VK_FORMAT_R5G5B5A1_UNORM_PACK16, "VK_FORMAT_R5G5B5A1_UNORM_PACK16" },
    { VK_FORMAT_R5G6B5_UNORM_PACK16, "VK_FORMAT_R5G6B5_UNORM_PACK16" },
    { VK_FORMAT_R64G64B64A64_SFLOAT, "VK_FORMAT_R64G64B64A64_SFLOAT" },
    { VK_FORMAT_R64G64B64A64_SINT, "VK_FORMAT_R64G64B64A64_SINT" },
    { VK_FORMAT_R64G64B64A64_UINT, "VK_FORMAT_R64G64B64A64_UINT" },
    { VK_FORMAT_R64G64B64_SFLOAT, "VK_FORMAT_R64G64B64_SFLOAT" },
    { VK_FORMAT_R64G64B64_SINT, "VK_FORMAT_R64G64B64_SINT" },
    { VK_FORMAT_R64G64B64_UINT, "VK_FORMAT_R64G64B64_UINT" },
    { VK_FORMAT_R64G64_SFLOAT, "VK_FORMAT_R64G64_SFLOAT" },
    { VK_FORMAT_R64G64_SINT, "VK_FORMAT_R64G64_SINT" },
    { VK_FORMAT_R64G64_UINT, "VK_FORMAT_R64G64_UINT" },
    { VK_FORMAT_R64_SFLOAT, "VK_FORMAT_R64_SFLOAT" },
    { VK_FORMAT_R64_SINT, "VK_FORMAT_R64_SINT" },
    { VK_FORMAT_R64_UINT, "VK_FORMAT_R64_UINT" },
    { VK_FORMAT_R8G8B8A8_SINT, "VK_FORMAT_R8G8B8A8_SINT" },
    { VK_FORMAT_R8G8B8A8_SNORM, "VK_FORMAT_R8G8B8A8_SNORM" },
    { VK_FORMAT_R8G8B8A8_SRGB, "VK_FORMAT_R8G8B8A8_SRGB" },
    { VK_FORMAT_R8G8B8A8_SSCALED, "VK_FORMAT_R8G8B8A8_SSCALED" },
    { VK_FORMAT_R8G8B8A8_UINT, "VK_FORMAT_R8G8B8A8_UINT" },
    { VK_FORMAT_R8G8B8A8_UNORM, "VK_FORMAT_R8G8B8A8_UNORM" },
    { VK_FORMAT_R8G8B8A8_USCALED, "VK_FORMAT_R8G8B8A8_USCALED" },
    { VK_FORMAT_R8G8B8_SINT, "VK_FORMAT_R8G8B8_SINT" },
    { VK_FORMAT_R8G8B8_SNORM, "VK_FORMAT_R8G8B8_SNORM" },
    { VK_FORMAT_R8G8B8_SRGB, "VK_FORMAT_R8G8B8_SRGB" },
    { VK_FORMAT_R8G8B8_SSCALED, "VK_FORMAT_R8G8B8_SSCALED" },
    { VK_FORMAT_R8G8B8_UINT, "VK_FORMAT_R8G8B8_UINT" },
    { VK_FORMAT_R8G8B8_UNORM, "VK_FORMAT_R8G8B8_UNORM" },
    { VK_FORMAT_R8G8B8_USCALED, "VK_FORMAT_R8G8B8_USCALED" },
    { VK_FORMAT_R8G8_SINT, "VK_FORMAT_R8G8_SINT" },
    { VK_FORMAT_R8G8_SNORM, "VK_FORMAT_R8G8_SNORM" },
    { VK_FORMAT_R8G8_SRGB, "VK_FORMAT_R8G8_SRGB" },
    { VK_FORMAT_R8G8_SSCALED, "VK_FORMAT_R8G8_SSCALED" },
    { VK_FORMAT_R8G8_UINT, "VK_FORMAT_R8G8_UINT" },
    { VK_FORMAT_R8G8_UNORM, "VK_FORMAT_R8G8_UNORM" },
    { VK_FORMAT_R8G8_USCALED, "VK_FORMAT_R8G8_USCALED" },
    { VK_FORMAT_R8_SINT, "VK_FORMAT_R8_SINT" },
    { VK_FORMAT_R8_SNORM, "VK_FORMAT_R8_SNORM" },
    { VK_FORMAT_R8_SRGB, "VK_FORMAT_R8_SRGB" },
    { VK_FORMAT_R8_SSCALED, "VK_FORMAT_R8_SSCALED" },
    { VK_FORMAT_R8_UINT, "VK_FORMAT_R8_UINT" },
    { VK_FORMAT_R8_UNORM, "VK_FORMAT_R8_UNORM" },
    { VK_FORMAT_R8_USCALED, "VK_FORMAT_R8_USCALED" },
    { VK_FORMAT_S8_UINT, "VK_FORMAT_S8_UINT" },
    { VK_FORMAT_UNDEFINED, "VK_FORMAT_UNDEFINED" },
    { VK_FORMAT_X8_D24_UNORM_PACK32, "VK_FORMAT_X8_D24_UNORM_PACK32" }
};

std::map<VkPhysicalDeviceType, char const*> const Renderer::_vulkanPhysicalDeviceTypeMap =
{
    { VK_PHYSICAL_DEVICE_TYPE_CPU, "VK_PHYSICAL_DEVICE_TYPE_CPU" },
    { VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU" },
    { VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU, "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU" },
    { VK_PHYSICAL_DEVICE_TYPE_OTHER, "VK_PHYSICAL_DEVICE_TYPE_OTHER" },
    { VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU, "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU" }
};

std::map<VkPresentModeKHR, char const*> const Renderer::_vulkanPresentModeMap =
{
    { VK_PRESENT_MODE_FIFO_KHR, "VK_PRESENT_MODE_FIFO_KHR" },
    { VK_PRESENT_MODE_FIFO_RELAXED_KHR, "VK_PRESENT_MODE_FIFO_RELAXED_KHR" },
    { VK_PRESENT_MODE_IMMEDIATE_KHR, "VK_PRESENT_MODE_IMMEDIATE_KHR" },
    { VK_PRESENT_MODE_MAILBOX_KHR, "VK_PRESENT_MODE_MAILBOX_KHR" },
    { VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR, "VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR" },
    { VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR, "VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR" }
};

std::map<VkResult, char const*> const Renderer::_vulkanResultMap =
{
    { VK_ERROR_DEVICE_LOST, "VK_ERROR_DEVICE_LOST" },
    { VK_ERROR_EXTENSION_NOT_PRESENT, "VK_ERROR_EXTENSION_NOT_PRESENT" },
    { VK_ERROR_FEATURE_NOT_PRESENT, "VK_ERROR_FEATURE_NOT_PRESENT" },
    { VK_ERROR_FRAGMENTATION_EXT, "VK_ERROR_FRAGMENTATION_EXT" },
    { VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT, "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT" },
    { VK_ERROR_INCOMPATIBLE_DRIVER, "VK_ERROR_INCOMPATIBLE_DRIVER" },
    { VK_ERROR_INITIALIZATION_FAILED, "VK_ERROR_INITIALIZATION_FAILED" },
    { VK_ERROR_INVALID_EXTERNAL_HANDLE, "VK_ERROR_INVALID_EXTERNAL_HANDLE" },
    { VK_ERROR_INVALID_SHADER_NV, "VK_ERROR_INVALID_SHADER_NV" },
    { VK_ERROR_LAYER_NOT_PRESENT, "VK_ERROR_LAYER_NOT_PRESENT" },
    { VK_ERROR_MEMORY_MAP_FAILED, "VK_ERROR_MEMORY_MAP_FAILED" },
    { VK_ERROR_NATIVE_WINDOW_IN_USE_KHR, "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR" },
    { VK_ERROR_OUT_OF_DATE_KHR, "VK_ERROR_OUT_OF_DATE_KHR" },
    { VK_ERROR_OUT_OF_DEVICE_MEMORY, "VK_ERROR_OUT_OF_DEVICE_MEMORY" },
    { VK_ERROR_OUT_OF_HOST_MEMORY, "VK_ERROR_OUT_OF_HOST_MEMORY" },
    { VK_ERROR_OUT_OF_POOL_MEMORY, "VK_ERROR_OUT_OF_POOL_MEMORY" },
    { VK_ERROR_SURFACE_LOST_KHR, "VK_ERROR_SURFACE_LOST_KHR" },
    { VK_ERROR_TOO_MANY_OBJECTS, "VK_ERROR_TOO_MANY_OBJECTS" },
    { VK_SUBOPTIMAL_KHR, "VK_SUBOPTIMAL_KHR" }
};

std::map<VkSurfaceTransformFlagsKHR, char const*> const Renderer::_vulkanSurfaceTransformMap =
{
    { VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR, "VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR" },

    {
        VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR,
        "VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR"
    },

    {
        VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR,
        "VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR"
    },

    {
        VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR,
        "VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR"
    },

    { VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, "VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR" },
    { VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR, "VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR" },
    { VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR, "VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR" },
    { VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR, "VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR" },
    { VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR, "VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR" }
};

//----------------------------------------------------------------------------------------------------------------------

Renderer::Renderer () noexcept:
    _depthImageFormat ( VK_FORMAT_UNDEFINED ),
    _depthStencilImageFormat ( VK_FORMAT_UNDEFINED ),
    _device ( VK_NULL_HANDLE ),
    _dpi ( 96.0F ),
    _instance ( VK_NULL_HANDLE ),
    _isDeviceExtensionChecked ( false ),
    _isDeviceExtensionSupported ( false ),
    _maxUniformBufferRange {},
    _memoryAllocator {},
    _physicalDevice ( VK_NULL_HANDLE ),
    _queue ( VK_NULL_HANDLE ),
    _queueFamilyIndex ( VK_QUEUE_FAMILY_IGNORED ),
    _surface ( VK_NULL_HANDLE ),
    _surfaceFormat ( VK_FORMAT_UNDEFINED ),
    _surfaceSize { .width = 0U, .height = 0U },
    _surfaceTransform ( VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR ),
    _swapchain ( VK_NULL_HANDLE ),

#ifdef ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

    vkCreateDebugUtilsMessengerEXT ( nullptr ),
    vkDestroyDebugUtilsMessengerEXT ( nullptr ),
    _debugUtilsMessenger ( VK_NULL_HANDLE ),
    _debugUtilsMessengerCreateInfo {},

#endif // ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

    _viewportResolution { .width = 0U, .height = 0U },
    _physicalDeviceGroups {},
    _physicalDeviceInfo {},
    _physicalDeviceMemoryProperties {},
    _surfaceFormats {},
    _swapchainImages {},
    _swapchainImageViews {},
    _presentationEngineTransform {},
    _vulkanLoader {}
{
    // NOTHING
}

bool Renderer::CheckSwapchainStatus () noexcept
{
    VkSurfaceCapabilitiesKHR caps {};

    bool tmp = CheckVkResult (
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR ( _physicalDevice, _surface, &caps ),
        "Renderer::CheckSwapchainStatus",
        "Can't get Vulkan surface capabilities"
    );

    if ( !tmp )
        return false;

    if ( _surfaceTransform != caps.currentTransform )
        tmp = false;

    if ( std::memcmp ( &_surfaceSize, &caps.currentExtent, sizeof ( _surfaceSize ) ) != 0 )
        tmp = false;

    return tmp;
}

bool Renderer::CreateShader ( VkShaderModule &shader,
    std::string &&shaderFile,
    char const* errorMessage
) const noexcept
{
    File vertexShader ( shaderFile );

    if ( !vertexShader.LoadContent () )
        return false;

    std::vector<uint8_t> const &spirV = vertexShader.GetContent ();

    VkShaderModuleCreateInfo const shaderModuleCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .codeSize = spirV.size (),
        .pCode = reinterpret_cast<uint32_t const*> ( spirV.data () )
    };

    return CheckVkResult ( vkCreateShaderModule ( _device, &shaderModuleCreateInfo, nullptr, &shader ),
        "Renderer::CreateShader",
        errorMessage
    );
}

bool Renderer::FinishAllJobs () noexcept
{
    return android_vulkan::Renderer::CheckVkResult ( vkQueueWaitIdle ( _queue ),
        "Renderer::FinishAllJobs",
        "Can't wait queue idle"
    );
}

VkFormat Renderer::GetDefaultDepthFormat () const noexcept
{
    return _depthImageFormat;
}

VkFormat Renderer::GetDefaultDepthStencilFormat () const noexcept
{
    return _depthStencilImageFormat;
}

VkDevice Renderer::GetDevice () const noexcept
{
    return _device;
}

float Renderer::GetDPI () const noexcept
{
    return _dpi;
}

size_t Renderer::GetMaxUniformBufferRange () const noexcept
{
    return _maxUniformBufferRange;
}

size_t Renderer::GetPresentImageCount () const noexcept
{
    return _swapchainImageViews.size ();
}

VkImageView const &Renderer::GetPresentImageView ( size_t imageIndex ) const noexcept
{
    return _swapchainImageViews[ imageIndex ];
}

GXMat4 const &Renderer::GetPresentationEngineTransform () const noexcept
{
    return _presentationEngineTransform;
}

VkQueue Renderer::GetQueue () const noexcept
{
    return _queue;
}

uint32_t Renderer::GetQueueFamilyIndex () const noexcept
{
    return _queueFamilyIndex;
}

VkFormat Renderer::GetSurfaceFormat () const noexcept
{
    return _surfaceFormat;
}

VkExtent2D const &Renderer::GetSurfaceSize () const noexcept
{
    return _surfaceSize;
}

VkSwapchainKHR &Renderer::GetSwapchain () noexcept
{
    return _swapchain;
}

VkExtent2D const &Renderer::GetViewportResolution () const noexcept
{
    return _viewportResolution;
}

bool Renderer::OnCreateSwapchain ( ANativeWindow &nativeWindow, bool vSync ) noexcept
{
    if ( !DeploySurface ( nativeWindow ) )
        return false;

    if ( DeploySwapchain ( vSync ) )
        return true;

    DestroySurface ();
    return false;
}

void Renderer::OnDestroySwapchain () noexcept
{
    DestroySwapchain ();
    DestroySurface ();
}

bool Renderer::OnCreateDevice ( float dpi ) noexcept
{
    if ( !_vulkanLoader.AcquireBootstrapFunctions () )
        return false;

    if ( !PrintInstanceLayerInfo () )
        return false;

    if ( !DeployInstance () )
        return false;

#ifdef ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

    if ( !DeployDebugFeatures () )
        return false;

#endif

    uint32_t physicalDeviceCount = 0U;
    vkEnumeratePhysicalDevices ( _instance, &physicalDeviceCount, nullptr );

    if ( !physicalDeviceCount )
    {

#ifdef ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

        DestroyDebugFeatures ();

#endif

        DestroyInstance ();

        LogError ( "Renderer::OnCreateDevice - There is no any Vulkan physical device." );
        return false;
    }

    LogInfo ( "Renderer::OnInit - Vulkan physical devices detected: %u.", physicalDeviceCount );

    std::vector<VkPhysicalDevice> physicalDevices ( static_cast<size_t> ( physicalDeviceCount ) );
    VkPhysicalDevice* deviceList = physicalDevices.data ();

    bool result = CheckVkResult ( vkEnumeratePhysicalDevices ( _instance, &physicalDeviceCount, deviceList ),
        "Renderer::OnCreateDevice",
        "Can't get Vulkan physical devices"
    );

    if ( !result )
    {

#ifdef ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

        DestroyDebugFeatures ();

#endif

        DestroyInstance ();
        return false;
    }

    for ( uint32_t i = 0U; i < physicalDeviceCount; ++i )
    {
        if ( PrintPhysicalDeviceInfo ( i, deviceList[ i ] ) )
            continue;

        _physicalDeviceInfo.clear ();

#ifdef ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

        DestroyDebugFeatures ();

#endif

        DestroyInstance ();
        return false;
    }

    uint32_t physicalDeviceGroupCount = 0U;
    vkEnumeratePhysicalDeviceGroups ( _instance, &physicalDeviceGroupCount, nullptr );

    if ( !physicalDeviceGroupCount )
    {
        _physicalDeviceInfo.clear ();

#ifdef ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

        DestroyDebugFeatures ();

#endif

        DestroyInstance ();

        LogError ( "Renderer::OnCreateDevice - There is no any Vulkan physical device groups." );
        return false;
    }

    LogInfo ( "Renderer::OnInit - Vulkan physical devices groups detected: %u.", physicalDeviceGroupCount );

    _physicalDeviceGroups.resize ( static_cast<size_t> ( physicalDeviceGroupCount ) );
    VkPhysicalDeviceGroupProperties* groupProps = _physicalDeviceGroups.data ();

    for ( auto &item : _physicalDeviceGroups )
        item.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES;

    result = CheckVkResult ( vkEnumeratePhysicalDeviceGroups ( _instance, &physicalDeviceGroupCount, groupProps ),
        "Renderer::OnCreateDevice",
        "Can't get Vulkan physical device groups"
    );

    if ( !result )
    {
        _physicalDeviceGroups.clear ();
        _physicalDeviceInfo.clear ();

#ifdef ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

        DestroyDebugFeatures ();

#endif

        DestroyInstance ();
        return false;
    }

    for ( uint32_t i = 0U; i < physicalDeviceGroupCount; ++i )
        PrintPhysicalDeviceGroupInfo ( i, groupProps[ i ] );

    if ( DeployDevice () )
    {
        _dpi = dpi;
        return true;
    }

    _physicalDeviceGroups.clear ();
    _physicalDeviceInfo.clear ();

#ifdef ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

    DestroyDebugFeatures ();

#endif

    DestroyInstance ();
    return false;
}

void Renderer::OnDestroyDevice () noexcept
{
    if ( !CheckVkResult ( vkDeviceWaitIdle ( _device ), "Renderer::OnDestroyDevice", "Can't wait device idle" ) )
        return;

    _physicalDeviceGroups.clear ();
    _physicalDeviceInfo.clear ();

    DestroyDevice ();

#ifdef ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

    DestroyDebugFeatures ();

#endif

    DestroyInstance ();

    if ( !_vulkanLoader.Unload () )
    {
        LogError ( "Renderer::OnDestroyDevice - Can't unload Vulkan functions." );
    }
}

bool Renderer::TryAllocateMemory ( VkDeviceMemory &memory,
    VkDeviceSize &offset,
    VkMemoryRequirements const &requirements,
    VkMemoryPropertyFlags memoryProperties,
    char const* errorMessage
) noexcept
{
    if ( _memoryAllocator.TryAllocateMemory ( memory, offset, _device, requirements, memoryProperties ) )
        return true;

    LogError ( "Renderer::TryAllocateMemory - %s.", errorMessage );
    return false;
}

void Renderer::FreeMemory ( VkDeviceMemory memory, VkDeviceSize offset ) noexcept
{
    _memoryAllocator.FreeMemory ( _device, memory, offset );
}

bool Renderer::MapMemory ( void* &ptr,
    VkDeviceMemory memory,
    VkDeviceSize offset,
    char const* from,
    char const* message
) noexcept
{
    return _memoryAllocator.MapMemory ( ptr, _device, memory, offset, from, message );
}

void Renderer::UnmapMemory ( VkDeviceMemory memory ) noexcept
{
    _memoryAllocator.UnmapMemory ( _device, memory );
}

[[maybe_unused]] void Renderer::MakeVulkanMemorySnapshot () noexcept
{
    _memoryAllocator.MakeSnapshot ();
}

bool Renderer::CheckVkResult ( VkResult result, char const* from, char const* message ) noexcept
{
    if ( result == VK_SUCCESS )
        return true;

    LogError ( "%s - %s. Error: %s.", from, message, ResolveVkResult ( result ) );
    return false;
}

VkImageAspectFlags Renderer::ResolveImageViewAspect ( VkFormat format ) noexcept
{
    static std::unordered_map<VkFormat, VkImageAspectFlags> const mapper =
    {
        { VK_FORMAT_D16_UNORM, VK_IMAGE_ASPECT_DEPTH_BIT },
        { VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT },

        {
            VK_FORMAT_D16_UNORM_S8_UINT,
            AV_VK_FLAG ( VK_IMAGE_ASPECT_DEPTH_BIT ) | AV_VK_FLAG ( VK_IMAGE_ASPECT_STENCIL_BIT )
        },

        {
            VK_FORMAT_D24_UNORM_S8_UINT,
            AV_VK_FLAG ( VK_IMAGE_ASPECT_DEPTH_BIT ) | AV_VK_FLAG ( VK_IMAGE_ASPECT_STENCIL_BIT )
        },

        {
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            AV_VK_FLAG ( VK_IMAGE_ASPECT_DEPTH_BIT ) | AV_VK_FLAG ( VK_IMAGE_ASPECT_STENCIL_BIT )
        },

        { VK_FORMAT_S8_UINT, VK_IMAGE_ASPECT_STENCIL_BIT },
        { VK_FORMAT_X8_D24_UNORM_PACK32, VK_IMAGE_ASPECT_DEPTH_BIT }
    };

    auto const findResult = mapper.find ( format );
    return findResult == mapper.cend () ? VK_IMAGE_ASPECT_COLOR_BIT : findResult->second;
}

char const* Renderer::ResolveVkFormat ( VkFormat format ) noexcept
{
    auto const findResult = _vulkanFormatMap.find ( format );
    return findResult == _vulkanFormatMap.cend () ? UNKNOWN_RESULT : findResult->second;
}

bool Renderer::CheckExtensionShaderFloat16Int8 ( std::set<std::string> const &allExtensions ) noexcept
{
    if ( !CheckExtensionCommon ( allExtensions, VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME ) )
        return false;

    VkPhysicalDeviceFloat16Int8FeaturesKHR hardwareSupport
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT16_INT8_FEATURES_KHR,
        .pNext = nullptr,
        .shaderFloat16 = VK_FALSE,
        .shaderInt8 = VK_FALSE
    };

    VkPhysicalDeviceFeatures2 probe
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &hardwareSupport,
        .features {}
    };

    vkGetPhysicalDeviceFeatures2 ( _physicalDevice, &probe );

    if ( hardwareSupport.shaderFloat16 )
    {
        LogInfo ( "%sOK: shaderFloat16", INDENT_2 );
        return true;
    }

    LogError ( "%sFAIL: shaderFloat16", INDENT_2 );
    return false;
}

bool Renderer::CheckRequiredDeviceExtensions ( std::vector<char const*> const &deviceExtensions ) noexcept
{
    if ( _isDeviceExtensionChecked )
        return _isDeviceExtensionSupported;

    std::set<std::string> allExtensions;
    allExtensions.insert ( deviceExtensions.cbegin (), deviceExtensions.cend () );

    LogInfo ( ">>> Checking required device extensions..." );

    // Note bitwise '&' is intentional. All checks must be done to view whole picture.

    _isDeviceExtensionSupported = AV_BITWISE ( CheckExtensionShaderFloat16Int8 ( allExtensions ) ) &
        AV_BITWISE ( CheckExtensionCommon ( allExtensions, VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME ) ) &
        AV_BITWISE ( CheckExtensionCommon ( allExtensions, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME ) ) &
        AV_BITWISE ( CheckExtensionCommon ( allExtensions, VK_KHR_SWAPCHAIN_EXTENSION_NAME ) );

    _isDeviceExtensionChecked = true;
    return _isDeviceExtensionSupported;
}

bool Renderer::CheckRequiredFeatures ( VkPhysicalDevice physicalDevice, size_t const* features, size_t count ) noexcept
{
    auto const &featureInfo = _physicalDeviceInfo[ physicalDevice ];

    LogInfo ( "Renderer::CheckRequiredFeatures - Checking required features..." );

    // std::set is for alphabetical ordering.
    std::set<std::string_view> supportedFeatures;
    std::set<std::string_view> unsupportedFeatures;

    for ( size_t i = 0U; i < count; ++i )
    {
        size_t const offset = features[ i ];

        auto const enable = *reinterpret_cast<VkBool32 const*> (
            reinterpret_cast<uint8_t const*> ( &featureInfo._features ) + offset
        );

        auto const &featureName = g_vkFeatureMap.find ( offset )->second;

        if ( enable )
        {
            supportedFeatures.emplace ( featureName );
            continue;
        }

        unsupportedFeatures.emplace ( featureName );
    }

    for ( auto const &feature : supportedFeatures )
        LogInfo ( "%sOK: %s", INDENT_1, feature.data () );

    for ( auto const &feature : unsupportedFeatures )
        LogError ( "%sFAIL: %s", INDENT_1, feature.data () );

    return unsupportedFeatures.empty ();
}

bool Renderer::CheckRequiredFormats () noexcept
{
    LogInfo ( "Renderer::CheckRequiredFormats - Checking required formats..." );
    std::vector<char const*> unsupportedFormats;

    auto probe = [&] ( VkFormat format, char const* name ) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties ( _physicalDevice, format, &props );

        if ( !props.linearTilingFeatures && !props.optimalTilingFeatures && !props.bufferFeatures )
        {
            unsupportedFormats.push_back ( name );
            return;
        }

        LogInfo ( "%sOK: %s", INDENT_1, name );

        PrintVkFlagsProp ( INDENT_2,
            "linearTilingFeatures",
            props.linearTilingFeatures,
            g_vkFormatFeatureFlagBitsMapperItems,
            g_vkFormatFeatureFlagBitsMapper
        );

        PrintVkFlagsProp ( INDENT_2,
            "optimalTilingFeatures",
            props.optimalTilingFeatures,
            g_vkFormatFeatureFlagBitsMapperItems,
            g_vkFormatFeatureFlagBitsMapper
        );

        PrintVkFlagsProp ( INDENT_2,
            "bufferFeatures",
            props.bufferFeatures,
            g_vkFormatFeatureFlagBitsMapperItems,
            g_vkFormatFeatureFlagBitsMapper
        );

        constexpr auto logcatAntispamDelay = std::chrono::milliseconds ( 1U );
        std::this_thread::sleep_for ( logcatAntispamDelay );
    };

    probe ( VK_FORMAT_A2R10G10B10_UNORM_PACK32, "VK_FORMAT_A2R10G10B10_UNORM_PACK32" );
    probe ( VK_FORMAT_ASTC_6x6_SRGB_BLOCK, "VK_FORMAT_ASTC_6x6_SRGB_BLOCK" );
    probe ( VK_FORMAT_ASTC_6x6_UNORM_BLOCK, "VK_FORMAT_ASTC_6x6_UNORM_BLOCK" );
    probe ( VK_FORMAT_D16_UNORM, "VK_FORMAT_D16_UNORM" );
    probe ( VK_FORMAT_D24_UNORM_S8_UINT, "VK_FORMAT_D24_UNORM_S8_UINT" );
    probe ( VK_FORMAT_D32_SFLOAT, "VK_FORMAT_D32_SFLOAT" );
    probe ( VK_FORMAT_R16G16B16A16_SFLOAT, "VK_FORMAT_R16G16B16A16_SFLOAT" );
    probe ( VK_FORMAT_R8G8B8A8_SRGB, "VK_FORMAT_R8G8B8A8_SRGB" );
    probe ( VK_FORMAT_R8G8B8A8_UNORM, "VK_FORMAT_R8G8B8A8_UNORM" );
    probe ( VK_FORMAT_R8G8_SRGB, "VK_FORMAT_R8G8_SRGB" );
    probe ( VK_FORMAT_R8_SRGB, "VK_FORMAT_R8_SRGB" );
    probe ( VK_FORMAT_R8_UNORM, "VK_FORMAT_R8_UNORM" );
    probe ( VK_FORMAT_S8_UINT, "VK_FORMAT_S8_UINT" );
    probe ( VK_FORMAT_X8_D24_UNORM_PACK32, "VK_FORMAT_X8_D24_UNORM_PACK32" );
    probe ( VK_FORMAT_R16_SFLOAT, "VK_FORMAT_R16_SFLOAT" );

    if ( unsupportedFormats.empty () )
        return true;

    for ( char const* format : unsupportedFormats )
        LogError ( "%sFAIL: %s", INDENT_1, format );

    return false;
}

#ifdef ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

bool Renderer::DeployDebugFeatures () noexcept
{
    vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT> (
        vkGetInstanceProcAddr ( _instance, "vkCreateDebugUtilsMessengerEXT" )
    );

    AV_ASSERT ( vkCreateDebugUtilsMessengerEXT )

    vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT> (
        vkGetInstanceProcAddr ( _instance, "vkDestroyDebugUtilsMessengerEXT" )
    );

    return CheckVkResult (
        vkCreateDebugUtilsMessengerEXT ( _instance, &_debugUtilsMessengerCreateInfo, nullptr, &_debugUtilsMessenger ),
        "DeployDebugFeatures",
        "Can't Vulkan debug callback"
    );
}

void Renderer::DestroyDebugFeatures () noexcept
{
    if ( _debugUtilsMessenger == VK_NULL_HANDLE )
        return;

    vkDestroyDebugUtilsMessengerEXT ( _instance, _debugUtilsMessenger, nullptr );
    _debugUtilsMessenger = VK_NULL_HANDLE;
}

#endif // ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

bool Renderer::DeployDevice () noexcept
{
    constexpr float priorities = 1.0F;

    VkDeviceQueueCreateInfo deviceQueueCreateInfo;
    deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo.pNext = nullptr;
    deviceQueueCreateInfo.flags = 0U;
    deviceQueueCreateInfo.queueCount = 1U;
    deviceQueueCreateInfo.pQueuePriorities = &priorities;

    if ( !SelectTargetHardware ( _physicalDevice, _queueFamilyIndex ) )
        return false;

    deviceQueueCreateInfo.queueFamilyIndex = _queueFamilyIndex;
    auto const &caps = _physicalDeviceInfo[ _physicalDevice ];

    if ( !CheckRequiredDeviceExtensions ( caps._extensions ) )
        return false;

    constexpr size_t const features[] =
    {
        offsetof ( VkPhysicalDeviceFeatures, textureCompressionASTC_LDR )
    };

    if ( !CheckRequiredFeatures ( _physicalDevice, features, std::size ( features ) ) )
        return false;

    if ( !CheckRequiredFormats () )
        return false;

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

    constexpr char const* extensions[] =
    {
        VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
        VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME,
        VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME,
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkDeviceCreateInfo const deviceCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &float16Int8Features,
        .flags = 0U,
        .queueCreateInfoCount = 1U,
        .pQueueCreateInfos = &deviceQueueCreateInfo,
        .enabledLayerCount = 0U,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = static_cast<uint32_t> ( std::size ( extensions ) ),
        .ppEnabledExtensionNames = extensions,
        .pEnabledFeatures = &caps._features
    };

    bool const result = CheckVkResult ( vkCreateDevice ( _physicalDevice, &deviceCreateInfo, nullptr, &_device ),
        "Renderer::DeployDevice",
        "Can't create device"
    );

    if ( !result )
        return false;

    AV_REGISTER_DEVICE ( "Renderer::_device" )

    if ( !_vulkanLoader.AcquireDeviceFunctions ( _device ) )
        return false;

    vkGetDeviceQueue ( _device, _queueFamilyIndex, 0U, &_queue );
    _memoryAllocator.Init ( _physicalDeviceMemoryProperties );
    return true;
}

void Renderer::DestroyDevice () noexcept
{
    _memoryAllocator.Destroy ( _device );

    if ( !_device )
    {
        AV_CHECK_VULKAN_LEAKS ()
        return;
    }

    // Note it's intentional to unregister device BEFORE destruction for correct memory leak check stuff.
    AV_UNREGISTER_DEVICE ( "Renderer::_device" )
    AV_CHECK_VULKAN_LEAKS ()

    vkDestroyDevice ( _device, nullptr );
    _device = VK_NULL_HANDLE;
    _queue = VK_NULL_HANDLE;
}

bool Renderer::DeployInstance () noexcept
{
    uint32_t supportedVersion = 0U;

    bool result = CheckVkResult ( vkEnumerateInstanceVersion ( &supportedVersion ),
        "Renderer::DeployInstance",
        "Can't query instance version"
    );

    if ( !result )
        return false;

    //                                                                            major      minor      patch
    constexpr uint32_t const targetVersion = TARGET_VULKAN_VERSION & UINT32_C ( 0b1111111111'1111111111'000000000000 );

    if ( targetVersion > supportedVersion )
    {
        LogError ( "Renderer::DeployInstance - Requested Vulkan version %u.%u.%u is not supported by hardware "
            "which is capable of only %u.%u.xxx.",
            MAJOR,
            MINOR,
            PATCH,
            ( supportedVersion & UINT32_C ( 0b1111111111'0000000000'000000000000 ) ) >> 22U,
            ( supportedVersion & UINT32_C ( 0b0000000000'1111111111'000000000000 ) ) >> 12U
        );

        return false;
    }

    constexpr VkApplicationInfo const applicationInfo
    {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = APPLICATION_NAME,
        .applicationVersion = 1U,
        .pEngineName = ENGINE_NAME,
        .engineVersion = 1U,
        .apiVersion = TARGET_VULKAN_VERSION
    };

    VkInstanceCreateInfo instanceCreateInfo {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.flags = 0U;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;

#ifdef ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

    // [2022/07/26] GPU assisted validation is impossible on MALI G76 (driver 26) due to lack of required
    // feature - VkPhysicalDeviceFeatures::vertexPipelineStoresAndAtomics.
    constexpr static VkValidationFeatureEnableEXT const validationFeatures[] =
    {
        VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
        VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT
    };

    constexpr VkValidationFeaturesEXT validationInfo
    {
        .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
        .pNext = nullptr,
        .enabledValidationFeatureCount = static_cast<uint32_t> ( std::size ( validationFeatures ) ),
        .pEnabledValidationFeatures = validationFeatures,
        .disabledValidationFeatureCount = 0U,
        .pDisabledValidationFeatures = nullptr
    };

    _debugUtilsMessengerCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = &validationInfo,
        .flags = 0U,

        .messageSeverity = AV_VK_FLAG ( VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT ) |
            AV_VK_FLAG ( VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ),

        .messageType = AV_VK_FLAG ( VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT ) |
            AV_VK_FLAG ( VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT ) |
            AV_VK_FLAG ( VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT ),

        .pfnUserCallback = &Renderer::OnVulkanDebugUtils,
        .pUserData = nullptr
    };

    constexpr static char const* layers[] =
    {
        "VK_LAYER_KHRONOS_validation"
    };

    constexpr static char const* extensions[] =
    {
        VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME
    };

    instanceCreateInfo.pNext = &_debugUtilsMessengerCreateInfo;
    instanceCreateInfo.enabledLayerCount = static_cast<uint32_t> ( std::size ( layers ) );
    instanceCreateInfo.ppEnabledLayerNames = layers;

#else

    constexpr static const char* extensions[] =
    {
        VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME
    };

    instanceCreateInfo.pNext = nullptr;
    instanceCreateInfo.enabledLayerCount = 0U;
    instanceCreateInfo.ppEnabledLayerNames = nullptr;

#endif

    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t const> ( std::size ( extensions ) );
    instanceCreateInfo.ppEnabledExtensionNames = extensions;

    result = CheckVkResult ( vkCreateInstance ( &instanceCreateInfo, nullptr, &_instance ),
        "Renderer::DeployInstance",
        "Can't create Vulkan instance"
    );

    if ( !result )
        return false;

    return _vulkanLoader.AcquireInstanceFunctions ( _instance );
}

void Renderer::DestroyInstance () noexcept
{
    if ( !_instance )
        return;

    vkDestroyInstance ( _instance, nullptr );
    _instance = VK_NULL_HANDLE;
}

bool Renderer::DeploySurface ( ANativeWindow &nativeWindow ) noexcept
{
    VkAndroidSurfaceCreateInfoKHR const androidSurfaceCreateInfoKHR
    {
        .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0U,
        .window = &nativeWindow
    };

    bool result = CheckVkResult (
        vkCreateAndroidSurfaceKHR ( _instance, &androidSurfaceCreateInfoKHR, nullptr, &_surface ),
        "Renderer::DeploySurface",
        "Can't create Vulkan surface"
    );

    if ( !result )
        return false;

    AV_REGISTER_SURFACE ( "Renderer::_surface" )

    VkSurfaceCapabilitiesKHR &surfaceCapabilitiesKHR = _physicalDeviceInfo[ _physicalDevice ]._surfaceCapabilities;

    result = CheckVkResult (
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR ( _physicalDevice, _surface, &surfaceCapabilitiesKHR ),
        "Renderer::DeploySurface",
        "Can't get Vulkan surface capabilities"
    );

    if ( !result )
    {
        DestroySurface ();
        return false;
    }

    PrintVkSurfaceCapabilities ( surfaceCapabilitiesKHR );
    _surfaceSize = surfaceCapabilitiesKHR.currentExtent;
    _surfaceTransform = surfaceCapabilitiesKHR.currentTransform;

    if ( _surfaceTransform != VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR )
    {
        LogError ( "Renderer::DeploySurface - Unexpected surface transform: %s",
            ResolveVkSurfaceTransform ( _surfaceTransform )
        );

        DestroySurface ();
        return false;
    }

#ifdef ANDROID_NATIVE_MODE_PORTRAIT

    _presentationEngineTransform.RotationZ ( GX_MATH_HALF_PI );
    _viewportResolution.width = _surfaceSize.height;
    _viewportResolution.height = _surfaceSize.width;

#elif defined ( ANDROID_NATIVE_MODE_LANDSCAPE )

    _presentationEngineTransform.Identity ();
    _viewportResolution = _surfaceSize;

#else

#error Please specify ANDROID_NATIVE_MODE_PORTRAIT or ANDROID_NATIVE_MODE_LANDSCAPE in the preprocessor macros.

#endif

    VkBool32 isSupported = VK_FALSE;

    result = CheckVkResult (
        vkGetPhysicalDeviceSurfaceSupportKHR ( _physicalDevice, _queueFamilyIndex, _surface, &isSupported ),
        "Renderer::DeploySurface",
        "Can't check Vulkan surface support by physical device"
    );

    if ( !result )
    {
        DestroySurface ();
        return false;
    }

    if ( !isSupported )
    {
        DestroySurface ();

        LogError ( "Renderer::DeploySurface - Physical device does not support by Vulkan surface." );
        return false;
    }

    uint32_t formatCount = 0U;
    vkGetPhysicalDeviceSurfaceFormatsKHR ( _physicalDevice, _surface, &formatCount, nullptr );

    if ( !formatCount )
    {
        DestroySurface ();

        LogError ( "Renderer::DeploySurface - There is not any Vulkan surface formats." );
        return false;
    }

    LogInfo ( "Renderer::DeploySurface - Vulkan surface formats detected: %u.", formatCount );

    _surfaceFormats.resize ( static_cast<size_t> ( formatCount ) );
    VkSurfaceFormatKHR* formatList = _surfaceFormats.data ();

    result = CheckVkResult (
        vkGetPhysicalDeviceSurfaceFormatsKHR ( _physicalDevice, _surface, &formatCount, formatList ),
        "Renderer::DeploySurface",
        "Can't get Vulkan surface formats"
    );

    if ( !result )
    {
        DestroySurface ();
        return false;
    }

    for ( uint32_t i = 0U; i < formatCount; ++i )
        PrintVkSurfaceFormatKHRProp ( i, formatList[ i ] );

    return true;
}

void Renderer::DestroySurface () noexcept
{
    vkDestroySurfaceKHR ( _instance, _surface, nullptr );
    _surface = VK_NULL_HANDLE;
    AV_UNREGISTER_SURFACE ( "Renderer::_surface" )
}

bool Renderer::DeploySwapchain ( bool vSync ) noexcept
{
    VkPresentModeKHR presentMode;

    if ( !SelectTargetPresentMode ( presentMode, vSync ) )
    {
        LogError ( "Renderer::DeploySwapchain - Can't select present mode." );
        return false;
    }

    VkCompositeAlphaFlagBitsKHR compositeAlpha;

    if ( !SelectTargetCompositeAlpha ( compositeAlpha ) )
    {
        LogError ( "Renderer::DeploySwapchain - Can't select composite alpha mode." );
        return false;
    }

    VkColorSpaceKHR colorSpace;

    if ( !SelectTargetSurfaceFormat ( _surfaceFormat, colorSpace, _depthImageFormat, _depthStencilImageFormat ) )
    {
        LogError ( "Renderer::DeploySwapchain - Can't select image format and color space." );
        return false;
    }

    VkSwapchainCreateInfoKHR const swapchainInfo
    {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0U,
        .surface = _surface,
        .minImageCount = 3U,
        .imageFormat = _surfaceFormat,
        .imageColorSpace = colorSpace,
        .imageExtent = _surfaceSize,
        .imageArrayLayers = 1U,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr,

        // There is no much to say but you have to consider image rotation after projection transform in your code.
        // That's a cost for memory bandwidth saving in the mobile device world.
        // See https://community.arm.com/developer/tools-software/graphics/b/blog/posts/appropriate-use-of-surface-rotation
        // See https://github.com/KhronosGroup/Vulkan-Samples/blob/master/samples/performance/surface_rotation/surface_rotation_tutorial.md
        .preTransform = _surfaceTransform,

        .compositeAlpha = compositeAlpha,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE
    };

    bool result = CheckVkResult ( vkCreateSwapchainKHR ( _device, &swapchainInfo, nullptr, &_swapchain ),
        "Renderer::DeploySwapchain",
        "Can't create swapchain"
    );

    if ( !result )
        return false;

    AV_REGISTER_SWAPCHAIN ( "Renderer::_swapchain" )

    uint32_t imageCount = 0U;
    vkGetSwapchainImagesKHR ( _device, _swapchain, &imageCount, nullptr );

    if ( !imageCount )
    {
        DestroySwapchain ();

        LogError ( "Renderer::DeploySwapchain - There is no any swapchain images." );
        return false;
    }

    LogInfo ( "Renderer::DeploySwapchain - Swapchain images detected: %u.", imageCount );

    _swapchainImages.resize ( static_cast<size_t> ( imageCount ) );

    result = CheckVkResult ( vkGetSwapchainImagesKHR ( _device, _swapchain, &imageCount, _swapchainImages.data () ),
        "Renderer::DeploySwapchain",
        "Can't create swapchain"
    );

    if ( !result )
        return false;

    _swapchainImageViews.clear ();
    _swapchainImageViews.reserve ( static_cast<size_t> ( imageCount ) );

    VkImageViewCreateInfo imageViewCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .image = VK_NULL_HANDLE,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = swapchainInfo.imageFormat,

        .components
        {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY
        },

        .subresourceRange
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0U,
            .levelCount = 1U,
            .baseArrayLayer = 0U,
            .layerCount = 1U
        }
    };

    for ( uint32_t i = 0U; i < imageCount; ++i )
    {
        imageViewCreateInfo.image = _swapchainImages[ i ];
        VkImageView imageView = VK_NULL_HANDLE;

        result = CheckVkResult ( vkCreateImageView ( _device, &imageViewCreateInfo, nullptr, &imageView ),
            "Renderer::DeploySwapchain",
            "Can't create swapchain image view"
        );

        if ( result )
        {
            _swapchainImageViews.push_back ( imageView );

            AV_REGISTER_IMAGE_VIEW (
                "Renderer::_swapchainImageViews[ " + std::to_string ( static_cast<int> ( i ) ) + "U ]"
            )

            continue;
        }

        DestroySwapchain ();
        return false;
    }

    return true;
}

void Renderer::DestroySwapchain () noexcept
{
    size_t const count = _swapchainImageViews.size ();

    for ( size_t i = 0U; i < count; ++i )
    {
        vkDestroyImageView ( _device, _swapchainImageViews[ i ], nullptr );

        AV_UNREGISTER_IMAGE_VIEW (
            "Renderer::_swapchainImageViews[ " + std::to_string ( static_cast<int> ( i ) ) + "U ]"
        )
    }

    vkDestroySwapchainKHR ( _device, _swapchain, nullptr );
    _swapchain = VK_NULL_HANDLE;
    AV_UNREGISTER_SWAPCHAIN ( "Renderer::_swapchain" )
}

bool Renderer::PrintPhysicalDeviceExtensionInfo ( VkPhysicalDevice physicalDevice ) noexcept
{
    uint32_t extensionCount = 0U;
    vkEnumerateDeviceExtensionProperties ( physicalDevice, nullptr, &extensionCount, nullptr );

    if ( !extensionCount )
    {
        LogError ( "Renderer::PrintPhysicalDeviceExtensionInfo - There is no any physical device extensions." );
        return false;
    }

    LogInfo ( ">>> Physical device extensions detected: %u.", extensionCount);

    std::vector<VkExtensionProperties> extensions ( static_cast<size_t> ( extensionCount ) );
    VkExtensionProperties* extensionList = extensions.data ();

    bool const result = CheckVkResult (
        vkEnumerateDeviceExtensionProperties ( physicalDevice,
            nullptr,
            &extensionCount,
            extensionList
        ),

        "Renderer::PrintPhysicalDeviceExtensionInfo",
        "Can't get physical device extensions"
    );

    if ( !result )
        return false;

    VulkanPhysicalDeviceInfo &capabilities = _physicalDeviceInfo[ physicalDevice ];
    std::vector<char const*> &targetExtensions = capabilities._extensions;
    std::vector<char> &targetExtensionStorage = capabilities._extensionStorage;

    targetExtensions.reserve ( static_cast<size_t> ( extensionCount ) );
    size_t offset = 0U;

    for ( uint32_t i = 0U; i < extensionCount; ++i )
    {
        VkExtensionProperties const &prop = extensionList[ i ];
        PrintVkExtensionProp ( i, "Physical device", prop );

        size_t const size = strlen ( prop.extensionName ) + 1U;
        size_t const neededSpace = offset + size;

        while ( targetExtensionStorage.size () < neededSpace )
            targetExtensionStorage.resize ( targetExtensionStorage.size () * 2U );

        std::memcpy ( targetExtensionStorage.data () + offset, prop.extensionName, size );
        targetExtensions.push_back ( reinterpret_cast<char const*> ( offset ) );

        offset = neededSpace;
    }

    // Adjusting extension pointers.
    offset = reinterpret_cast<size_t> ( targetExtensionStorage.data () );

    for ( auto &pointer : targetExtensions )
        pointer += offset;

    return true;
}

void Renderer::PrintPhysicalDeviceFeatureInfo ( VkPhysicalDevice physicalDevice ) noexcept
{
    LogInfo ( ">>> Features:" );

    auto &features = _physicalDeviceInfo[ physicalDevice ];
    vkGetPhysicalDeviceFeatures ( physicalDevice, &features._features );

    // Note std::set will sort strings too.
    std::set<std::string_view> supportedFeatures;
    std::set<std::string_view> unsupportedFeatures;

    for ( auto const &probe : g_vkFeatureMap )
    {
        auto const enable = *reinterpret_cast<VkBool32 const*> (
            reinterpret_cast<uint8_t const*> ( &features._features ) + probe.first
        );

        if ( enable )
        {
            supportedFeatures.insert ( probe.second );
            continue;
        }

        unsupportedFeatures.insert ( probe.second );
    }

    LogInfo ( "%sSupported:", INDENT_2 );

    for ( auto &item : supportedFeatures )
        LogInfo ( "%s%s", INDENT_3, item.data () );

    LogInfo ( "%sUnsupported:", INDENT_2 );

    for ( auto &item : unsupportedFeatures )
    {
        LogInfo ( "%s%s", INDENT_3, item.data () );
    }
}

void Renderer::PrintPhysicalDeviceLimits ( VkPhysicalDeviceLimits const &limits ) noexcept
{
    LogInfo ( ">>> Limits:" );

    PrintUINT32Prop ( INDENT_1, "maxImageDimension1D", limits.maxImageDimension1D );
    PrintUINT32Prop ( INDENT_1, "maxImageDimension2D", limits.maxImageDimension2D );
    PrintUINT32Prop ( INDENT_1, "maxImageDimension3D", limits.maxImageDimension3D );
    PrintUINT32Prop ( INDENT_1, "maxImageDimensionCube", limits.maxImageDimensionCube );
    PrintUINT32Prop ( INDENT_1, "maxImageArrayLayers", limits.maxImageArrayLayers );
    PrintUINT32Prop ( INDENT_1, "maxTexelBufferElements", limits.maxTexelBufferElements );

    _maxUniformBufferRange = static_cast<size_t> ( limits.maxUniformBufferRange );
    PrintUINT32Prop ( INDENT_1, "maxUniformBufferRange", limits.maxUniformBufferRange );

    PrintUINT32Prop ( INDENT_1, "maxStorageBufferRange", limits.maxStorageBufferRange );
    PrintUINT32Prop ( INDENT_1, "maxPushConstantsSize", limits.maxPushConstantsSize );
    PrintUINT32Prop ( INDENT_1, "maxMemoryAllocationCount", limits.maxMemoryAllocationCount );
    PrintUINT32Prop ( INDENT_1, "maxSamplerAllocationCount", limits.maxSamplerAllocationCount );
    PrintSizeProp ( INDENT_1, "bufferImageGranularity", static_cast<size_t> ( limits.bufferImageGranularity ) );
    PrintSizeProp ( INDENT_1, "sparseAddressSpaceSize", static_cast<size_t> ( limits.sparseAddressSpaceSize ) );
    PrintUINT32Prop ( INDENT_1, "maxBoundDescriptorSets", limits.maxBoundDescriptorSets );
    PrintUINT32Prop ( INDENT_1, "maxPerStageDescriptorSamplers", limits.maxPerStageDescriptorSamplers );
    PrintUINT32Prop ( INDENT_1, "maxPerStageDescriptorUniformBuffers", limits.maxPerStageDescriptorUniformBuffers );
    PrintUINT32Prop ( INDENT_1, "maxPerStageDescriptorStorageBuffers", limits.maxPerStageDescriptorStorageBuffers );
    PrintUINT32Prop ( INDENT_1, "maxPerStageDescriptorSampledImages", limits.maxPerStageDescriptorSampledImages );
    PrintUINT32Prop ( INDENT_1, "maxPerStageDescriptorStorageImages", limits.maxPerStageDescriptorStorageImages );
    PrintUINT32Prop ( INDENT_1, "maxPerStageDescriptorInputAttachments", limits.maxPerStageDescriptorInputAttachments );
    PrintUINT32Prop ( INDENT_1, "maxPerStageResources", limits.maxPerStageResources );
    PrintUINT32Prop ( INDENT_1, "maxDescriptorSetSamplers", limits.maxDescriptorSetSamplers );
    PrintUINT32Prop ( INDENT_1, "maxDescriptorSetUniformBuffers", limits.maxDescriptorSetUniformBuffers );
    PrintUINT32Prop ( INDENT_1, "maxDescriptorSetUniformBuffersDynamic", limits.maxDescriptorSetUniformBuffersDynamic );
    PrintUINT32Prop ( INDENT_1, "maxDescriptorSetStorageBuffers", limits.maxDescriptorSetStorageBuffers );
    PrintUINT32Prop ( INDENT_1, "maxDescriptorSetStorageBuffersDynamic", limits.maxDescriptorSetStorageBuffersDynamic );
    PrintUINT32Prop ( INDENT_1, "maxDescriptorSetSampledImages", limits.maxDescriptorSetSampledImages );
    PrintUINT32Prop ( INDENT_1, "maxDescriptorSetStorageImages", limits.maxDescriptorSetStorageImages );
    PrintUINT32Prop ( INDENT_1, "maxDescriptorSetInputAttachments", limits.maxDescriptorSetInputAttachments );
    PrintUINT32Prop ( INDENT_1, "maxVertexInputAttributes", limits.maxVertexInputAttributes );
    PrintUINT32Prop ( INDENT_1, "maxVertexInputBindings", limits.maxVertexInputBindings );
    PrintUINT32Prop ( INDENT_1, "maxVertexInputAttributeOffset", limits.maxVertexInputAttributeOffset );
    PrintUINT32Prop ( INDENT_1, "maxVertexInputBindingStride", limits.maxVertexInputBindingStride );
    PrintUINT32Prop ( INDENT_1, "maxVertexOutputComponents", limits.maxVertexOutputComponents );
    PrintUINT32Prop ( INDENT_1, "maxTessellationGenerationLevel", limits.maxTessellationGenerationLevel );
    PrintUINT32Prop ( INDENT_1, "maxTessellationPatchSize", limits.maxTessellationPatchSize );

    PrintUINT32Prop ( INDENT_1,
        "maxTessellationControlPerVertexInputComponents",
        limits.maxTessellationControlPerVertexInputComponents
    );

    PrintUINT32Prop ( INDENT_1,
        "maxTessellationControlPerVertexOutputComponents",
        limits.maxTessellationControlPerVertexOutputComponents
    );

    PrintUINT32Prop ( INDENT_1,
        "maxTessellationControlPerPatchOutputComponents",
        limits.maxTessellationControlPerPatchOutputComponents
    );

    PrintUINT32Prop ( INDENT_1,
        "maxTessellationControlTotalOutputComponents",
        limits.maxTessellationControlTotalOutputComponents
    );

    PrintUINT32Prop ( INDENT_1,
        "maxTessellationEvaluationInputComponents",
        limits.maxTessellationEvaluationInputComponents
    );

    PrintUINT32Prop ( INDENT_1,
        "maxTessellationEvaluationOutputComponents",
        limits.maxTessellationEvaluationOutputComponents
    );

    PrintUINT32Prop ( INDENT_1, "maxGeometryShaderInvocations", limits.maxGeometryShaderInvocations );
    PrintUINT32Prop ( INDENT_1, "maxGeometryInputComponents", limits.maxGeometryInputComponents );
    PrintUINT32Prop ( INDENT_1, "maxGeometryOutputComponents", limits.maxGeometryOutputComponents );
    PrintUINT32Prop ( INDENT_1, "maxGeometryOutputVertices", limits.maxGeometryOutputVertices );
    PrintUINT32Prop ( INDENT_1, "maxGeometryTotalOutputComponents", limits.maxGeometryTotalOutputComponents );
    PrintUINT32Prop ( INDENT_1, "maxFragmentInputComponents", limits.maxFragmentInputComponents );
    PrintUINT32Prop ( INDENT_1, "maxFragmentOutputAttachments", limits.maxFragmentOutputAttachments );
    PrintUINT32Prop ( INDENT_1, "maxFragmentDualSrcAttachments", limits.maxFragmentDualSrcAttachments );
    PrintUINT32Prop ( INDENT_1, "maxFragmentCombinedOutputResources", limits.maxFragmentCombinedOutputResources );
    PrintUINT32Prop ( INDENT_1, "maxComputeSharedMemorySize", limits.maxComputeSharedMemorySize );

    PrintUINT32Vec3Prop ( INDENT_1, "maxComputeWorkGroupCount", limits.maxComputeWorkGroupCount );
    PrintUINT32Prop ( INDENT_1, "maxComputeWorkGroupInvocations", limits.maxComputeWorkGroupInvocations );
    PrintUINT32Vec3Prop ( INDENT_1, "maxComputeWorkGroupSize", limits.maxComputeWorkGroupSize );
    PrintUINT32Prop ( INDENT_1, "subPixelPrecisionBits", limits.subPixelPrecisionBits );
    PrintUINT32Prop ( INDENT_1, "subTexelPrecisionBits", limits.subTexelPrecisionBits );
    PrintUINT32Prop ( INDENT_1, "mipmapPrecisionBits", limits.mipmapPrecisionBits );
    PrintUINT32Prop ( INDENT_1, "maxDrawIndexedIndexValue", limits.maxDrawIndexedIndexValue );
    PrintUINT32Prop ( INDENT_1, "maxDrawIndirectCount", limits.maxDrawIndirectCount );
    PrintFloatProp ( INDENT_1, "maxSamplerLodBias", limits.maxSamplerLodBias );
    PrintFloatProp ( INDENT_1, "maxSamplerAnisotropy", limits.maxSamplerAnisotropy );
    PrintUINT32Prop ( INDENT_1, "maxViewports", limits.maxViewports );
    PrintUINT32Vec2Prop ( INDENT_1, "maxViewportDimensions", limits.maxViewportDimensions );
    PrintFloatVec2Prop ( INDENT_1, "viewportBoundsRange", limits.viewportBoundsRange );
    PrintUINT32Prop ( INDENT_1, "viewportSubPixelBits", limits.viewportSubPixelBits );
    PrintSizeProp ( INDENT_1, "minMemoryMapAlignment", limits.minMemoryMapAlignment );

    PrintSizeProp ( INDENT_1,
        "minTexelBufferOffsetAlignment",
        static_cast<size_t> ( limits.minTexelBufferOffsetAlignment )
    );

    PrintSizeProp ( INDENT_1,
        "minUniformBufferOffsetAlignment",
        static_cast<size_t> ( limits.minUniformBufferOffsetAlignment )
    );

    PrintSizeProp ( INDENT_1,
        "minStorageBufferOffsetAlignment",
        static_cast<size_t> ( limits.minStorageBufferOffsetAlignment )
    );

    PrintINT32Prop ( INDENT_1, "minTexelOffset", limits.minTexelOffset );
    PrintUINT32Prop ( INDENT_1, "maxTexelOffset", limits.maxTexelOffset );
    PrintINT32Prop ( INDENT_1, "minTexelGatherOffset", limits.minTexelGatherOffset );
    PrintUINT32Prop ( INDENT_1, "maxTexelGatherOffset", limits.maxTexelGatherOffset );
    PrintFloatProp ( INDENT_1, "minInterpolationOffset", limits.minInterpolationOffset );
    PrintFloatProp ( INDENT_1, "maxInterpolationOffset", limits.maxInterpolationOffset );
    PrintUINT32Prop ( INDENT_1, "subPixelInterpolationOffsetBits", limits.subPixelInterpolationOffsetBits );
    PrintUINT32Prop ( INDENT_1, "maxFramebufferWidth", limits.maxFramebufferWidth );
    PrintUINT32Prop ( INDENT_1, "maxFramebufferHeight", limits.maxFramebufferHeight );
    PrintUINT32Prop ( INDENT_1, "maxFramebufferLayers", limits.maxFramebufferLayers );

    PrintVkFlagsProp ( INDENT_1,
        "framebufferColorSampleCounts",
        limits.framebufferColorSampleCounts,
        g_vkSampleCountFlagMapperItems,
        g_vkSampleCountFlagMapper
    );

    PrintVkFlagsProp ( INDENT_1,
        "framebufferDepthSampleCounts",
        limits.framebufferDepthSampleCounts,
        g_vkSampleCountFlagMapperItems,
        g_vkSampleCountFlagMapper
    );

    PrintVkFlagsProp ( INDENT_1,
        "framebufferStencilSampleCounts",
        limits.framebufferStencilSampleCounts,
        g_vkSampleCountFlagMapperItems,
        g_vkSampleCountFlagMapper
    );

    PrintVkFlagsProp ( INDENT_1,
        "framebufferNoAttachmentsSampleCounts",
        limits.framebufferNoAttachmentsSampleCounts,
        g_vkSampleCountFlagMapperItems,
        g_vkSampleCountFlagMapper
    );

    PrintUINT32Prop ( INDENT_1, "maxColorAttachments", limits.maxColorAttachments );

    PrintVkFlagsProp ( INDENT_1,
        "sampledImageColorSampleCounts",
        limits.sampledImageColorSampleCounts,
        g_vkSampleCountFlagMapperItems,
        g_vkSampleCountFlagMapper
    );

    PrintVkFlagsProp ( INDENT_1,
        "sampledImageIntegerSampleCounts",
        limits.sampledImageIntegerSampleCounts,
        g_vkSampleCountFlagMapperItems,
        g_vkSampleCountFlagMapper
    );

    PrintVkFlagsProp ( INDENT_1,
        "sampledImageDepthSampleCounts",
        limits.sampledImageDepthSampleCounts,
        g_vkSampleCountFlagMapperItems,
        g_vkSampleCountFlagMapper
    );

    PrintVkFlagsProp ( INDENT_1,
        "sampledImageStencilSampleCounts",
        limits.sampledImageStencilSampleCounts,
        g_vkSampleCountFlagMapperItems,
        g_vkSampleCountFlagMapper
    );

    PrintVkFlagsProp ( INDENT_1,
        "storageImageSampleCounts",
        limits.storageImageSampleCounts,
        g_vkSampleCountFlagMapperItems,
        g_vkSampleCountFlagMapper
    );

    PrintUINT32Prop ( INDENT_1, "maxSampleMaskWords", limits.maxSampleMaskWords );
    PrintVkBool32Prop ( INDENT_1, "timestampComputeAndGraphics", limits.timestampComputeAndGraphics );
    PrintFloatProp ( INDENT_1, "timestampPeriod", limits.timestampPeriod );
    PrintUINT32Prop ( INDENT_1, "maxClipDistances", limits.maxClipDistances );
    PrintUINT32Prop ( INDENT_1, "maxCullDistances", limits.maxCullDistances );
    PrintUINT32Prop ( INDENT_1, "maxCombinedClipAndCullDistances", limits.maxCombinedClipAndCullDistances );
    PrintUINT32Prop ( INDENT_1, "discreteQueuePriorities", limits.discreteQueuePriorities );
    PrintFloatVec2Prop ( INDENT_1, "pointSizeRange", limits.pointSizeRange );
    PrintFloatVec2Prop ( INDENT_1, "lineWidthRange", limits.lineWidthRange );
    PrintFloatProp ( INDENT_1, "pointSizeGranularity", limits.pointSizeGranularity );
    PrintFloatProp ( INDENT_1, "lineWidthGranularity", limits.lineWidthGranularity );
    PrintVkBool32Prop ( INDENT_1, "strictLines", limits.strictLines );
    PrintVkBool32Prop ( INDENT_1, "standardSampleLocations", limits.standardSampleLocations );

    PrintSizeProp ( INDENT_1,
        "optimalBufferCopyOffsetAlignment",
        static_cast<size_t> ( limits.optimalBufferCopyOffsetAlignment )
    );

    PrintSizeProp ( INDENT_1,
        "optimalBufferCopyRowPitchAlignment",
        static_cast<size_t> ( limits.optimalBufferCopyRowPitchAlignment )
    );

    PrintSizeProp ( INDENT_1, "nonCoherentAtomSize", static_cast<size_t> ( limits.nonCoherentAtomSize ) );
}

void Renderer::PrintPhysicalDeviceMemoryProperties ( VkPhysicalDevice physicalDevice ) noexcept
{
    vkGetPhysicalDeviceMemoryProperties ( physicalDevice, &_physicalDeviceMemoryProperties );

    LogInfo ( ">>> Memory properties:" );
    PrintUINT32Prop ( INDENT_1, "memoryTypeCount", _physicalDeviceMemoryProperties.memoryTypeCount );

    for ( uint32_t i = 0U; i < _physicalDeviceMemoryProperties.memoryTypeCount; ++i )
    {
        VkMemoryType const &type = _physicalDeviceMemoryProperties.memoryTypes[ i ];
        LogInfo ( "%smemoryType: #%u", INDENT_2, i );

        PrintVkFlagsProp ( INDENT_3,
            "memoryTypes",
            type.propertyFlags,
            g_vkMemoryPropertyFlagBitsMapperItems,
            g_vkMemoryPropertyFlagBitsMapper
        );

        PrintUINT32Prop ( INDENT_3, "heapIndex", type.heapIndex );
    }

    PrintUINT32Prop ( INDENT_1, "memoryHeapCount", _physicalDeviceMemoryProperties.memoryHeapCount );

    for ( uint32_t i = 0U; i < _physicalDeviceMemoryProperties.memoryHeapCount; ++i )
    {
        VkMemoryHeap const &heap = _physicalDeviceMemoryProperties.memoryHeaps[ i ];

        LogInfo ( "%smemoryHeap: #%u", INDENT_2, i );
        PrintSizeProp ( INDENT_3, "size", static_cast<size_t> ( heap.size ) );

        PrintVkFlagsProp ( INDENT_3,
            "memoryTypes",
            heap.flags,
            g_vkMemoryHeapFlagBitsMapperItems,
            g_vkMemoryHeapFlagBitsMapper
        );
    }
}

bool Renderer::PrintPhysicalDeviceInfo ( uint32_t deviceIndex, VkPhysicalDevice physicalDevice ) noexcept
{
    LogInfo ( "Renderer::PrintPhysicalDeviceInfo - Vulkan physical device #%u", deviceIndex );

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties ( physicalDevice, &props );

    PrintVkHandler ( INDENT_1, "Device handler", physicalDevice );

    PrintPhysicalDeviceCommonProps ( props );
    PrintPhysicalDeviceLimits ( props.limits );
    PrintPhysicalDeviceSparse ( props.sparseProperties );
    PrintPhysicalDeviceFeatureInfo ( physicalDevice );

    if ( !PrintPhysicalDeviceExtensionInfo ( physicalDevice ) )
        return false;

    if ( !PrintPhysicalDeviceLayerInfo ( physicalDevice ) )
        return false;

    PrintPhysicalDeviceMemoryProperties ( physicalDevice );

    uint32_t queueFamilyCount = 0U;
    vkGetPhysicalDeviceQueueFamilyProperties ( physicalDevice, &queueFamilyCount, nullptr );

    if ( !queueFamilyCount )
    {
        LogError ( "Renderer::PrintPhysicalDeviceInfo - There is no any Vulkan physical device queue families." );
        return false;
    }

    LogInfo ( ">>> Vulkan physical device queue families detected: %u.", queueFamilyCount );

    std::vector<VkQueueFamilyProperties> queueFamilyProps ( static_cast<size_t> ( queueFamilyCount ) );
    VkQueueFamilyProperties* queueFamilyPropList = queueFamilyProps.data ();
    vkGetPhysicalDeviceQueueFamilyProperties ( physicalDevice, &queueFamilyCount, queueFamilyPropList );

    auto &info = _physicalDeviceInfo[ physicalDevice ];
    auto &queueFamilies = info._queueFamilyInfo;
    queueFamilies.reserve ( static_cast<size_t> ( queueFamilyCount ) );

    for ( uint32_t i = 0U; i < queueFamilyCount; ++i )
    {
        VkQueueFamilyProperties const &familyProps = queueFamilyPropList[ i ];
        PrintPhysicalDeviceQueueFamilyInfo ( i, familyProps );
        queueFamilies.emplace_back ( std::make_pair ( familyProps.queueFlags, familyProps.queueCount ) );
    }

    return true;
}

bool Renderer::SelectTargetCompositeAlpha ( VkCompositeAlphaFlagBitsKHR &targetCompositeAlpha ) const noexcept
{
    // Priority mode: VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR.
    constexpr VkCompositeAlphaFlagBitsKHR const priorityMode = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    auto const findResult = _physicalDeviceInfo.find ( _physicalDevice );
    VkSurfaceCapabilitiesKHR const &surfaceCapabilitiesKHR = findResult->second._surfaceCapabilities;

    targetCompositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    if ( surfaceCapabilitiesKHR.supportedCompositeAlpha & priorityMode )
    {
        targetCompositeAlpha = priorityMode;
    }
    else
    {
        constexpr auto limit = static_cast<unsigned int> ( VK_COMPOSITE_ALPHA_FLAG_BITS_MAX_ENUM_KHR );
        auto const available = static_cast<unsigned int const> ( surfaceCapabilitiesKHR.supportedCompositeAlpha );
        unsigned int probe = 1U;

        while ( probe != limit )
        {
            if ( available & probe )
                break;

            probe <<= 1U;
        }

        targetCompositeAlpha = static_cast<VkCompositeAlphaFlagBitsKHR> ( probe );
    }

    LogInfo ( "Renderer::SelectTargetCompositeAlpha - Composite alpha selected: %s.",
        ResolveVkCompositeAlpha ( targetCompositeAlpha )
    );

    return targetCompositeAlpha != VK_COMPOSITE_ALPHA_FLAG_BITS_MAX_ENUM_KHR;
}

bool Renderer::SelectTargetHardware ( VkPhysicalDevice &targetPhysicalDevice,
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

bool Renderer::SelectTargetPresentMode ( VkPresentModeKHR &targetPresentMode, bool vSync ) const noexcept
{
    // Try to find VK_PRESENT_MODE_MAILBOX_KHR present mode.

    VkPresentModeKHR const desirableMode = vSync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_MAILBOX_KHR;

    uint32_t modeCount = 0U;

    bool const result = CheckVkResult (
        vkGetPhysicalDeviceSurfacePresentModesKHR ( _physicalDevice,
            _surface,
            &modeCount,
            nullptr
        ),

        "Renderer::SelectTargetPresentMode",
        "Can't get Vulkan present modes"
    );

    if ( !result )
        return false;

    LogInfo ( "Renderer::SelectTargetPresentMode - Present modes detected: %u.", modeCount );

    std::vector<VkPresentModeKHR> modes ( static_cast<size_t> ( modeCount ) );
    VkPresentModeKHR* modeList = modes.data ();
    vkGetPhysicalDeviceSurfacePresentModesKHR ( _physicalDevice, _surface, &modeCount, modeList );

    targetPresentMode = VK_PRESENT_MODE_FIFO_KHR;

    for ( uint32_t i = 0U; i < modeCount; ++i )
    {
        VkPresentModeKHR const mode = modeList[ i ];
        PrintVkPresentModeProp ( i, mode );

        if ( mode != desirableMode )
            continue;

        targetPresentMode = mode;
    }

    LogInfo ( "Renderer::SelectTargetPresentMode - Presented mode selected: %s.",
        ResolveVkPresentModeKHR ( targetPresentMode )
    );

    return true;
}

bool Renderer::SelectTargetSurfaceFormat ( VkFormat &targetColorFormat,
    VkColorSpaceKHR &targetColorSpace,
    VkFormat &targetDepthFormat,
    VkFormat &targetDepthStencilFormat
) const noexcept
{
    // Find sRGBA8 format.

    if ( _surfaceFormats.size () == 1U && _surfaceFormats[ 0U ].format == VK_FORMAT_UNDEFINED )
    {
        targetColorFormat = VK_FORMAT_R8G8B8A8_SRGB;
        targetColorSpace = _surfaceFormats[ 0U ].colorSpace;
    }
    else
    {
        bool isFound = false;

        for ( auto const &item : _surfaceFormats )
        {
            if ( item.format != VK_FORMAT_R8G8B8A8_SRGB )
                continue;

            targetColorFormat = VK_FORMAT_R8G8B8A8_SRGB;
            targetColorSpace = _surfaceFormats[ 0U ].colorSpace;
            isFound = true;

            break;
        }

        if ( !isFound )
        {
            return false;
        }
    }

    auto const check = [ & ] ( VkFormat &dst,
        VkFormat const* options,
        size_t count,
        char const* type
    ) noexcept -> bool {
        for ( size_t i = 0U; i < count; ++i )
        {
            VkFormat const format = options[ i ];

            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties ( _physicalDevice, format, &props );

            if ( !( props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT ) )
                continue;

            dst = format;
            return true;
        }

        LogError ( "Renderer::SelectTargetSurfaceFormat - Can't select % format.", type );
        return false;
    };

    constexpr VkFormat const depthStencilOptions[]
    {
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT
    };

    constexpr VkFormat const depthOptions[]
    {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_X8_D24_UNORM_PACK32,
        VK_FORMAT_D16_UNORM
    };

    if ( !check ( targetDepthFormat, depthOptions, std::size ( depthOptions ), "depth" ) )
        return false;

    if ( !check ( targetDepthStencilFormat, depthStencilOptions, std::size ( depthStencilOptions ), "depth|stencil" ) )
        return false;

    constexpr char const format[] = R"__(Renderer::SelectTargetSurfaceFormat - Surface format selected:
%sColor format: %s
%sColor space: %s
%sDepth format: %s
%sDepth|stencil format: %s
)__";

    LogInfo ( format,
        INDENT_1,
        ResolveVkFormat ( targetColorFormat ),
        INDENT_1,
        ResolveVkColorSpaceKHR ( targetColorSpace ),
        INDENT_1,
        ResolveVkFormat ( targetDepthFormat ),
        INDENT_1,
        ResolveVkFormat ( targetDepthStencilFormat )
    );

    return true;
}

bool Renderer::CheckExtensionCommon ( std::set<std::string> const &allExtensions,
    char const* extension
) noexcept
{
    LogInfo ( "%sChecking %s...", INDENT_1, extension );

    if ( allExtensions.count ( extension ) < 1U )
    {
        LogError ( "%sFAIL: unsupported", INDENT_2 );
        return false;
    }

    LogInfo ( "%sOK: presented", INDENT_2 );
    return true;
}

#ifdef ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

VkBool32 VKAPI_PTR Renderer::OnVulkanDebugUtils ( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
    void* /*pUserData*/
)
{
    if ( g_validationFilter.count ( static_cast<uint32_t> ( pCallbackData->messageIdNumber ) ) > 0U )
        return VK_FALSE;

    auto const encodeObjects = [] ( std::string &dst,
        uint32_t count,
        VkDebugUtilsObjectNameInfoEXT const* objects
    ) noexcept -> char const* {
        if ( !count )
            return "N/A";

        dst += objects->pObjectName ? objects->pObjectName : "[nullptr]";

        for ( uint32_t i = 1U; i < count; ++i )
        {
            VkDebugUtilsObjectNameInfoEXT const &object = objects[ i ];
            dst += ", ";
            dst += object.pObjectName ? object.pObjectName : "[nullptr]";
        }

        return dst.c_str ();
    };

    auto const encodeLabel = [] ( std::string &dst,
        uint32_t count,
        VkDebugUtilsLabelEXT const* labels
    ) noexcept -> char const* {
        if ( !count )
            return "N/A";

        dst += labels->pLabelName ? labels->pLabelName : "[nullptr]";

        for ( uint32_t i = 1U; i < count; ++i )
        {
            VkDebugUtilsLabelEXT const &label = labels[ i ];
            dst += ", ";
            dst += label.pLabelName ? label.pLabelName : "[nullptr]";
        }

        return dst.c_str ();
    };

    constexpr char const format[] =
R"(Renderer::OnVulkanDebugReport:
severity: %s
type: %s
message ID name: %s
message ID: 0x%08x
queues: %s
command buffers: %s
objects: %s
message: %s
)";

    std::string queues {};
    std::string commandBuffers {};
    std::string objects {};

    std::string const severity = StringifyVkFlags ( messageSeverity,
        g_vkDebugUtilsMessageSeverityFlagBitsEXTMapperItems,
        g_vkDebugUtilsMessageSeverityFlagBitsEXTMapper
    );

    std::string const type = StringifyVkFlags ( messageTypes,
        g_vkDebugUtilsMessageTypeFlagBitsEXTMapperItems,
        g_vkDebugUtilsMessageTypeFlagBitsEXTMapper
    );

    auto const prettyMessage = [] ( char const* message ) noexcept -> char const* {
        char const* cases[] = { message, "N/A" };
        return cases[ static_cast<size_t> ( message == nullptr ) ];
    };

    constexpr size_t removeFirstSpace = 1U;

    LogError ( format,
        severity.c_str () + removeFirstSpace,
        type.c_str () + removeFirstSpace,
        prettyMessage ( pCallbackData->pMessageIdName ),
        pCallbackData->messageIdNumber,
        encodeLabel ( queues, pCallbackData->queueLabelCount, pCallbackData->pQueueLabels ),
        encodeLabel ( commandBuffers, pCallbackData->cmdBufLabelCount, pCallbackData->pCmdBufLabels ),
        encodeObjects ( objects, pCallbackData->objectCount, pCallbackData->pObjects ),
        pCallbackData->pMessage
    );

#ifdef ANDROID_VULKAN_STRICT_MODE

    AV_ASSERT ( false )

#endif // ANDROID_VULKAN_STRICT_MODE

    return VK_FALSE;
}

#endif // ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

bool Renderer::PrintCoreExtensions () noexcept
{
    uint32_t extensionCount = 0U;
    vkEnumerateInstanceExtensionProperties ( nullptr, &extensionCount, nullptr );

    if ( !extensionCount )
    {
        LogError ( "Renderer::PrintCoreExtensions - There is no any core extensions!" );
        return false;
    }

    LogInfo ( "Renderer::PrintCoreExtensions - Instance core extensions detected: %u.", extensionCount );

    std::vector<VkExtensionProperties> extensions ( static_cast<size_t> ( extensionCount ) );
    VkExtensionProperties* extensionList = extensions.data ();

    bool const result = CheckVkResult (
        vkEnumerateInstanceExtensionProperties ( nullptr, &extensionCount, extensionList ),
        "Renderer::PrintCoreExtensions",
        "Can't get instance core extensions"
    );

    if ( !result )
        return false;

    for ( uint32_t i = 0U; i < extensionCount; ++i )
        PrintVkExtensionProp ( i, "Instance core", extensionList[ i ] );

    return true;
}

void Renderer::PrintFloatProp ( char const* indent, char const* name, float value ) noexcept
{
    LogInfo ( "%s%s: %g", indent, name, value );
}

void Renderer::PrintFloatVec2Prop ( char const* indent, char const* name, float const value[] ) noexcept
{
    LogInfo ( "%s%s: %g, %g", indent, name, value[ 0U ], value[ 1U ] );
}

void Renderer::PrintINT32Prop ( char const* indent, char const* name, int32_t value ) noexcept
{
    LogInfo ( "%s%s: %i", indent, name, value );
}

bool Renderer::PrintInstanceLayerInfo () noexcept
{
    uint32_t layerCount = 0U;
    vkEnumerateInstanceLayerProperties ( &layerCount, nullptr );

    if ( !layerCount )
    {
        LogInfo ( "Renderer::PrintInstanceLayerInfo - Instance does not contain any layers." );
        return PrintCoreExtensions ();
    }

    LogInfo ( ">>> Instance layers detected: %u.", layerCount );

    std::vector<VkLayerProperties> layers ( static_cast<size_t> ( layerCount ) );
    VkLayerProperties* layerList = layers.data ();

    bool const result = CheckVkResult ( vkEnumerateInstanceLayerProperties ( &layerCount, layerList ),
        "Renderer::PrintCoreExtensions",
        "Can't get instance core extensions"
    );

    if ( !result )
        return false;

    for ( uint32_t i = 0U; i < layerCount; ++i )
        PrintVkLayerProperties ( i, layerList[ i ] );

    return PrintCoreExtensions ();
}

void Renderer::PrintPhysicalDeviceCommonProps ( VkPhysicalDeviceProperties const &props ) noexcept
{
    LogInfo ( ">>> Common properties:" );

    PrintVkVersion ( INDENT_1, "apiVersion", props.apiVersion );
    PrintUINT32Prop ( INDENT_1, "driverVersion", props.driverVersion );
    PrintUINT32Prop ( INDENT_1, "vendorID", props.vendorID );
    PrintUINT32Prop ( INDENT_1, "deviceID", props.deviceID );
    PrintUTF8Prop ( INDENT_1, "deviceType", ResolvePhysicalDeviceType ( props.deviceType ) );
    PrintUTF8Prop ( INDENT_1, "deviceName", props.deviceName );

    LogInfo ( "%spipelineCacheUUID: {%x%x%x%x-%x%x-%x%x-%x%x-%x%x%x%x%x%x}",
        INDENT_1,
        props.pipelineCacheUUID[ 0U ],
        props.pipelineCacheUUID[ 1U ],
        props.pipelineCacheUUID[ 2U ],
        props.pipelineCacheUUID[ 3U ],
        props.pipelineCacheUUID[ 4U ],
        props.pipelineCacheUUID[ 5U ],
        props.pipelineCacheUUID[ 6U ],
        props.pipelineCacheUUID[ 7U ],
        props.pipelineCacheUUID[ 8U ],
        props.pipelineCacheUUID[ 9U ],
        props.pipelineCacheUUID[ 10U ],
        props.pipelineCacheUUID[ 11U ],
        props.pipelineCacheUUID[ 12U ],
        props.pipelineCacheUUID[ 13U ],
        props.pipelineCacheUUID[ 14U ],
        props.pipelineCacheUUID[ 15U ]
    );
}

void Renderer::PrintPhysicalDeviceGroupInfo ( uint32_t groupIndex,
    VkPhysicalDeviceGroupProperties const &props
) noexcept
{
    LogInfo ( "Renderer::PrintPhysicalDeviceGroupInfo - Vulkan physical device group #%u", groupIndex );

    PrintUINT32Prop ( INDENT_1, "physicalDeviceCount", props.physicalDeviceCount );

    for ( uint32_t i = 0U; i < props.physicalDeviceCount; ++i )
        PrintVkHandler ( INDENT_2, "Device handler", props.physicalDevices[ i ] );

    PrintVkBool32Prop ( INDENT_1, "subsetAllocation", props.subsetAllocation );
}

bool Renderer::PrintPhysicalDeviceLayerInfo ( VkPhysicalDevice physicalDevice ) noexcept
{
    uint32_t layerCount = 0U;
    vkEnumerateDeviceLayerProperties ( physicalDevice, &layerCount, nullptr );

    LogInfo ( ">>> Physical device layers detected: %u.", layerCount );

    if ( !layerCount )
        return true;

    std::vector<VkLayerProperties> layers ( static_cast<size_t> ( layerCount ) );
    VkLayerProperties* layerList = layers.data ();

    bool const result = CheckVkResult ( vkEnumerateDeviceLayerProperties ( physicalDevice, &layerCount, layerList ),
        "Renderer::PrintPhysicalDeviceLayerInfo",
        "Can't get physical device layers"
    );

    if ( !result )
        return false;

    for ( uint32_t i = 0U; i < layerCount; ++i )
        PrintVkLayerProperties ( i, layerList[ i ] );

    return true;
}

void Renderer::PrintPhysicalDeviceQueueFamilyInfo ( uint32_t queueFamilyIndex,
    VkQueueFamilyProperties const &props
) noexcept
{
    LogInfo ( "%sQueue family: #%u", INDENT_1, queueFamilyIndex );

    PrintVkFlagsProp ( INDENT_2, "queueFlags", props.queueFlags, g_vkQueueFlagMapperItems, g_vkQueueFlagMapper );
    PrintUINT32Prop ( INDENT_2, "queueCount", props.queueCount );
    PrintUINT32Prop ( INDENT_2, "timestampValidBits", props.timestampValidBits );
    PrintVkExtent3DProp ( INDENT_2, "minImageTransferGranularity", props.minImageTransferGranularity );
}

void Renderer::PrintUINT32Prop ( char const* indent, char const* name, uint32_t value ) noexcept
{
    LogInfo ( "%s%s: %u", indent, name, value );
}

void Renderer::PrintUINT32Vec2Prop ( char const* indent, char const* name, uint32_t const value[] ) noexcept
{
    LogInfo ( "%s%s: %u, %u", indent, name, value[ 0U ], value[ 1U ] );
}

void Renderer::PrintUINT32Vec3Prop ( char const* indent, char const* name, uint32_t const value[] ) noexcept
{
    LogInfo ( "%s%s: %u, %u, %u", indent, name, value[ 0U ], value[ 1U ], value[ 2U ] );
}

void Renderer::PrintUTF8Prop ( char const* indent, char const* name, char const* value ) noexcept
{
    LogInfo ( "%s%s: %s", indent, name, value );
}

void Renderer::PrintVkBool32Prop ( char const* indent, char const* name, VkBool32 value ) noexcept
{
    LogInfo ( "%s%s: %s", indent, name, value ? "VK_TRUE" : "VK_FALSE" );
}

void Renderer::PrintVkExtent2DProp ( char const* indent, char const* name, VkExtent2D const &value ) noexcept
{
    LogInfo ( "%s%s:", indent, name );
    LogInfo ( "%s%swidth: %u", indent, INDENT_1, value.width );
    LogInfo ( "%s%sheight: %u", indent, INDENT_1, value.height );
}

void Renderer::PrintVkExtent3DProp ( char const* indent, char const* name, VkExtent3D const &value ) noexcept
{
    LogInfo ( "%s%s:", indent, name );
    LogInfo ( "%s%swidth: %u", indent, INDENT_1, value.width );
    LogInfo ( "%s%sheight: %u", indent, INDENT_1, value.height );
    LogInfo ( "%s%sdepth: %u", indent, INDENT_1, value.depth );
}

void Renderer::PrintPhysicalDeviceSparse ( VkPhysicalDeviceSparseProperties const &sparse ) noexcept
{
    LogInfo ( ">>> Sparse:" );

    PrintVkBool32Prop ( INDENT_1, "residencyStandard2DBlockShape", sparse.residencyStandard2DBlockShape );

    PrintVkBool32Prop ( INDENT_1,
        "residencyStandard2DMultisampleBlockShape",
        sparse.residencyStandard2DMultisampleBlockShape
    );

    PrintVkBool32Prop ( INDENT_1, "residencyStandard3DBlockShape", sparse.residencyStandard3DBlockShape );
    PrintVkBool32Prop ( INDENT_1, "residencyAlignedMipSize", sparse.residencyAlignedMipSize );
    PrintVkBool32Prop ( INDENT_1, "residencyNonResidentStrict", sparse.residencyNonResidentStrict );
}

void Renderer::PrintSizeProp ( char const* indent, char const* name, size_t value ) noexcept
{
    LogInfo ( "%s%s: %zu", indent, name, value );
}

void Renderer::PrintVkExtensionProp ( uint32_t extensionIndex, char const* category,
    VkExtensionProperties const &extension
) noexcept
{
    LogInfo ( "%s%s extension: #%u", INDENT_1, category, extensionIndex );

    PrintUTF8Prop ( INDENT_2, "extensionName", extension.extensionName );
    PrintUINT32Prop ( INDENT_2, "specVersion", extension.specVersion );
}

void Renderer::PrintVkFlagsProp ( char const* indent,
    char const* name,
    VkFlags flags,
    size_t flagSetCount,
    std::pair<uint32_t, char const*> const flagSet[]
) noexcept
{
    if ( !flags )
    {
        LogInfo ( "%s%s: not set", indent, name );
        return;
    }

    LogInfo ( "%s%s:%s", indent, name, StringifyVkFlags ( flags, flagSetCount, flagSet ).c_str () );
}

void Renderer::PrintVkHandler ( char const* indent, char const* name, void* handler ) noexcept
{
    LogInfo ( "%s%s: %p", indent, name, handler );
}

void Renderer::PrintVkLayerProperties ( uint32_t layerIndex, VkLayerProperties const &layer ) noexcept
{
    LogInfo ( "%sLayer: #%u", INDENT_1, layerIndex );

    PrintUTF8Prop ( INDENT_2, "layerName", layer.layerName );
    PrintVkVersion ( INDENT_2, "specVersion", layer.specVersion );
    PrintUINT32Prop ( INDENT_2, "implementationVersion", layer.implementationVersion );
    PrintUTF8Prop ( INDENT_2, "description", layer.description );
}

void Renderer::PrintVkPresentModeProp ( uint32_t modeIndex, VkPresentModeKHR mode ) noexcept
{
    LogInfo ( "%sMode: #%u", INDENT_1, modeIndex );
    PrintUTF8Prop ( INDENT_2, "type", ResolveVkPresentModeKHR ( mode ) );
}

void Renderer::PrintVkSurfaceCapabilities ( VkSurfaceCapabilitiesKHR const &caps ) noexcept
{
    LogInfo ( ">>> Surface:" );

    PrintUINT32Prop ( INDENT_1, "minImageCount", caps.minImageCount );
    PrintUINT32Prop ( INDENT_1, "maxImageCount", caps.maxImageCount );
    PrintVkExtent2DProp ( INDENT_1, "currentExtent", caps.currentExtent );
    PrintVkExtent2DProp ( INDENT_1, "minImageExtent", caps.minImageExtent );
    PrintVkExtent2DProp ( INDENT_1, "maxImageExtent", caps.maxImageExtent );
    PrintUINT32Prop ( INDENT_1, "maxImageArrayLayers", caps.maxImageArrayLayers );

    PrintVkFlagsProp ( INDENT_1,
        "supportedTransforms",
        caps.supportedTransforms,
        g_vkSurfaceTransformFlagBitsKHRMapperItems,
        g_vkSurfaceTransformFlagBitsKHRMapper
    );

    LogInfo ( "%scurrentTransform: %s", INDENT_1, ResolveVkSurfaceTransform ( caps.currentTransform ) );

    PrintVkFlagsProp ( INDENT_1,
        "supportedCompositeAlpha",
        caps.supportedCompositeAlpha,
        g_vkCompositeAlphaFlagBitsKHRMapperItems,
        g_vkCompositeAlphaFlagBitsKHRMapper
    );

    PrintVkFlagsProp ( INDENT_1,
        "supportedUsageFlags",
        caps.supportedUsageFlags,
        g_vkImageUsageFlagBitsMapperItems,
        g_vkImageUsageFlagBitsMapper
    );
}

void Renderer::PrintVkSurfaceFormatKHRProp ( uint32_t formatIndex, VkSurfaceFormatKHR const &format ) noexcept
{
    LogInfo ( "%sSurface format: #%u", INDENT_1, formatIndex );

    PrintUTF8Prop ( INDENT_2, "format", ResolveVkFormat ( format.format ) );
    PrintUTF8Prop ( INDENT_2, "colorSpace ", ResolveVkColorSpaceKHR ( format.colorSpace ) );
}

void Renderer::PrintVkVersion ( char const* indent, char const* name, uint32_t version ) noexcept
{
    // Note vulkan_core.h is a little bit dirty from clang-tidy point of view.
    // So suppress this third-party mess via "NOLINT" control comment.
    uint32_t const major = VK_VERSION_MAJOR ( version ); // NOLINT
    uint32_t const minor = VK_VERSION_MINOR ( version ); // NOLINT
    uint32_t const patch = VK_VERSION_PATCH ( version ); // NOLINT

    LogInfo ( "%s%s: %u.%u.%u", indent, name, major, minor, patch );
}

char const* Renderer::ResolvePhysicalDeviceType ( VkPhysicalDeviceType type ) noexcept
{
    auto const findResult = _vulkanPhysicalDeviceTypeMap.find ( type );
    return findResult == _vulkanPhysicalDeviceTypeMap.cend () ? UNKNOWN_RESULT : findResult->second;
}

char const* Renderer::ResolveVkColorSpaceKHR ( VkColorSpaceKHR colorSpace ) noexcept
{
    auto const findResult = _vulkanColorSpaceMap.find ( colorSpace );
    return findResult == _vulkanColorSpaceMap.cend () ? UNKNOWN_RESULT : findResult->second;
}

char const* Renderer::ResolveVkCompositeAlpha ( VkCompositeAlphaFlagBitsKHR compositeAlpha ) noexcept
{
    auto const findResult = _vulkanCompositeAlphaMap.find ( compositeAlpha );
    return findResult == _vulkanCompositeAlphaMap.cend () ? UNKNOWN_RESULT : findResult->second;
}

char const* Renderer::ResolveVkPresentModeKHR ( VkPresentModeKHR mode ) noexcept
{
    auto const findResult = _vulkanPresentModeMap.find ( mode );
    return findResult == _vulkanPresentModeMap.cend () ? UNKNOWN_RESULT : findResult->second;
}

char const* Renderer::ResolveVkResult ( VkResult result ) noexcept
{
    auto const findResult = _vulkanResultMap.find ( result );

    if ( findResult != _vulkanResultMap.cend () )
        return findResult->second;

    constexpr static char const* unknownResult = "UNKNOWN";
    return unknownResult;
}

char const* Renderer::ResolveVkSurfaceTransform ( VkSurfaceTransformFlagsKHR transform ) noexcept
{
    auto const findResult = _vulkanSurfaceTransformMap.find ( transform );

    if ( findResult != _vulkanSurfaceTransformMap.cend () )
        return findResult->second;

    constexpr static char const* unknownResult = "UNKNOWN";
    return unknownResult;
}

std::string Renderer::StringifyVkFlags ( VkFlags flags,
    size_t flagSetCount,
    std::pair<uint32_t, char const*> const flagSet[]
) noexcept
{
    std::string result;
    auto const bitmask = static_cast<uint32_t> ( flags );

    for ( size_t i = 0U; i < flagSetCount; ++i )
    {
        auto const &item = flagSet[ i ];

        if ( !( item.first & bitmask ) )
            continue;

        result += " ";
        result += item.second;
    }

    return result;
}

} // namespace android_vulkan
