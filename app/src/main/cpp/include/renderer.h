#ifndef RENDERER_H
#define RENDERER_H


#include "warning.h"

AV_DISABLE_COMMON_WARNINGS

#include <map>
#include <vector>
#include <vulkan_wrapper.h>
#include <android/native_window.h>

AV_RESTORE_WARNING_STATE

#include "logger.h"


namespace android_vulkan {

struct VulkanPhysicalDeviceInfo final
{
    std::vector<char>                               _extensionStorage;

    std::vector<const char*>                        _extensions;
    VkPhysicalDeviceFeatures                        _features;

    std::vector<std::pair<VkFlags, uint32_t>>       _queueFamilyInfo;

    VulkanPhysicalDeviceInfo ();
    ~VulkanPhysicalDeviceInfo () = default;

    VulkanPhysicalDeviceInfo ( const VulkanPhysicalDeviceInfo &other ) = delete;
    VulkanPhysicalDeviceInfo& operator = ( const VulkanPhysicalDeviceInfo &other ) = delete;
};

//----------------------------------------------------------------------------------------------------------------------

class Renderer final
{
    using LogType = void (*) ( const char* format, ... );

    private:
        // Note VkImage is a VK_DEFINE_NON_DISPATCHABLE_HANDLE type.
        VkDebugReportCallbackEXT                                            _debugReportCallback;

        // Note VkImage is a VK_DEFINE_NON_DISPATCHABLE_HANDLE type.
        VkImage                                                             _depthStencilImage;

        VkFormat                                                            _depthStencilImageFormat;

        // Note VkDeviceMemory and VkImageView are a VK_DEFINE_NON_DISPATCHABLE_HANDLE types.
        VkDeviceMemory                                                      _depthStencilImageMemory;
        VkImageView                                                         _depthStencilImageView;

        VkDevice                                                            _device;
        VkInstance                                                          _instance;
        VkPhysicalDevice                                                    _physicalDevice;

        VkQueue                                                             _queue;
        uint32_t                                                            _queueFamilyIndex;

        // Note VkRenderPass is a VK_DEFINE_NON_DISPATCHABLE_HANDLE.
        VkRenderPass                                                        _renderPass;

        // Note VkSurfaceKHR is a VK_DEFINE_NON_DISPATCHABLE_HANDLE.
        VkSurfaceKHR                                                        _surface;

        VkFormat                                                            _surfaceFormat;

        // Note VkSwapchainKHR is a VK_DEFINE_NON_DISPATCHABLE_HANDLE.
        VkSwapchainKHR                                                      _swapchain;

        PFN_vkCreateDebugReportCallbackEXT                                  vkCreateDebugReportCallbackEXT;
        PFN_vkDestroyDebugReportCallbackEXT                                 vkDestroyDebugReportCallbackEXT;

        VkDebugReportCallbackCreateInfoEXT                                  _debugReportCallbackCreateInfoEXT;

        std::map<VkDebugReportFlagsEXT, std::pair<LogType, const char*>>    _loggerMapper;

        std::vector<VkPhysicalDeviceGroupProperties>                        _physicalDeviceGroups;
        std::map<VkPhysicalDevice, VulkanPhysicalDeviceInfo>                _physicalDeviceInfo;
        VkPhysicalDeviceMemoryProperties                                    _physicalDeviceMemoryProperties;

        std::vector<VkFramebuffer>                                          _presentFramebuffers;

        std::vector<VkSurfaceFormatKHR>                                     _surfaceFormats;
        VkExtent2D                                                          _surfaceSize;

        std::vector<VkImage>                                                _swapchainImages;
        std::vector<VkImageView>                                            _swapchainImageViews;

        static const std::map<VkColorSpaceKHR, const char*>                 _vulkanColorSpaceMap;
        static const std::map<VkFormat, const char*>                        _vulkanFormatMap;
        static const std::map<VkDebugReportObjectTypeEXT, const char*>      _vulkanObjectTypeMap;
        static const std::map<VkPhysicalDeviceType, const char*>            _vulkanPhysicalDeviceTypeMap;
        static const std::map<VkPresentModeKHR, const char*>                _vulkanPresentModeMap;
        static const std::map<VkResult, const char*>                        _vulkanResultMap;

    public:
        Renderer ();
        ~Renderer () = default;

        Renderer ( const Renderer &other ) = delete;
        Renderer& operator = ( const Renderer &other ) = delete;

        // Method returns true is "result" equals VK_SUCCESS. Otherwise method returns false.
        bool CheckVkResult ( VkResult result, const char* from, const char* message ) const;

        bool CreateShader ( VkShaderModule &shader,
            std::string &&shaderFile,
            const char* errorMessage
        ) const;

        VkFormat GetDefaultDepthStencilFormat () const;
        VkDevice GetDevice () const;
        VkFramebuffer GetPresentFramebuffer ( uint32_t framebufferIndex ) const;
        size_t GetPresentFramebufferCount () const;
        VkRenderPass GetPresentRenderPass () const;
        VkQueue GetQueue () const;
        uint32_t GetQueueFamilyIndex () const;
        VkFormat GetSurfaceFormat () const;
        const VkExtent2D& GetSurfaceSize () const;
        VkSwapchainKHR& GetSwapchain ();

        bool IsReady () const;

        bool OnInit ( ANativeWindow &nativeWindow, bool vSync );
        void OnDestroy ();

        const char* ResolveVkFormat ( VkFormat format ) const;

        bool SelectTargetMemoryTypeIndex ( uint32_t &targetMemoryTypeIndex,
            const VkMemoryRequirements &memoryRequirements,
            VkMemoryPropertyFlags memoryProperties
        ) const;

        // Note method will invoke AV_REGISTER_DEVICE_MEMORY internally if success.
        bool TryAllocateMemory ( VkDeviceMemory &memory,
            const VkMemoryRequirements &requirements,
            VkMemoryPropertyFlags memoryProperties,
            const char* errorMessage
        ) const;

    private:
        bool DeployDebugFeatures ();
        void DestroyDebugFeatures ();

        bool DeployDevice ();
        void DestroyDevice ();

        bool DeployPresentFramebuffers ();
        void DestroyPresentFramebuffers ();

        bool DeployInstance ();
        void DestroyInstance ();

        bool DeployRenderPass ();
        void DestroyRenderPass ();

        bool DeploySurface ( ANativeWindow &nativeWindow );
        void DestroySurface ();

        bool DeploySwapchain ( bool vSync );
        void DestroySwapchain ();

        bool PrintCoreExtensions () const;
        void PrintFloatProp ( const char* indent, const char* name, float value ) const;
        void PrintFloatVec2Prop ( const char* indent, const char* name, const float value[] ) const;
        void PrintINT32Prop ( const char* indent, const char* name, int32_t value ) const;

        bool PrintInstanceLayerInfo () const;

        void PrintPhysicalDeviceCommonProps ( const VkPhysicalDeviceProperties &props ) const;
        bool PrintPhysicalDeviceExtensionInfo ( VkPhysicalDevice physicalDevice );
        bool PrintPhysicalDeviceFeatureInfo ( VkPhysicalDevice physicalDevice );
        void PrintPhysicalDeviceGroupInfo ( uint32_t groupIndex, const VkPhysicalDeviceGroupProperties &props ) const;
        bool PrintPhysicalDeviceLayerInfo ( const VkPhysicalDevice physicalDevice ) const;
        void PrintPhysicalDeviceLimits ( const VkPhysicalDeviceLimits &limits ) const;
        void PrintPhysicalDeviceMemoryProperties ( VkPhysicalDevice physicalDevice );
        bool PrintPhysicalDeviceInfo ( uint32_t deviceIndex, VkPhysicalDevice physicalDevice );

        void PrintPhysicalDeviceQueueFamilyInfo ( uint32_t queueFamilyIndex,
            const VkQueueFamilyProperties &props
        ) const;

        void PrintPhysicalDeviceSparse ( const VkPhysicalDeviceSparseProperties &sparse ) const;
        void PrintVkBool32Prop ( const char* indent, const char* name, VkBool32 value ) const;

        void PrintVkExtent2DProp ( const char* indent, const char* name, const VkExtent2D &value ) const;
        void PrintVkExtent3DProp ( const char* indent, const char* name, const VkExtent3D &value ) const;

        void PrintVkExtensionProp ( uint32_t extensionIndex,
            const char* category,
            const VkExtensionProperties &extension
        ) const;

        void PrintVkFlagsProp ( const char* indent,
            const char* name,
            VkFlags flags,
            size_t flagSetCount,
            const std::pair<uint32_t, const char*> flagSet[]
        ) const;

        void PrintVkHandler ( const char* indent, const char* name, void* handler ) const;

        void PrintVkLayerProperties ( uint32_t layerIndex, const VkLayerProperties &layer ) const;
        void PrintVkPresentModeProp ( uint32_t modeIndex, VkPresentModeKHR mode ) const;
        void PrintVkSurfaceCapabilities ( const VkSurfaceCapabilitiesKHR &caps );
        void PrintVkSurfaceFormatKHRProp ( uint32_t formatIndex, const VkSurfaceFormatKHR &format ) const;
        void PrintVkVersion ( const char* indent, const char* name, uint32_t version ) const;
        void PrintSizeProp ( const char* indent, const char* name, size_t value ) const;
        void PrintUINT32Prop ( const char* indent, const char* name, uint32_t value ) const;
        void PrintUINT32Vec2Prop ( const char* indent, const char* name, const uint32_t value[] ) const;
        void PrintUINT32Vec3Prop ( const char* indent, const char* name, const uint32_t value[] ) const;
        void PrintUTF8Prop ( const char* indent, const char* name, const char* value ) const;

        const char* ResolvePhysicalDeviceType ( VkPhysicalDeviceType type ) const;
        const char* ResolveVkDebugReportObjectType ( VkDebugReportObjectTypeEXT type ) const;
        const char* ResolveVkColorSpaceKHR ( VkColorSpaceKHR colorSpace ) const;
        const char* ResolveVkPresentModeKHR ( VkPresentModeKHR mode ) const;
        const char* ResolveVkResult ( VkResult result ) const;

        bool SelectTargetHardware ( VkPhysicalDevice &targetPhysicalDevice, uint32_t &targetQueueFamilyIndex ) const;
        bool SelectTargetPresentMode ( VkPresentModeKHR &targetPresentMode, bool vSync ) const;

        bool SelectTargetSurfaceFormat ( VkFormat &targetColorFormat,
            VkColorSpaceKHR &targetColorSpace,
            VkFormat &targetDepthStencilFormat
        ) const;

        static VkBool32 VKAPI_PTR OnVulkanDebugReport ( VkDebugReportFlagsEXT flags,
            VkDebugReportObjectTypeEXT objectType,
            uint64_t object,
            size_t location,
            int32_t messageCode,
            const char* pLayerPrefix,
            const char* pMessage,
            void* pUserData
        );
};

} // namespace android_vulkan


#endif // RENDERER_H
