#ifndef RENDERER_H
#define RENDERER_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <map>
#include <vector>
#include <vulkan_wrapper.h>
#include <android/native_window.h>

GX_RESTORE_WARNING_STATE

#include <GXCommon/GXMath.h>
#include "logger.h"


namespace android_vulkan {

struct VulkanPhysicalDeviceInfo final
{
    std::vector<char>                               _extensionStorage;

    std::vector<char const*>                        _extensions;
    VkPhysicalDeviceFeatures                        _features;

    std::vector<std::pair<VkFlags, uint32_t>>       _queueFamilyInfo;
    VkSurfaceCapabilitiesKHR                        _surfaceCapabilities;

    VulkanPhysicalDeviceInfo ();
    ~VulkanPhysicalDeviceInfo () = default;

    VulkanPhysicalDeviceInfo ( const VulkanPhysicalDeviceInfo &other ) = delete;
    VulkanPhysicalDeviceInfo& operator = ( const VulkanPhysicalDeviceInfo &other ) = delete;
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

#endif

        VkExtent2D                                                          _viewportResolution;

        std::vector<VkPhysicalDeviceGroupProperties>                        _physicalDeviceGroups;
        std::map<VkPhysicalDevice, VulkanPhysicalDeviceInfo>                _physicalDeviceInfo;
        VkPhysicalDeviceMemoryProperties                                    _physicalDeviceMemoryProperties;

        std::vector<VkSurfaceFormatKHR>                                     _surfaceFormats;

        std::vector<VkImage>                                                _swapchainImages;
        std::vector<VkImageView>                                            _swapchainImageViews;

        GXMat4                                                              _presentationEngineTransform;

        static const std::map<VkColorSpaceKHR, char const*>                 _vulkanColorSpaceMap;
        static const std::map<VkCompositeAlphaFlagBitsKHR, char const*>     _vulkanCompositeAlphaMap;
        static const std::map<VkFormat, char const*>                        _vulkanFormatMap;
        static const std::map<VkPhysicalDeviceType, char const*>            _vulkanPhysicalDeviceTypeMap;
        static const std::map<VkPresentModeKHR, char const*>                _vulkanPresentModeMap;
        static const std::map<VkResult, char const*>                        _vulkanResultMap;
        static const std::map<VkSurfaceTransformFlagsKHR, char const*>      _vulkanSurfaceTransformMap;

    public:
        Renderer ();

        Renderer ( Renderer const &other ) = delete;
        Renderer& operator = ( Renderer const &other ) = delete;

        Renderer ( Renderer &&other ) = delete;
        Renderer& operator = ( Renderer &&other ) = delete;

        ~Renderer () = default;

        // Method returns true if swapchain does not change. User code can safely render frames.
        // Otherwise method returns false.
        bool CheckSwapchainStatus ();

        // Method returns true is "result" equals VK_SUCCESS. Otherwise method returns false.
        bool CheckVkResult ( VkResult result, char const* from, char const* message ) const;

        bool CreateShader ( VkShaderModule &shader,
            std::string &&shaderFile,
            char const* errorMessage
        ) const;

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

        [[nodiscard]] bool IsReady () const;

        [[nodiscard]] bool OnInit ( ANativeWindow &nativeWindow, bool vSync );
        void OnDestroy ();

        [[nodiscard]] char const* ResolveVkFormat ( VkFormat format ) const;

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

    private:
        [[nodiscard]] bool CheckRequiredDeviceExtensions ( const std::vector<char const*> &deviceExtensions,
            char const* const* requiredExtensions,
            size_t requiredExtensionCount
        );

        [[nodiscard]] bool CheckRequiredFormats ();

#ifdef ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

        [[nodiscard]] bool DeployDebugFeatures ();
        void DestroyDebugFeatures ();

#endif

        [[nodiscard]] bool DeployDevice ();
        void DestroyDevice ();

        [[nodiscard]] bool DeployInstance ();
        void DestroyInstance ();

        [[nodiscard]] bool DeploySurface ( ANativeWindow &nativeWindow );
        void DestroySurface ();

        [[nodiscard]] bool DeploySwapchain ( bool vSync );
        void DestroySwapchain ();

        [[nodiscard]] bool PrintCoreExtensions () const;
        void PrintFloatProp ( char const* indent, char const* name, float value ) const;
        void PrintFloatVec2Prop ( char const* indent, char const* name, const float value[] ) const;
        void PrintINT32Prop ( char const* indent, char const* name, int32_t value ) const;

        [[nodiscard]] bool PrintInstanceLayerInfo () const;

        void PrintPhysicalDeviceCommonProps ( VkPhysicalDeviceProperties const &props ) const;
        [[nodiscard]] bool PrintPhysicalDeviceExtensionInfo ( VkPhysicalDevice physicalDevice );
        [[nodiscard]] bool PrintPhysicalDeviceFeatureInfo ( VkPhysicalDevice physicalDevice );
        void PrintPhysicalDeviceGroupInfo ( uint32_t groupIndex, VkPhysicalDeviceGroupProperties const &props ) const;
        [[nodiscard]] bool PrintPhysicalDeviceLayerInfo ( VkPhysicalDevice physicalDevice ) const;
        void PrintPhysicalDeviceLimits ( VkPhysicalDeviceLimits const &limits );
        void PrintPhysicalDeviceMemoryProperties ( VkPhysicalDevice physicalDevice );
        [[nodiscard]] bool PrintPhysicalDeviceInfo ( uint32_t deviceIndex, VkPhysicalDevice physicalDevice );

        void PrintPhysicalDeviceQueueFamilyInfo ( uint32_t queueFamilyIndex,
            const VkQueueFamilyProperties &props
        ) const;

        void PrintPhysicalDeviceSparse ( VkPhysicalDeviceSparseProperties const &sparse ) const;
        void PrintVkBool32Prop ( char const* indent, char const* name, VkBool32 value ) const;

        void PrintVkExtent2DProp ( char const* indent, char const* name, const VkExtent2D &value ) const;
        void PrintVkExtent3DProp ( char const* indent, char const* name, const VkExtent3D &value ) const;

        void PrintVkExtensionProp ( uint32_t extensionIndex,
            char const* category,
            VkExtensionProperties const &extension
        ) const;

        void PrintVkFlagsProp ( char const* indent,
            char const* name,
            VkFlags flags,
            size_t flagSetCount,
            std::pair<uint32_t, char const*> const flagSet[]
        ) const;

        void PrintVkHandler ( char const* indent, char const* name, void* handler ) const;

        void PrintVkLayerProperties ( uint32_t layerIndex, const VkLayerProperties &layer ) const;
        void PrintVkPresentModeProp ( uint32_t modeIndex, VkPresentModeKHR mode ) const;
        void PrintVkSurfaceCapabilities ( VkSurfaceCapabilitiesKHR const &caps );
        void PrintVkSurfaceFormatKHRProp ( uint32_t formatIndex, VkSurfaceFormatKHR const &format ) const;
        void PrintVkVersion ( char const* indent, char const* name, uint32_t version ) const;
        void PrintSizeProp ( char const* indent, char const* name, size_t value ) const;
        void PrintUINT32Prop ( char const* indent, char const* name, uint32_t value ) const;
        void PrintUINT32Vec2Prop ( char const* indent, char const* name, const uint32_t value[] ) const;
        void PrintUINT32Vec3Prop ( char const* indent, char const* name, const uint32_t value[] ) const;
        void PrintUTF8Prop ( char const* indent, char const* name, char const* value ) const;

        [[nodiscard]] char const* ResolvePhysicalDeviceType ( VkPhysicalDeviceType type ) const;
        [[nodiscard]] char const* ResolveVkDebugReportObjectType ( VkDebugReportObjectTypeEXT type ) const;
        [[nodiscard]] char const* ResolveVkColorSpaceKHR ( VkColorSpaceKHR colorSpace ) const;
        [[nodiscard]] char const* ResolveVkCompositeAlpha ( VkCompositeAlphaFlagBitsKHR compositeAlpha ) const;
        [[nodiscard]] char const* ResolveVkPresentModeKHR ( VkPresentModeKHR mode ) const;
        [[nodiscard]] char const* ResolveVkResult ( VkResult result ) const;
        [[nodiscard]] char const* ResolveVkSurfaceTransform ( VkSurfaceTransformFlagsKHR transform ) const;

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

#endif

};

} // namespace android_vulkan


#endif // RENDERER_H
