#ifndef RENDERER_H
#define RENDERER_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <map>
#include <set>
#include <vector>
#include <android/native_window.h>

GX_RESTORE_WARNING_STATE

#include <GXCommon/GXMath.h>
#include "logger.h"
#include "vulkan_api.h"
#include "vulkan_loader.h"


namespace android_vulkan {

struct VulkanPhysicalDeviceInfo final
{
    std::vector<char>                               _extensionStorage;

    std::vector<char const*>                        _extensions;
    VkPhysicalDeviceFeatures                        _features;

    std::vector<std::pair<VkFlags, uint32_t>>       _queueFamilyInfo;
    VkSurfaceCapabilitiesKHR                        _surfaceCapabilities;

    VulkanPhysicalDeviceInfo () noexcept;

    VulkanPhysicalDeviceInfo ( VulkanPhysicalDeviceInfo const & ) = delete;
    VulkanPhysicalDeviceInfo& operator = ( VulkanPhysicalDeviceInfo const & ) = delete;

    ~VulkanPhysicalDeviceInfo () = default;
};

//----------------------------------------------------------------------------------------------------------------------

class Renderer final
{
        using LogType = void (*) ( char const* format, ... );

    private:
        VkFormat                                                            _depthStencilImageFormat;
        VkDevice                                                            _device;
        VkInstance                                                          _instance;

        bool                                                                _isDeviceExtensionChecked;
        bool                                                                _isDeviceExtensionSupported;

        size_t                                                              _maxUniformBufferRange;

        VkPhysicalDevice                                                    _physicalDevice;

        VkQueue                                                             _queue;
        uint32_t                                                            _queueFamilyIndex;

        VkSurfaceKHR                                                        _surface;
        VkFormat                                                            _surfaceFormat;
        VkExtent2D                                                          _surfaceSize;
        VkSurfaceTransformFlagBitsKHR                                       _surfaceTransform;
        VkSwapchainKHR                                                      _swapchain;

#ifdef ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

        PFN_vkCreateDebugReportCallbackEXT                                  vkCreateDebugReportCallbackEXT;
        PFN_vkDestroyDebugReportCallbackEXT                                 vkDestroyDebugReportCallbackEXT;

        VkDebugReportCallbackEXT                                            _debugReportCallback;
        VkDebugReportCallbackCreateInfoEXT                                  _debugReportCallbackCreateInfoEXT;

        std::map<VkDebugReportFlagsEXT, std::pair<LogType, char const*>>    _loggerMapper;
        static std::map<VkDebugReportObjectTypeEXT, char const*> const      _vulkanObjectTypeMap;

#endif // ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

        VkExtent2D                                                          _viewportResolution;

        std::vector<VkPhysicalDeviceGroupProperties>                        _physicalDeviceGroups;
        std::map<VkPhysicalDevice, VulkanPhysicalDeviceInfo>                _physicalDeviceInfo;
        VkPhysicalDeviceMemoryProperties                                    _physicalDeviceMemoryProperties;

        std::vector<VkSurfaceFormatKHR>                                     _surfaceFormats;

        std::vector<VkImage>                                                _swapchainImages;
        std::vector<VkImageView>                                            _swapchainImageViews;

        GXMat4                                                              _presentationEngineTransform;
        VulkanLoader                                                        _vulkanLoader;

        static std::map<VkColorSpaceKHR, char const*> const                 _vulkanColorSpaceMap;
        static std::map<VkCompositeAlphaFlagBitsKHR, char const*> const     _vulkanCompositeAlphaMap;
        static std::map<VkFormat, char const*> const                        _vulkanFormatMap;
        static std::map<VkPhysicalDeviceType, char const*> const            _vulkanPhysicalDeviceTypeMap;
        static std::map<VkPresentModeKHR, char const*> const                _vulkanPresentModeMap;
        static std::map<VkResult, char const*> const                        _vulkanResultMap;
        static std::map<VkSurfaceTransformFlagsKHR, char const*> const      _vulkanSurfaceTransformMap;

    public:
        Renderer () noexcept;

        Renderer ( Renderer const & ) = delete;
        Renderer& operator = ( Renderer const & ) = delete;

        Renderer ( Renderer && ) = delete;
        Renderer& operator = ( Renderer && ) = delete;

        ~Renderer () = default;

        // Method returns true if swapchain does not change. User code can safely render frames.
        // Otherwise method returns false.
        [[nodiscard]] bool CheckSwapchainStatus ();

        [[nodiscard]] bool CreateShader ( VkShaderModule &shader,
            std::string &&shaderFile,
            char const* errorMessage
        ) const;

        [[nodiscard]] bool FinishAllJobs ();

        [[nodiscard]] VkFormat GetDefaultDepthStencilFormat () const;
        [[nodiscard]] VkDevice GetDevice () const;
        [[nodiscard]] size_t GetMaxUniformBufferRange () const;

        [[nodiscard]] size_t GetPresentImageCount () const;
        [[nodiscard]] VkImageView const& GetPresentImageView ( size_t imageIndex ) const;

        // Note this transform MUST be applied after projection transform to compensate screen orientation on the
        // mobile device. For more information please reference by links:
        // https://community.arm.com/developer/tools-software/graphics/b/blog/posts/appropriate-use-of-surface-rotation
        // https://github.com/KhronosGroup/Vulkan-Samples/blob/master/samples/performance/surface_rotation/surface_rotation_tutorial.md
        [[nodiscard]] GXMat4 const& GetPresentationEngineTransform () const;

        [[nodiscard]] VkQueue GetQueue () const;
        [[nodiscard]] uint32_t GetQueueFamilyIndex () const;
        [[nodiscard]] VkFormat GetSurfaceFormat () const;
        [[nodiscard]] VkExtent2D const& GetSurfaceSize () const;
        [[nodiscard]] VkSwapchainKHR& GetSwapchain ();

        // This resolution must be used by projection matrices. Resolution takes into consideration
        // current device orientation. The actual presentation image resolution can be acquired
        // by Renderer::GetSurfaceSize API.
        [[nodiscard]] VkExtent2D const& GetViewportResolution () const;

        [[nodiscard]] bool IsDeviceCreated () const;
        [[nodiscard]] bool IsSwapchainCreated () const;

        [[nodiscard]] bool OnCreateSwapchain ( ANativeWindow &nativeWindow, bool vSync );
        void OnDestroySwapchain ();

        [[nodiscard]] bool OnCreateDevice ();
        void OnDestroyDevice ();

        // Note method will invoke AV_REGISTER_DEVICE_MEMORY internally if success.
        [[nodiscard]] bool TryAllocateMemory ( VkDeviceMemory &memory,
            size_t size,
            VkMemoryPropertyFlags memoryProperties,
            char const* errorMessage
        ) const;

        // Note method will invoke AV_REGISTER_DEVICE_MEMORY internally if success.
        [[nodiscard]] bool TryAllocateMemory ( VkDeviceMemory &memory,
            VkMemoryRequirements const &requirements,
            VkMemoryPropertyFlags memoryProperties,
            char const* errorMessage
        ) const;

        // Method returns true is "result" equals VK_SUCCESS. Otherwise method returns false.
        [[nodiscard]] static bool CheckVkResult ( VkResult result, char const* from, char const* message );

        [[nodiscard]] static VkImageAspectFlags ResolveImageViewAspect ( VkFormat format );
        [[nodiscard]] static char const* ResolveVkFormat ( VkFormat format );

    private:
        [[nodiscard]] bool CheckExtensionMultiview ( std::set<std::string> const &allExtensions ) noexcept;
        [[nodiscard]] bool CheckExtensionShaderFloat16Int8 ( std::set<std::string> const &allExtensions ) noexcept;
        [[nodiscard]] bool CheckRequiredDeviceExtensions ( std::vector<char const*> const &deviceExtensions ) noexcept;

        // "features" is an array of offsets inside VkPhysicalDeviceFeatures structure.
        [[nodiscard]] bool CheckRequiredFeatures ( VkPhysicalDevice physicalDevice,
            size_t const* features,
            size_t count
        );

        [[nodiscard]] bool CheckRequiredFormats ();

#ifdef ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

        [[nodiscard]] bool DeployDebugFeatures ();
        void DestroyDebugFeatures ();

#endif // ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

        [[nodiscard]] bool DeployDevice ();
        void DestroyDevice ();

        [[nodiscard]] bool DeployInstance ();
        void DestroyInstance ();

        [[nodiscard]] bool DeploySurface ( ANativeWindow &nativeWindow );
        void DestroySurface ();

        [[nodiscard]] bool DeploySwapchain ( bool vSync );
        void DestroySwapchain ();

        [[nodiscard]] bool PrintPhysicalDeviceExtensionInfo ( VkPhysicalDevice physicalDevice );
        [[nodiscard]] bool PrintPhysicalDeviceFeatureInfo ( VkPhysicalDevice physicalDevice );

        void PrintPhysicalDeviceLimits ( VkPhysicalDeviceLimits const &limits );
        void PrintPhysicalDeviceMemoryProperties ( VkPhysicalDevice physicalDevice );
        [[nodiscard]] bool PrintPhysicalDeviceInfo ( uint32_t deviceIndex, VkPhysicalDevice physicalDevice );

        [[nodiscard]] bool SelectTargetCompositeAlpha ( VkCompositeAlphaFlagBitsKHR &targetCompositeAlpha ) const;

        [[nodiscard]] bool SelectTargetHardware ( VkPhysicalDevice &targetPhysicalDevice,
            uint32_t &targetQueueFamilyIndex
        ) const;

        [[nodiscard]] bool SelectTargetMemoryTypeIndex ( uint32_t &targetMemoryTypeIndex,
            VkMemoryPropertyFlags memoryProperties
        ) const;

        [[nodiscard]] bool SelectTargetMemoryTypeIndex ( uint32_t &targetMemoryTypeIndex,
            VkMemoryRequirements const &memoryRequirements,
            VkMemoryPropertyFlags memoryProperties
        ) const;

        [[nodiscard]] bool SelectTargetPresentMode ( VkPresentModeKHR &targetPresentMode, bool vSync ) const;

        [[nodiscard]] bool SelectTargetSurfaceFormat ( VkFormat &targetColorFormat,
            VkColorSpaceKHR &targetColorSpace,
            VkFormat &targetDepthStencilFormat
        ) const;

        [[nodiscard]] static bool CheckExtensionCommon ( std::set<std::string> const &allExtensions,
            char const* extension
        ) noexcept;

#ifdef ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

        [[nodiscard]] static VkBool32 VKAPI_PTR OnVulkanDebugReport ( VkDebugReportFlagsEXT flags,
            VkDebugReportObjectTypeEXT objectType,
            uint64_t object,
            size_t location,
            int32_t messageCode,
            const char* pLayerPrefix,
            const char* pMessage,
            void* pUserData
        );

#endif // ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

        [[nodiscard]] static bool PrintCoreExtensions ();
        static void PrintFloatProp ( char const* indent, char const* name, float value );
        static void PrintFloatVec2Prop ( char const* indent, char const* name, const float value[] );
        static void PrintINT32Prop ( char const* indent, char const* name, int32_t value );
        [[nodiscard]] static bool PrintInstanceLayerInfo ();
        static void PrintPhysicalDeviceCommonProps ( VkPhysicalDeviceProperties const &props );
        static void PrintPhysicalDeviceGroupInfo ( uint32_t groupIndex, VkPhysicalDeviceGroupProperties const &props );
        [[nodiscard]] static bool PrintPhysicalDeviceLayerInfo ( VkPhysicalDevice physicalDevice );

        static void PrintPhysicalDeviceQueueFamilyInfo ( uint32_t queueFamilyIndex,
            VkQueueFamilyProperties const &props
        );

        static void PrintPhysicalDeviceSparse ( VkPhysicalDeviceSparseProperties const &sparse );
        static void PrintSizeProp ( char const* indent, char const* name, size_t value );
        static void PrintUINT32Prop ( char const* indent, char const* name, uint32_t value );
        static void PrintUINT32Vec2Prop ( char const* indent, char const* name, const uint32_t value[] );
        static void PrintUINT32Vec3Prop ( char const* indent, char const* name, const uint32_t value[] );
        static void PrintUTF8Prop ( char const* indent, char const* name, char const* value );
        static void PrintVkBool32Prop ( char const* indent, char const* name, VkBool32 value );
        static void PrintVkExtent2DProp ( char const* indent, char const* name, const VkExtent2D &value );
        static void PrintVkExtent3DProp ( char const* indent, char const* name, const VkExtent3D &value );

        static void PrintVkExtensionProp ( uint32_t extensionIndex,
            char const* category,
            VkExtensionProperties const &extension
        );

        static void PrintVkFlagsProp ( char const* indent,
            char const* name,
            VkFlags flags,
            size_t flagSetCount,
            std::pair<uint32_t, char const*> const flagSet[]
        );

        static void PrintVkHandler ( char const* indent, char const* name, void* handler );
        static void PrintVkLayerProperties ( uint32_t layerIndex, VkLayerProperties const &layer );
        static void PrintVkPresentModeProp ( uint32_t modeIndex, VkPresentModeKHR mode );
        static void PrintVkSurfaceCapabilities ( VkSurfaceCapabilitiesKHR const &caps );
        static void PrintVkSurfaceFormatKHRProp ( uint32_t formatIndex, VkSurfaceFormatKHR const &format );
        static void PrintVkVersion ( char const* indent, char const* name, uint32_t version );

        [[nodiscard]] static char const* ResolvePhysicalDeviceType ( VkPhysicalDeviceType type );

#ifdef ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

        [[nodiscard]] static char const* ResolveVkDebugReportObjectType ( VkDebugReportObjectTypeEXT type );

#endif // ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

        [[nodiscard]] static char const* ResolveVkColorSpaceKHR ( VkColorSpaceKHR colorSpace );
        [[nodiscard]] static char const* ResolveVkCompositeAlpha ( VkCompositeAlphaFlagBitsKHR compositeAlpha );
        [[nodiscard]] static char const* ResolveVkPresentModeKHR ( VkPresentModeKHR mode );
        [[nodiscard]] static char const* ResolveVkResult ( VkResult result );
        [[nodiscard]] static char const* ResolveVkSurfaceTransform ( VkSurfaceTransformFlagsKHR transform );
};

} // namespace android_vulkan


#endif // RENDERER_H
